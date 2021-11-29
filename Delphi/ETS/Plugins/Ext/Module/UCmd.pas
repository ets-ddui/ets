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
unit UCmd;

interface

uses
  Classes, SysUtils, Windows, UInterface, UTool;

type
  {$IFDEF LAZARUS}
  {$M+}
  {$ELSE}
  {$METHODINFO ON}
  {$ENDIF}
  TCmd = class
  strict private
    FThread: TThread;
    function GetRunning: Boolean;
  private
    FSendQueue: Variant;
    FReceiveQueue: Variant;
    FLastError: Integer;
  public
    constructor Create(AFileName: String);
    destructor Destroy; override;
  published
    procedure Stop;
    property Running: Boolean read GetRunning;
    property LastError: Integer read FLastError write FLastError;
    procedure Send(AMessage: String);
    function Receive: String;
  end;
  {$IFDEF LAZARUS}
  {$M-}
  {$ELSE}
  {$METHODINFO OFF}
  {$ENDIF}

implementation

type
  TCmdRunner = class(TThread)
  private
    FRunning: Boolean;
  strict private
    FFileName: String;
    FParent: TCmd;
  strict protected
    procedure DoTerminate; override;
    procedure Execute; override;
  public
    constructor Create(AParent: TCmd; AFileName: String); reintroduce;
  end;

{ TCmd }

constructor TCmd.Create(AFileName: String);
begin
  FSendQueue := Variant(GetRawManager.Plugins['Ext']).GetQueue('__TCmd.FSendQueue__');
  FReceiveQueue := Variant(GetRawManager.Plugins['Ext']).GetQueue('__TCmd.FReceiveQueue__');

  FThread := TCmdRunner.Create(Self, AFileName);
  TCmdRunner(FThread).FRunning := True;
  FThread.Resume;
end;

destructor TCmd.Destroy;
begin
  Stop;
  FreeAndNil(FThread);

  VarClear(FSendQueue);
  VarClear(FReceiveQueue);

  inherited;
end;

function TCmd.GetRunning: Boolean;
begin
  Result := TCmdRunner(FThread).FRunning;
end;

function TCmd.Receive: String;
begin
  FLastError := 0;
  Result := FReceiveQueue.GetData;
  if Result = '' then
    FLastError := FReceiveQueue.LastError;
end;

procedure TCmd.Send(AMessage: String);
begin
  FLastError := 0;
  if VarToBoolean(FSendQueue.AddData(AMessage)) then
    Exit;

  FLastError := FSendQueue.LastError;
end;

procedure TCmd.Stop;
begin
  FThread.Terminate;
  FThread.WaitFor;
end;

{ TCmdRunner }

constructor TCmdRunner.Create(AParent: TCmd; AFileName: String);
begin
  inherited Create(True);

  FFileName := AFileName;
  FParent := AParent;
end;

procedure TCmdRunner.DoTerminate;
begin
  inherited;

  FRunning := False;
end;

