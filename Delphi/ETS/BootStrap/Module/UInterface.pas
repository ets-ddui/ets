{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)，可扩展工具集。

  本工具软件是开源自由软件，您可以遵照 MIT 协议，修改和发布此程序。
  发布此库的目的是希望其有用，但不做任何保证。
  如果将本库用于商业项目，由于本库中的Bug，而引起的任何风险及损失，本作者不承担任何责任。

  开源地址: https://github.com/ets-ddui/ets
  开源协议: The MIT License (MIT)
  作者邮箱: xinghun87@163.com
  官方博客：https://blog.csdn.net/xinghun61
}
unit UInterface;

interface

uses
  Windows, Classes, Controls, SysUtils, UDUICore;

type
  TPinStyle = (psNone, psLeft, psTop, psRight, psBottom, psClose);
  TMessageType = (mtAddLog, mtGetValue); //通过消息循环保证线程安全性，通过此类型标识不同的业务处理逻辑

  IInterfaceNoRefCount = Pointer; //不带引用计数的IInterface接口(利用了将接口强转成Pointer类型，不会触发引用计数功能的特性)

  ICallBack = interface(IInterface)
    ['{B02C1A8D-1B17-4930-867C-7F0734FD3F69}']
    function CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT; stdcall;
  end;

  //Delphi的属性与C++不兼容(返回值作为函数的第一个参数传入，后面跟接口对象地址，以及函数入参)
  //因此，将C++和Delphi的接口做分离
  //IManager用于与C++等外部接口的对接
  //IRawManager供Delphi内部使用
  IManager = interface(IInterface)
    ['{E43AC553-3020-4A0A-91D4-7CA936DE27E4}']
    function GetPlugins(AFileName: WideString; var AResult: IDispatch): HRESULT; stdcall;
    function GetService(AServiceName: WideString; var AResult: Variant): HRESULT; stdcall;
    function Lock: HRESULT; stdcall;
    function UnLock: HRESULT; stdcall;
  end;

  IRawManager = interface(IInterface)
    ['{B1495366-12C1-4239-8203-9D666DA4E1A2}']
    function GetPlugins(AFileName: WideString): IDispatch;
    function GetService(AServiceName: WideString): Variant;

    property Plugins[AFileName: WideString]: IDispatch read GetPlugins;
    //Service的目的是将系统内部的服务提供给扩展插件使用，
    //扩展插件可通过IDispatch接口调用这些服务中定义的函数；
    //调用方法样例：
    //GetRawManager.Service['ServiceName'].FunctionName(Variant(Param1), Variant(Param2), ...)
    //从测试结果看，Delphi编译器好像有Bug，如果传入的Param1为常量，没有问题，但如果为变量，
    //则编译器会将变量地址传入给被调用函数，目前只能通过将变量转换为Variant类型后才能正确处理；
    //推测可能和ObjAuto.pas->ObjectInvoke的实现有关，函数入参被作为Variant的数组处理的，
    //如果传入的变量为String类型，可以正常处理，但改成WideString就成了乱码，
    //用Integer类型作为入参，则被转换为地址；
    property Service[AServiceName: WideString]: Variant read GetService;
  end;

  TEncodingType = (etBinary, etGbk, etUtf8, etUnicode);
  IMemoryBlock = interface(IInterface)
    ['{A186C64B-DA6F-438E-A4B8-A485F7C7573C}']
    function GetEncoding(var AResult: TEncodingType): HRESULT; stdcall;
    function SetEncoding(AValue: TEncodingType): HRESULT; stdcall;
    function GetSize(var AResult: Cardinal): HRESULT; stdcall;
    function SetSize(AValue: Cardinal): HRESULT; stdcall;
    function Read(APosition: Cardinal; var AValue: Byte; var ALength: Cardinal): HRESULT; stdcall;
    function Write(APosition: Cardinal; var AValue: Byte; ALength: Cardinal): HRESULT; stdcall;
  end;

  TParam = packed record
    FHandle: Cardinal;
    FRect: TRect;
    FParent: TDUIBase;
    FConfig: Pointer; //Framework.json中的TQJson对象
  end;
  TParams = array of TParam;
  //主窗口，通过这个接口可获取到主窗口及辅助功能(例如日志、系统设置等)的相关信息
  IChild = interface;
  IParent = interface(IInterface)
    ['{8FECB1DC-D28F-4A41-A023-DA18BEC0D77C}']
    procedure AddChild(ACaption: WideString; AChild: IChild);
    function GetParam(AIndex: Integer): TParam;

    //创建子窗口时需要的各类参数
    property Param[AIndex: Integer]: TParam read GetParam; default;
  end;
  TNotifyType = (ntActive, ntDeActive, ntResize, ntIdle);
  IChild = interface(IInterface)
    ['{27B7A08D-B50F-4A07-974B-C944F541BBE9}']
    procedure Init(AParent: IParent; AIndex: Integer);
    procedure Notify(ANotifyType: TNotifyType; var AResult: Boolean);
  end;

  IChildCreator = interface(IInterface)
    ['{698D6B78-7561-4E24-8100-0C00415B60D7}']
    function GetID: WideString;
    function CreateChild(AOwner: TComponent): IChild;

    property ID: WideString read GetID;
  end;

  TScriptLanguage = (slJScript, slPython);
  IScript = interface(IInterface)
    ['{8A141C26-D302-40E9-8CD0-55A5EF619B3D}']
    procedure RegContainer(AContainer: IDispatch); stdcall;
    procedure RegFrame(AFrame: IDispatch); stdcall;
    procedure RunModule(AFileName, AEntryFunction: WideString); stdcall;
    procedure RunCode(ACode: WideString); stdcall;
  end;

  ILog = interface(IInterface)
    ['{85E7F4BA-3CB0-422C-A5A9-51A1D0989E23}']
    procedure AddLog(AMessage: WideString); stdcall;
  end;

  ILogManager = interface(IInterface)
    ['{48E2D872-5612-438F-9C98-FAE01B6DF5B5}']
    procedure Clear; stdcall;
    function GetCount: Integer; stdcall;
    function GetLog(AIndex: Integer): WideString; stdcall;
    function RegistCallBack(ACallBack: ICallBack): Pointer; stdcall;
    procedure UnRegistCallBack(AID: Pointer); stdcall;
  end;

  ISetting = interface(IInterface)
    ['{ADAECDD6-224F-4EFA-B659-4E6BBF37171C}']
    function GetCount: Integer; stdcall;
    function GetItem(AIndex: Integer): ISetting; stdcall;
    function GetItemByPath(APath: WideString): ISetting; stdcall;
    function GetType: Integer; stdcall;
    function GetValue: WideString; stdcall;
    function GetValueByPath(APath: WideString; ADefault: WideString): WideString; stdcall;
  end;

  ISettingManager = interface(IInterface)
    ['{CE98D149-D5A9-48CF-B2A1-8C79A979FC55}']
    function GetCount: Integer; stdcall;
    function GetItem(AIndex: Integer): ISettingManager; stdcall;
    function GetItemByPath(APath: WideString): ISettingManager; stdcall;
    function GetObject: Cardinal; stdcall;
    function GetType: Integer; stdcall;
    function GetValue: WideString; stdcall;
    function GetValueByPath(APath: WideString; ADefault: WideString): WideString; stdcall;
    procedure SetValue(AValue: WideString); stdcall;
    procedure SetValueByPath(APath: WideString; AValue: WideString); stdcall;
    function IsDirty: Boolean; stdcall;
    procedure Save; stdcall;
    function RegistCallBack(ACallBack: ICallBack): Pointer; stdcall;
    procedure UnRegistCallBack(AID: Pointer); stdcall;
  end;

  ITrayIconEvent = interface(IInterface)
    ['{6DD7167E-F02D-4F90-B0A3-3B650493ECED}']
    procedure DoClick(AID, ASubID: Integer);
  end;

  //IUnknown接口默认是通过RTTI实现，如果要定制，只能从TComponent继承，
  //并实现一个IVCLComObject接口的对象(感觉像是IDispatch、异常处理、析构处理的整合)，
  //处理起来较麻烦，对聚合这类常用模式不好处理，因此单独定制，
  //Delphi标准的TInterfacedObject不再建议使用，统一使用这里的版本
  TInterfacedBase = class(TObject, IInterface)
  private
    FParent: IInterfaceNoRefCount; //不定义成IInterface是为了防止循环引用
  protected
    FRefCount: Integer;
    function QueryInterface(const AIID: TGUID; out AResult): HResult; virtual; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  public
    constructor Create(AParent: IInterfaceNoRefCount = nil); reintroduce; virtual;
  end;

  //TInterfacedTest用于调试，针对某个特定类，观察其引用计数的变化(需替换目标类的基类)
  TInterfacedTest = class(TInterfacedBase, IInterface)
  protected
    function QueryInterface(const AIID: TGUID; out AResult): HResult; override; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  end;

  TCallBackList = class
  private
    FList: TList;
  public
    constructor Create;
    destructor Destroy; override;
    function Add(ACallBack: ICallBack): Pointer;
    procedure Delete(AID: Pointer);
    procedure Clear;
    function CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT;
  end;

  function GetManager: IManager;
  function GetRawManager: IRawManager;
  procedure SetManager(AValue: IManager);

