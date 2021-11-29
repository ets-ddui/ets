{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)������չ���߼���

  �����������ǿ�Դ�������������������� MIT Э�飬�޸ĺͷ����˳���
  �����˿��Ŀ����ϣ�������ã��������κα�֤��
  ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

  ��Դ��ַ: https://github.com/ets-ddui/ets
  ��ԴЭ��: The MIT License (MIT)
  ��������: xinghun87@163.com
  �ٷ����ͣ�https://blog.csdn.net/xinghun61
}
unit UDebugView;

{$i UConfigure.inc}

interface

uses
  Windows, SysUtils, Classes, Forms, UInterface, Controls, Buttons, ExtCtrls,
  StdCtrls, Messages, UFrameBase, UDUICore, UDUIForm, UDUIWinWrapper, UDUIPanel,
  UDUIButton, UDUIGrid, UDUITreeGrid;

const
  ETSM_GET_MESSAGE = WM_App + 3000;

type
  TFrmDebugView = class(TFrameBase)
    PnlControls: TDUIPanel;
    BtnStart: TDUIButton;
    BtnStop: TDUIButton;
    TgData: TDUITreeGrid;
    BtnClear: TDUIButton;
    procedure DoClick(ASender: TObject);
    procedure FrameBaseInit(AParent: IParent; AIndex: Integer);
    procedure FrameBaseNotify(ANotifyType: TNotifyType; var AResult: Boolean);
  private
    FThread: TThread;
    FQueue: OleVariant;
    procedure ETSMGetMessage(var AMessage: TMessage); message ETSM_GET_MESSAGE;
    procedure DoTerminate(ASender: TObject);
  end;

implementation

{$R *.dfm}

uses
  UTool, UQueueManager;

type
  TDebugViewThread = class(TThread)
  private
    FEventBufferReady, FEventDataReady, FMap: THandle;
    FBuffer: Pointer;
    FQueue: OleVariant;
  protected
    procedure Execute; override;
  public
    constructor Create(AQueue: OleVariant); reintroduce;
    destructor Destroy; override;
  end;

{ TDebugViewThread }

constructor TDebugViewThread.Create(AQueue: OleVariant);
begin
  inherited Create(True);
  FQueue := AQueue;

  FEventBufferReady := 0;
  FEventDataReady := 0;
  FMap := 0;
  FBuffer := nil;
  repeat
    FEventBufferReady := CreateEvent(nil, False, False, 'DBWIN_BUFFER_READY');
    if FEventBufferReady = 0 then
    begin
      //raise Exception.Create('DBWIN_BUFFER_READY����ʧ��');
      Break;
    end;
    if GetLastError = ERROR_ALREADY_EXISTS then
    begin
      //raise Exception.Create('DBWIN_BUFFER_READY�Ѿ�����');
      Break;
    end;

    FEventDataReady := CreateEvent(nil, False, False, 'DBWIN_DATA_READY');
    if FEventDataReady = 0 then
    begin
      //raise Exception.Create('DBWIN_DATA_READY����ʧ��');
      Break;
    end;

    FMap := CreateFileMapping(INVALID_HANDLE_VALUE, nil, PAGE_READWRITE, 0, 4096, 'DBWIN_BUFFER');
    if FMap = 0 then
    begin
      //raise Exception.Create('DBWIN_BUFFER����ʧ��');
      Break;
    end;

    FBuffer := MapViewOfFile(FMap, FILE_MAP_READ, 0, 0, 512);
    if FBuffer = nil then
    begin
      //raise Exception.Create('��DBWIN_BUFFERӳ�䵽�ڴ�ʧ��');
      Break;
    end;

    Exit;
  until True;

  Terminate; //�����쳣�Ż�ִ�е������������£�������ѭ���Ľ������˳�
end;

destructor TDebugViewThread.Destroy;
begin
  if Assigned(FBuffer) then
  begin
    UnmapViewOfFile(FBuffer);
    FBuffer := nil;
  end;

  if FMap <> 0 then
  begin
    CloseHandle(FMap);
    FMap := 0;
  end;

  if FEventDataReady <> 0 then
  begin
    CloseHandle(FEventDataReady);
    FEventDataReady := 0;
  end;

  if FEventBufferReady <> 0 then
  begin
    CloseHandle(FEventBufferReady);
    FEventBufferReady := 0;
  end;

  inherited;
end;