procedure TCmdRunner.Execute;
  function PipeInit(var AStdoutWrite, AStdoutRead, AStdinWrite, AStdinRead: THandle): Boolean;
  var
    sa: TSecurityAttributes;
  begin
    sa.nLength := SizeOf(TSecurityAttributes);
    sa.bInheritHandle := True;
    sa.lpSecurityDescriptor := nil;

    Result := CreatePipe(AStdoutRead, AStdoutWrite, @sa, 0);
    if not Result then
    begin
      ReturnValue := GetLastError;
      Exit;
    end;

    Result := CreatePipe(AStdinRead, AStdinWrite, @sa, 0);
    if not Result then
    begin
      ReturnValue := GetLastError;
      Exit;
    end;
  end;

  function ProcessInit(var AProcess, AThread: THandle;
    AStdoutWrite, AStdinRead: THandle): Boolean;
  var
    si: TStartUpInfo;
    pi: TProcessInformation;
  begin
    FillChar(si, SizeOf(si), #0);
    si.cb := SizeOf(si);
    si.hStdOutput := AStdoutWrite;
    si.hStdInput := AStdinRead;
    si.hStdError := AStdoutWrite;
    si.dwFlags := STARTF_USESTDHANDLES or STARTF_USESHOWWINDOW;
    si.wShowWindow := SW_HIDE;

    Result := CreateProcess(nil, PChar(FFileName), nil, nil, True,
      NORMAL_PRIORITY_CLASS, nil, PChar(ExtractFilePath(FFileName)),
      si, pi);
    if not Result then
    begin
      ReturnValue := GetLastError;
      Exit;
    end;

    AProcess := pi.hProcess;
    AThread := pi.hThread;
  end;

  procedure Run(const AProcess, AStdoutRead, AStdinWrite: THandle);
  const
    CBufferSize = 5000;
  var
    iTotalBytesAvail, iNumberOfBytesRead, iResult: Cardinal;
    str, strBuffer, strCommand: String;
    hProcess: THandle;
  begin
    SetLength(strBuffer, CBufferSize + 1);
    hProcess := AProcess;

    while not Terminated do
    begin
      //1.0 ��ȡ�������
      iTotalBytesAvail := 0;
      if not PeekNamedPipe(AStdoutRead, nil, 0, nil, @iTotalBytesAvail, nil) then
      begin
        ReturnValue := GetLastError;
        Exit;
      end;

      while iTotalBytesAvail > 0 do
      begin
        if not ReadFile(AStdoutRead, strBuffer[1], CBufferSize, iNumberOfBytesRead, nil) then
        begin
          ReturnValue := GetLastError;
          Exit;
        end;

        strBuffer[iNumberOfBytesRead] := #0;
        OemToChar(@strBuffer[1], @strBuffer[1]);
        while True do
        begin
          str := PChar(@strBuffer[1]);
          if VarToBoolean(FParent.FReceiveQueue.AddData(str)) then
            Break;

          Sleep(100);
        end;

        Dec(iTotalBytesAvail, iNumberOfBytesRead);
      end;

      //2.0 �����ⲿ����
      strCommand := FParent.FSendQueue.GetData;
      if strCommand <> '' then
      begin
        strCommand := strCommand + #$D#$A;

        if not WriteFile(AStdinWrite, strCommand[1], Length(strCommand), iNumberOfBytesRead, nil) then
        begin
          ReturnValue := GetLastError;
          Exit;
        end;
      end;

      //3.0 ������ִ�������ͬʱ������æ�ȴ�
      if hProcess = 0 then
        Break;

      case WaitForSingleObject(AProcess, 100) of
        WAIT_OBJECT_0:
          hProcess := 0; //����ִ����ɣ���ִ��һ��ѭ������ȡʣ�µķ����ı�
        WAIT_TIMEOUT:
          Continue;
      else
        ReturnValue := GetLastError;
        Exit;
      end;
    end;

    if GetExitCodeProcess(AProcess, iResult) then
    begin
      ReturnValue := GetLastError;
      Exit;
    end;

    ReturnValue := iResult;
  end;

var
  hStdoutWrite, hStdoutRead, hStdinWrite, hStdinRead, hProcess, hThread: THandle;
begin
  ReturnValue := -1;
  hStdoutWrite := 0;
  hStdoutRead := 0;
  hStdinWrite := 0;
  hStdinRead := 0;
  hProcess := 0;
  hThread := 0;

  try
    if not PipeInit(hStdoutWrite, hStdoutRead, hStdinWrite, hStdinRead) then
      Exit;

    if not ProcessInit(hProcess, hThread, hStdoutWrite, hStdinRead) then
      Exit;

    Run(hProcess, hStdoutRead, hStdinWrite);
  finally
    if hProcess <> 0 then
      CloseHandle(hProcess);

    if hThread <> 0 then
      CloseHandle(hThread);

    if hStdoutWrite <> 0 then
      CloseHandle(hStdoutWrite);

    if hStdoutRead <> 0 then
      CloseHandle(hStdoutRead);

    if hStdinWrite <> 0 then
      CloseHandle(hStdinWrite);

    if hStdinRead <> 0 then
      CloseHandle(hStdinRead);
  end;
end;

end.