implementation

var
  GManager: IManager = nil;

function GetManager: IManager;
begin
  Result := GManager;
end;

function GetRawManager: IRawManager;
begin
  GManager.QueryInterface(IRawManager, Result);
end;

procedure SetManager(AValue: IManager);
begin
  if GManager = AValue then
    Exit;

  GManager := AValue;
end;

{ TInterfacedBase }

constructor TInterfacedBase.Create(AParent: IInterfaceNoRefCount);
begin
  FParent := AParent;
end;

function TInterfacedBase.QueryInterface(const AIID: TGUID; out AResult): HResult;
begin
  //如果是聚合接口，则转发到父接口处理
  //否则，使用默认的RTTI方式处理(在当前实例的接口表中查找)
  if Assigned(FParent) then
    Result := IInterface(FParent).QueryInterface(AIID, AResult) //强转不会导致引用计数的变化
  else
  begin
    if GetInterface(AIID, AResult) then
      Result := 0
    else
      Result := E_NOINTERFACE;
  end;
end;

function TInterfacedBase._AddRef: Integer;
begin
  if Assigned(FParent) then
    Result := IInterface(FParent)._AddRef //强转不会导致引用计数的变化
  else
    Result := InterlockedIncrement(FRefCount);
end;

function TInterfacedBase._Release: Integer;
begin
  if Assigned(FParent) then
    Result := IInterface(FParent)._Release //强转不会导致引用计数的变化
  else
  begin
    Result := InterlockedDecrement(FRefCount);
    if Result = 0 then
      Destroy;
  end;