procedure TDebugViewThread.Execute;
var
  pid: PHandle;
  str: PAnsiChar;
begin
  if Terminated then
    Exit;

  pid := PHandle(FBuffer);
  str := PAnsiChar(DWORD(FBuffer) + SizeOf(THandle));

  SetEvent(FEventBufferReady);
  while not Terminated do
  begin
    case WaitForSingleObject(FEventDataReady, 100) of
      WAIT_OBJECT_0:
      begin
        FQueue.AddData(IntToStr(pid^));
        FQueue.AddData(String(str));
        SetEvent(FEventBufferReady);
      end;
      WAIT_TIMEOUT:
      begin
        Continue;
      end
    else
      Break;
    end;
  end;
end;

{ TFrmDebugView }

procedure TFrmDebugView.DoClick(ASender: TObject);
begin
  if ASender = BtnStart then //�����ʼ��ť
  begin
    BtnStart.Enabled := False;
    BtnStop.Enabled := True;

    FThread := TDebugViewThread.Create(FQueue);
    FThread.FreeOnTerminate := True;
    FThread.OnTerminate := DoTerminate;
    FThread.Resume;

    PerformAsync(ETSM_GET_MESSAGE, 0, 0);
  end
  else if ASender = BtnStop then //���ֹͣ��ť
  begin
    if Assigned(FThread) then
    begin
      FThread.Terminate;
    end;
  end
  else if ASender = BtnClear then
  begin
    TgData.RootNode.Clear;
  end;
end;

procedure TFrmDebugView.DoTerminate(ASender: TObject);
begin
  FThread := nil;
  BtnStart.Enabled := True;
  BtnStop.Enabled := False;
end;

procedure TFrmDebugView.ETSMGetMessage(var AMessage: TMessage);
var
  strProcessID, strMessage: String;
  tn: TDUITreeNode;
begin
  while Assigned(FThread) or (Integer(FQueue.Count) > 0) do
  begin
    if Integer(FQueue.Count) = 0 then
    begin
      //TThread.Synchronize����˵��(TThread.OnTerminate����ͨ���������ʵ�֣����������Idle���̻߳����)
      //1. Synchronize���ڲ�����SyncList(ȫ�ֱ���)������TSyncProc���͵�ָ�������
      //2. ��Application����Ϣѭ���У�ͨ����ϢWM_NULL����Idle��ȡSyncList�е����ݣ�
      //   �����нṹ�а�װ�ĺ���(���ȫ�ֺ���CheckSynchronize��ʵ��)��
      //�Ӳ��Խ������WM_NULL�ڼ��ر������²Żᴥ����ͨ��Idle�����ĸ��ʸ��ߣ�
      //ProcessMessages���ᴥ��Idle(����Ϣѭ�����õ�HandleMessage�ᴥ��)��
      //��ˣ�����ͨ������DoApplicationIdle������Idle�Ĵ�������֤DoTerminate����ִ��
      Application.ProcessMessages;
      {$IFNDEF LAZARUS}
      Application.DoApplicationIdle;
      {$ENDIF}
    end;

    strProcessID := FQueue.GetData;
    if FQueue.LastError <> 0 then
      Continue;

    repeat
      strMessage := FQueue.GetData;
    until FQueue.LastError = 0;

    tn := TgData.RootNode.First;
    while Assigned(tn) do
    begin
      if tn.Caption = strProcessID then
        Break;

      if tn = TgData.RootNode.Last then
        tn := nil
      else
        tn := tn.Next;
    end;

    if not Assigned(tn) then
    begin
      tn := TgData.RootNode.AddChild(strProcessID);
      TgData.Cells[TgData.Columns[1], tn] := GetProcessImageNameByID(StrToInt(strProcessID));
    end;

    tn := tn.AddChild(FormatDateTime('hh:nn:ss.zzz', Now), True);
    TgData.Cells[TgData.Columns[1], tn] := strMessage;
  end;
end;

procedure TFrmDebugView.FrameBaseInit(AParent: IParent; AIndex: Integer);
begin
  FQueue := TManager.GetQueue(ClassName);
end;

procedure TFrmDebugView.FrameBaseNotify(ANotifyType: TNotifyType; var AResult: Boolean);
begin
  if ANotifyType = ntDeActive then
  begin
    if Assigned(FThread) then
    begin
      AResult := False;
      Exit;
    end;
  end;
end;

initialization
  TFrameBase.RegFrame(TFrmDebugView);

end.