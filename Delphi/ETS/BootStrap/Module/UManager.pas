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
unit UManager;

interface

uses
  Windows, Forms, Classes, SysUtils, SyncObjs, Messages, UInterface;

const
  ETSM_MAIN_THREAD_LOCK = WM_APP + 2000;

type
  TETSLock = class
  private
    FLock: TCriticalSection;
    FMainThread, FWorkerThread: TEvent;
    procedure Lock;
    procedure UnLock;
  public
    constructor Create; reintroduce;
    destructor Destroy; override;
    procedure MainThreadWait;
  end;

  TManager = class(TInterfacedBase, IManager, IRawManager)
  private
    class var FLock: TETSLock;
    class procedure Init;
    class procedure UnInit;
  private
    FService: IDispatch;
  private
    {IManager实现}
    function GetPlugins(AFileName: WideString; var AResult: IDispatch): HRESULT; overload; stdcall;
    function GetService(AServiceName: WideString; var AResult: Variant): HRESULT; overload; stdcall;
    function Lock: HRESULT; stdcall;
    function UnLock: HRESULT; stdcall;
  private
    {IRawManager实现}
    function GetPlugins(AFileName: WideString): IDispatch; overload;
    function GetService(AServiceName: WideString): Variant; overload;
  public
    constructor Create(AParent: IInterfaceNoRefCount; AService: TObject); reintroduce;
    destructor Destroy; override;
  end;

implementation

uses
  Variants, ActiveX, IniFiles, ULibraryManager, UModuleBase, UTool;

{ TETSLock }

constructor TETSLock.Create;
begin
  inherited;

  FLock := TCriticalSection.Create;
  FMainThread := TEvent.Create(nil, False, False, '');
  FWorkerThread := TEvent.Create(nil, False, False, '');
end;

destructor TETSLock.Destroy;
begin
  FreeAndNil(FWorkerThread);
  FreeAndNil(FMainThread);
  FreeAndNil(FLock);

  inherited;
end;

procedure TETSLock.Lock;
begin
  if GetCurrentThreadID = MainThreadID then
    Exit;

  FLock.Enter;

  //步骤1，通知主线程进入锁定模式
  PostMessage(Application.MainFormHandle, ETSM_MAIN_THREAD_LOCK, 0, Integer(Self));

  //步骤3，主线程进入锁定模式，工作线程开始执行(主线程调用了MainThreadWait)
  FWorkerThread.WaitFor(INFINITE);
end;

procedure TETSLock.UnLock;
begin
  if GetCurrentThreadID = MainThreadID then
    Exit;

  //步骤5，工作线程执行结束，通知主线程恢复执行
  FMainThread.SetEvent;

  //步骤7，主线程已恢复，工作线程释放临界区
  FWorkerThread.WaitFor(INFINITE);

  FLock.Leave;
end;

procedure TETSLock.MainThreadWait;
begin
  //步骤2，通知工作线程开始执行
  FWorkerThread.SetEvent;

  //步骤4，主线程进入锁定模式
  FMainThread.WaitFor(INFINITE);

  //步骤6，通知工作线程，主线程已恢复
  FWorkerThread.SetEvent;
end;

{ TManager }

constructor TManager.Create(AParent: IInterfaceNoRefCount; AService: TObject);
begin
  inherited Create(AParent);

  FService := WrapperObject(AService);
end;

destructor TManager.Destroy;
begin
  FService := nil;

  inherited;
end;

function TManager.GetPlugins(AFileName: WideString; var AResult: IDispatch): HRESULT;
begin
  AResult := GetPlugins(AFileName);
  if Assigned(AResult) then
    Result := S_OK
  else
    Result := E_NOINTERFACE;
end;

function TManager.GetService(AServiceName: WideString; var AResult: Variant): HRESULT;
begin
  AResult := GetService(AServiceName);
  if not VarIsNull(AResult) then
    Result := S_OK
  else
    Result := E_NOINTERFACE;
end;

function TManager.GetPlugins(AFileName: WideString): IDispatch;
begin
  Result := TLibraryManager.LoadModule(AFileName);
end;

function TManager.GetService(AServiceName: WideString): Variant;
type
  TNames = array[0..0] of POleStr;
var
  iDispID: Integer;
  nm: TNames;
  param: TDispParams;
begin
  Result := Null;

  iDispID := -1;
  nm[0] := @AServiceName[1];
  if Succeeded(FService.GetIDsOfNames(GUID_NULL, @nm, 1, 0, @iDispID)) then
  begin
    if iDispID <= 0 then
      Exit;

    FillChar(param, SizeOf(param), 0);
    FService.Invoke(iDispID, GUID_NULL, 0, DISPATCH_METHOD, param, @Result, nil, nil);
  end;
end;

function TManager.Lock: HRESULT;
begin
  FLock.Lock;

  Result := S_OK;
end;

function TManager.UnLock: HRESULT;
begin
  FLock.UnLock;

  Result := S_OK;
end;

class procedure TManager.Init;
begin
  FLock := TETSLock.Create;
end;

class procedure TManager.UnInit;
begin
  FreeAndNil(FLock);
end;

initialization
  TManager.Init;

finalization
  TManager.UnInit;

end.
