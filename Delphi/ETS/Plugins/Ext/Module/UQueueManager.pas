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
unit UQueueManager;

{$i UConfigure.inc}

interface

uses
  Windows, Classes, SyncObjs, UInterface;

type
  //TManager�����й��������ⲿ����ͨ������GetQueue����ȡָ�����ƵĶ��ж���
  //TQueueObject��ÿ�δ������µĶ��ж���󣬶��Ỻ�浽TManager�У������Ӧ�����Ԫ����
  //TQueue�����ж���TQueueObject.FObject����ľ���TQueue�Ķ���ʵ��
  //TQueueData�������д洢��ʵ�����ݣ�Ϊ����ṹ
  TManager = class
  private
    class var FQueues: TStringList;
    class var FCreateLock: TCriticalSection;
  public
    class procedure Init;
    class procedure UnInit;
    class function GetQueue(AQueueName: String): IDispatch;
  end;

implementation

uses
  SysUtils, UDispatchWrapper, UTls;

type
  TQueueObject = class
  private
    FObject: IDispatch;
  public
    constructor Create(AObject: IDispatch);
  end;

  TQueueData = class
  private
    FNext: TQueueData; //������һ���ڵ�ĵ�ַ
    FData: String;
  public
    constructor Create(ANext: TQueueData; AData: String);
    property Data: String read FData;
  end;

  {$IFDEF LAZARUS}
  {$M+}
  {$ELSE}
  {$METHODINFO ON}
  {$ENDIF}
  TQueue = class
  private
    FActive: Boolean; //����״̬��Ĭ���ǿ���״̬�����رպ��޷���������������¼�������Լ�����ȡ��¼
    FCount: Integer; //�����еĵ�ǰ��¼��
    FMaxCount: Integer; //��������¼��
    FAddTimeOut: Integer; //����������Ӽ�¼ʱ�����ȴ�ʱ�䣬��λΪ����
    FGetTimeOut: Integer; //�Ӷ����ж�ȡ��¼ʱ�����ȴ�ʱ�䣬��λΪ����
    FLock: TCriticalSection;
    FAddHandle: THandle;
    FGetHandle: THandle;
    FHead: TQueueData; //������ʼ�ڵ�
    FLastNode: TQueueData; //����ĩ�˽ڵ��ַ
                           //������Ϊ�Ƚ��ȳ��Ķ��У�FLastNode��ʶ�����ݲ����λ�ã�
                           //FHead��ʶ��ȡ���ݵ�λ��
    procedure SetLastError(AValue: Integer);
    function GetCount: Integer; //ָ���������һ���ڵ�ĵ�ַ
    constructor Create(AMaxCount: Integer; AAddTimeOut: Integer; AGetTimeOut: Integer);
    function GetLastError: Integer;
  public
    destructor Destroy; override;
    procedure Clear;
  published
    function AddData(AData: String): Boolean;
    function GetData: String;
    property Count: Integer read GetCount;
    property Active: Boolean read FActive write FActive;
    property AddTimeOut: Integer read FAddTimeOut write FAddTimeOut;
    property GetTimeOut: Integer read FGetTimeOut write FGetTimeOut;
    property LastError: Integer read GetLastError;
  end;
  {$IFDEF LAZARUS}
  {$M-}
  {$ELSE}
  {$METHODINFO OFF}
  {$ENDIF}

{ TQueueObject }

constructor TQueueObject.Create(AObject: IDispatch);
begin
  FObject := AObject;
end;

{ TQueueData }

constructor TQueueData.Create(ANext: TQueueData; AData: String);
begin
  FNext := ANext;
  FData := AData;
end;

{ TManager }

class function TManager.GetQueue(AQueueName: String): IDispatch;
var
  i: Integer;
begin
  FCreateLock.Enter;
  try
    i := FQueues.IndexOf(AQueueName);
    if i >= 0 then
    begin
      Result := TQueueObject(FQueues.Objects[i]).FObject;
      Exit;
    end;

    Result := TDispatchWrapper.Create(TQueue.Create(1000000, 100, 100), True);
    FQueues.AddObject(AQueueName, TQueueObject.Create(Result));
  finally
    FCreateLock.Leave;
  end;
end;

class procedure TManager.Init;
begin
  FCreateLock := TCriticalSection.Create;
  FQueues := TStringList.Create;
  FQueues.Sorted := True;
end;

class procedure TManager.UnInit;
var
  i: Integer;
