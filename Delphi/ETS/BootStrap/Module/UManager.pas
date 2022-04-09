{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)������չ���߼���

  ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
  �����˿��Ŀ����ϣ�������ã��������κα�֤��
  ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

  ��Դ��ַ: https://github.com/ets-ddui/ets
            https://gitee.com/ets-ddui/ets
  ��ԴЭ��: The MIT License (MIT)
  ��������: xinghun87@163.com
  �ٷ����ͣ�https://blog.csdn.net/xinghun61
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
    {IManagerʵ��}
    function GetPlugins(AFileName: WideString; var AResult: IDispatch): HRESULT; overload; stdcall;
    function GetService(AServiceName: WideString; var AResult: Variant): HRESULT; overload; stdcall;
    function Lock: HRESULT; stdcall;
    function UnLock: HRESULT; stdcall;
  private
    {IRawManagerʵ��}
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

  //����1��֪ͨ���߳̽�������ģʽ
  PostMessage(Application.MainFormHandle, ETSM_MAIN_THREAD_LOCK, 0, Integer(Self));

  //����3�����߳̽�������ģʽ�������߳̿�ʼִ��(���̵߳�����MainThreadWait)
  FWorkerThread.WaitFor(INFINITE);
end;

procedure TETSLock.UnLock;
begin
  if GetCurrentThreadID = MainThreadID then
    Exit;

  //����5�������߳�ִ�н�����֪ͨ���ָ̻߳�ִ��
  FMainThread.SetEvent;

  //����7�����߳��ѻָ��������߳��ͷ��ٽ���
  FWorkerThread.WaitFor(INFINITE);

  FLock.Leave;
end;

procedure TETSLock.MainThreadWait;
begin
  //����2��֪ͨ�����߳̿�ʼִ��
  FWorkerThread.SetEvent;

  //����4�����߳̽�������ģʽ
  FMainThread.WaitFor(INFINITE);

  //����6��֪ͨ�����̣߳����߳��ѻָ�
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
