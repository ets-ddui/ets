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
      //raise Exception.Create('DBWIN_BUFFER_READY创建失败');
      Break;
    end;
    if GetLastError = ERROR_ALREADY_EXISTS then
    begin
      //raise Exception.Create('DBWIN_BUFFER_READY已经存在');
      Break;
    end;

    FEventDataReady := CreateEvent(nil, False, False, 'DBWIN_DATA_READY');
    if FEventDataReady = 0 then
    begin
      //raise Exception.Create('DBWIN_DATA_READY创建失败');
      Break;
    end;

    FMap := CreateFileMapping(INVALID_HANDLE_VALUE, nil, PAGE_READWRITE, 0, 4096, 'DBWIN_BUFFER');
    if FMap = 0 then
    begin
      //raise Exception.Create('DBWIN_BUFFER创建失败');
      Break;
    end;

    FBuffer := MapViewOfFile(FMap, FILE_MAP_READ, 0, 0, 512);
    if FBuffer = nil then
    begin
      //raise Exception.Create('将DBWIN_BUFFER映射到内存失败');
      Break;
    end;

    Exit;
  until True;

  Terminate; //出现异常才会执行到这里，正常情况下，在上面循环的结束会退出
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
  if ASender = BtnStart then //点击开始按钮
  begin
    BtnStart.Enabled := False;
    BtnStop.Enabled := True;

    FThread := TDebugViewThread.Create(FQueue);
    FThread.FreeOnTerminate := True;
    FThread.OnTerminate := DoTerminate;
    FThread.Resume;

    PerformAsync(ETSM_GET_MESSAGE, 0, 0);
  end
  else if ASender = BtnStop then //点击停止按钮
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
      //TThread.Synchronize机制说明(TThread.OnTerminate就是通过这个机制实现，如果不处理Idle，线程会假死)
      //1. Synchronize往内部队列SyncList(全局变量)中增加TSyncProc类型的指针变量；
      //2. 在Application的消息循环中，通过消息WM_NULL或函数Idle读取SyncList中的数据，
      //   并运行结构中包装的函数(详见全局函数CheckSynchronize的实现)；
      //从测试结果看，WM_NULL在极特别的情况下才会触发，通过Idle触发的概率更高，
      //ProcessMessages不会触发Idle(主消息循环调用的HandleMessage会触发)，
      //因此，这里通过调用DoApplicationIdle来触发Idle的处理，保证DoTerminate正常执行
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