begin
  FCreateLock.Enter;
  try
    for i := FQueues.Count - 1 downto 0 do
      FQueues.Objects[i].Free;
  finally
    FCreateLock.Leave;
  end;

  FreeAndNil(FQueues);
  FreeAndNil(FCreateLock);
end;

{ TQueue }

constructor TQueue.Create(AMaxCount, AAddTimeOut, AGetTimeOut: Integer);
begin
  FCount := 0;
  FMaxCount := AMaxCount;
  FAddTimeOut := AAddTimeOut;
  FGetTimeOut := AGetTimeOut;

  FHead := TQueueData.Create(nil, '');
  FLastNode := FHead;

  FLock := TCriticalSection.Create;
  FGetHandle := CreateEvent(nil, True, False, nil);
  FAddHandle := CreateSemaphore(nil, FMaxCount, FMaxCount, nil);

  FActive := True;
end;

destructor TQueue.Destroy;
begin
  Clear;

  FreeAndNil(FHead);
  FreeAndNil(FLock);
  CloseHandle(FGetHandle);
  CloseHandle(FAddHandle);
end;

procedure TQueue.Clear;
var
  pqd, pqdFree: TQueueData;
begin
  FLock.Enter;
  try
    if FCount = 0 then
    begin
      SetLastError(1);
      Exit;
    end;

    ResetEvent(FGetHandle);
    ReleaseSemaphore(FAddHandle, FCount, nil);

    pqd := FHead.FNext;
    FHead.FNext := nil;
    FLastNode := FHead;
  finally
    FLock.Leave;
  end;

  while Assigned(pqd) do
  begin
    //�ͷŵ�ǰ�ڵ���ڴ棬�����α�λ��pqd�Ƶ���һ���ڵ�
    pqdFree := pqd;
    pqd := pqd.FNext;
    FreeAndNil(pqdFree);
  end;

  SetLastError(0);
end;

function TQueue.AddData(AData: String): Boolean;
var
  qd: TQueueData;
begin
  Result := False;

  if not FActive then
  begin
    SetLastError(3);
    //FManager.Log.AddLog('�����ѹرգ��޷������µļ�¼');
    Exit;
  end;

  if WaitForSingleObject(FAddHandle, FAddTimeOut) <> WAIT_OBJECT_0 then
  begin
    SetLastError(2);
    //FManager.Log.AddLog('���еȴ���ʱ');
    Exit;
  end;

  try
    qd := TQueueData.Create(nil, AData);
  except
    on e: Exception do
    begin
      SetLastError(-1);
      ReleaseSemaphore(FAddHandle, 1, nil);

      //FManager.Log.AddLog('�ڴ治��');
      Exit;
    end;
  end;

  FLock.Enter;
  try
    FLastNode.FNext := qd;
    FLastNode := qd;

    Inc(FCount);
  finally
    FLock.Leave;
  end;

  SetEvent(FGetHandle);

  Result := True;

  SetLastError(0);
end;

function TQueue.GetData: String;
var
  qd: TQueueData;
begin
  Result := '';

  if WaitForSingleObject(FGetHandle, FGetTimeOut) <> WAIT_OBJECT_0 then
  begin
    SetLastError(2);
    //FManager.Log.AddLog('���еȴ���ʱ');
    Exit;
  end;

  FLock.Enter;
  try
    if FHead.FNext = nil then
    begin
      SetLastError(1);
      ResetEvent(FGetHandle);

      //FManager.Log.AddLog('������û������');
      Exit;
    end;

    Dec(FCount);
    qd := FHead.FNext;
    FHead.FNext := FHead.FNext.FNext;
    if FHead.FNext = nil then
    begin
      FLastNode := FHead;
    end;
  finally
    FLock.Leave;
  end;

  ReleaseSemaphore(FAddHandle, 1, nil);

  Result := qd.FData;
  FreeAndNil(qd);

  SetLastError(0);
end;

function TQueue.GetLastError: Integer;
begin
  Result := StrToIntDef(UTls.StaticGetTlsValue('__TQueue.LastError__'), 0);
end;

procedure TQueue.SetLastError(AValue: Integer);
begin
  //���һ�β����Ĵ����
  //0: �޴���
  //1: ������
  //2: ��ʱ
  //3: ���йر�
  //����: ���ش���
  UTls.StaticSetTlsValue('__TQueue.LastError__', IntToStr(AValue));
end;

function TQueue.GetCount: Integer;
begin
  FLock.Enter;
  try
    Result := FCount;
  finally
    FLock.Leave;
  end;

  SetLastError(0);
end;

initialization
  TManager.Init;

finalization
  TManager.UnInit;

end.
