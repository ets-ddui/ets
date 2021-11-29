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
unit UQueueManager;

{$i UConfigure.inc}

interface

uses
  Windows, Classes, SyncObjs, UInterface;

type
  //TManager：队列管理器，外部代码通过调用GetQueue，获取指定名称的队列对象
  //TQueueObject：每次创建了新的队列对象后，都会缓存到TManager中，此类对应缓存的元素项
  //TQueue：队列对象，TQueueObject.FObject保存的就是TQueue的对象实例
  //TQueueData：队列中存储的实际数据，为链表结构
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
    FNext: TQueueData; //链表下一个节点的地址
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
    FActive: Boolean; //队列状态，默认是开启状态，当关闭后，无法往队列中新增记录，但可以继续读取记录
    FCount: Integer; //队列中的当前记录数
    FMaxCount: Integer; //队列最大记录数
    FAddTimeOut: Integer; //往队列中添加记录时的最大等待时间，单位为毫秒
    FGetTimeOut: Integer; //从队列中读取记录时的最大等待时间，单位为毫秒
    FLock: TCriticalSection;
    FAddHandle: THandle;
    FGetHandle: THandle;
    FHead: TQueueData; //链表起始节点
    FLastNode: TQueueData; //链表末端节点地址
                           //此链表为先进先出的队列，FLastNode标识新数据插入的位置，
                           //FHead标识读取数据的位置
    procedure SetLastError(AValue: Integer);
    function GetCount: Integer; //指向链表最后一个节点的地址
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
    //释放当前节点的内存，并将游标位置pqd移到下一个节点
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
    //FManager.Log.AddLog('队列已关闭，无法增加新的记录');
    Exit;
  end;

  if WaitForSingleObject(FAddHandle, FAddTimeOut) <> WAIT_OBJECT_0 then
  begin
    SetLastError(2);
    //FManager.Log.AddLog('队列等待超时');
    Exit;
  end;

  try
    qd := TQueueData.Create(nil, AData);
  except
    on e: Exception do
    begin
      SetLastError(-1);
      ReleaseSemaphore(FAddHandle, 1, nil);

      //FManager.Log.AddLog('内存不足');
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
    //FManager.Log.AddLog('队列等待超时');
    Exit;
  end;

  FLock.Enter;
  try
    if FHead.FNext = nil then
    begin
      SetLastError(1);
      ResetEvent(FGetHandle);

      //FManager.Log.AddLog('队列中没有数据');
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
  //最近一次操作的错误号
  //0: 无错误
  //1: 无数据
  //2: 超时
  //3: 队列关闭
  //负数: 严重错误
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
