{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)������չ���߼���

  ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
  �����˿��Ŀ����ϣ�������ã��������κα�֤��
  ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

  ��Դ��ַ: https://github.com/ets-ddui/ets
  ��ԴЭ��: The MIT License (MIT)
  ��������: xinghun87@163.com
  �ٷ����ͣ�https://blog.csdn.net/xinghun61
}
unit UInterface;

interface

uses
  Windows, Classes, Controls, SysUtils, UDUICore;

type
  TPinStyle = (psNone, psLeft, psTop, psRight, psBottom, psClose);
  TMessageType = (mtAddLog, mtGetValue); //ͨ����Ϣѭ����֤�̰߳�ȫ�ԣ�ͨ�������ͱ�ʶ��ͬ��ҵ�����߼�

  IInterfaceNoRefCount = Pointer; //�������ü�����IInterface�ӿ�(�����˽��ӿ�ǿת��Pointer���ͣ����ᴥ�����ü������ܵ�����)

  ICallBack = interface(IInterface)
    ['{B02C1A8D-1B17-4930-867C-7F0734FD3F69}']
    function CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT; stdcall;
  end;

  //Delphi��������C++������(����ֵ��Ϊ�����ĵ�һ���������룬������ӿڶ����ַ���Լ��������)
  //��ˣ���C++��Delphi�Ľӿ�������
  //IManager������C++���ⲿ�ӿڵĶԽ�
  //IRawManager��Delphi�ڲ�ʹ��
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
    //Service��Ŀ���ǽ�ϵͳ�ڲ��ķ����ṩ����չ���ʹ�ã�
    //��չ�����ͨ��IDispatch�ӿڵ�����Щ�����ж���ĺ�����
    //���÷���������
    //GetRawManager.Service['ServiceName'].FunctionName(Variant(Param1), Variant(Param2), ...)
    //�Ӳ��Խ������Delphi������������Bug����������Param1Ϊ������û�����⣬�����Ϊ������
    //��������Ὣ������ַ����������ú�����Ŀǰֻ��ͨ��������ת��ΪVariant���ͺ������ȷ����
    //�Ʋ���ܺ�ObjAuto.pas->ObjectInvoke��ʵ���йأ�������α���ΪVariant�����鴦��ģ�
    //�������ı���ΪString���ͣ����������������ĳ�WideString�ͳ������룬
    //��Integer������Ϊ��Σ���ת��Ϊ��ַ��
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
    FConfig: Pointer; //Framework.json�е�TQJson����
  end;
  TParams = array of TParam;
  //�����ڣ�ͨ������ӿڿɻ�ȡ�������ڼ���������(������־��ϵͳ���õ�)�������Ϣ
  IChild = interface;
  IParent = interface(IInterface)
    ['{8FECB1DC-D28F-4A41-A023-DA18BEC0D77C}']
    procedure AddChild(ACaption: WideString; AChild: IChild);
    function GetParam(AIndex: Integer): TParam;

    //�����Ӵ���ʱ��Ҫ�ĸ������
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

  //IUnknown�ӿ�Ĭ����ͨ��RTTIʵ�֣����Ҫ���ƣ�ֻ�ܴ�TComponent�̳У�
  //��ʵ��һ��IVCLComObject�ӿڵĶ���(�о�����IDispatch���쳣�����������������)��
  //�����������鷳���Ծۺ����ೣ��ģʽ���ô�����˵������ƣ�
  //Delphi��׼��TInterfacedObject���ٽ���ʹ�ã�ͳһʹ������İ汾
  TInterfacedBase = class(TObject, IInterface)
  private
    FParent: IInterfaceNoRefCount; //�������IInterface��Ϊ�˷�ֹѭ������
  protected
    FRefCount: Integer;
    function QueryInterface(const AIID: TGUID; out AResult): HResult; virtual; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  public
    constructor Create(AParent: IInterfaceNoRefCount = nil); reintroduce; virtual;
  end;

  //TInterfacedTest���ڵ��ԣ����ĳ���ض��࣬�۲������ü����ı仯(���滻Ŀ����Ļ���)
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
  //����ǾۺϽӿڣ���ת�������ӿڴ���
  //����ʹ��Ĭ�ϵ�RTTI��ʽ����(�ڵ�ǰʵ���Ľӿڱ��в���)
  if Assigned(FParent) then
    Result := IInterface(FParent).QueryInterface(AIID, AResult) //ǿת���ᵼ�����ü����ı仯
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
    Result := IInterface(FParent)._AddRef //ǿת���ᵼ�����ü����ı仯
  else
    Result := InterlockedIncrement(FRefCount);
end;

function TInterfacedBase._Release: Integer;
begin
  if Assigned(FParent) then
    Result := IInterface(FParent)._Release //ǿת���ᵼ�����ü����ı仯
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