end;

{ TInterfacedTest }

function TInterfacedTest.QueryInterface(const AIID: TGUID; out AResult): HResult;
begin
  Result := inherited QueryInterface(AIID, AResult);
end;

function TInterfacedTest._AddRef: Integer;
begin
  Result := inherited _AddRef;
end;

function TInterfacedTest._Release: Integer;
begin
  Result := inherited _Release;
end;

{ TCallBackList }

type
  TCallBackItem = record
    FResult: HRESULT;
    FCallBack: ICallBack;
  end;
  PCallBackItem = ^TCallBackItem;

function TCallBackList.Add(ACallBack: ICallBack): Pointer;
var
  pcbi: PCallBackItem;
begin
  New(pcbi);
  pcbi^.FResult := S_OK;
  pcbi^.FCallBack := ACallBack;
  FList.Add(pcbi);

  Result := pcbi;
end;

function TCallBackList.CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT;
var
  i: Integer;
  pcbi: PCallBackItem;
begin
  Result := S_OK;

  for i := 0 to FList.Count - 1 do
  begin
    pcbi := FList[i];
    pcbi^.FResult := pcbi^.FCallBack.CallBack(AMessage, AWParam, ALParam);
    if Failed(pcbi^.FResult) then
      Result := pcbi^.FResult;
  end;
end;

procedure TCallBackList.Clear;
var
  i: Integer;
  pcbi: PCallBackItem;
begin
  for i := FList.Count - 1 downto 0 do
  begin
    pcbi := FList[i];
    Dispose(pcbi);
  end;
  FList.Clear;
end;

constructor TCallBackList.Create;
begin
  FList := TList.Create;
end;

procedure TCallBackList.Delete(AID: Pointer);
var
  i: Integer;
begin
  i := FList.IndexOf(AID);
  if i < 0 then
    Exit;

  Dispose(PCallBackItem(AID));
  FList.Delete(i);
end;

destructor TCallBackList.Destroy;
begin
  Clear;
  FreeAndNil(FList);

  inherited;
end;

initialization

finalization
  SetManager(nil);

end.
