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
unit UScriptThread;

{$i UConfigure.inc}

interface

uses
  Windows, Classes, SysUtils, StrUtils, Forms, ActiveX, UInterface, UMemoryBlock, UTool;

function GetScriptThreadContainer: IDispatch;

implementation

uses
  SyncObjs, UModuleBase;

type
  //TScriptThreadContainerΪ��̬ʵ��
  {$IFDEF LAZARUS}
  {$M+}
  {$ELSE}
  {$METHODINFO ON}
  {$ENDIF}
  TScriptThreadContainer = class
  private
    FThreads: TList;
    FCachedFile: TStringList;
    FTerminated: WordBool;
    FFrame: IDispatch;
  private
    class var FLock: TCriticalSection;
    class var FInstance: IDispatch;
    class procedure Init;
    class procedure UnInit;
  public
    constructor Create;
    destructor Destroy; override;
    procedure Terminate;
    //�ű��н�ֹ���ô˺���(��Ϊִ�нű����߳�Ҳ��WaitFor�ķ�Χ֮��)
    procedure WaitFor;
    procedure Restart;
  published
    procedure AddCacheFile(AFileName: String; ACode: IMemoryBlock);
    //����ֵ������Terminate����ҪAIndex��εĺ���
    function AddThread(AFileName, AEntryFunction: String): Integer;
    procedure GetCacheFile(AFileName: WideString; var ACode: IInterface; var ACached: WordBool);
    procedure RegFrame(AFrame: IDispatch);
    //AIndex����˵����
    //-1�������߳�
    //����ֵ��ָ��ĳ���߳�
    procedure Resume(AIndex: Integer = -1);
    property Terminated: WordBool read FTerminated;
  end;
  {$IFDEF LAZARUS}
  {$M-}
  {$ELSE}
  {$METHODINFO OFF}
  {$ENDIF}

function GetScriptThreadContainer: IDispatch;
begin
  Result := TScriptThreadContainer.FInstance;
end;

function GetScriptLanguage(AFileName: String): TScriptLanguage;
var
  strExt: String;
begin
  strExt := ExtractFileExt(AFileName);

  if CompareText(strExt, '.js') = 0 then
    Result := slJScript
  else if CompareText(strExt, '.py') = 0 then
    Result := slPython
  else
    Result := slJScript;
end;

{ TPressTestThread }

type
  TScriptThread = class(TThread)
  private
    FContainer: TScriptThreadContainer;
    FFileName, FEntryFunction: WideString;
  protected
    procedure Execute; override;
  public
    constructor Create(AContainer: TScriptThreadContainer;
      AFileName, AEntryFunction: String); reintroduce;
    procedure Terminate; reintroduce;
  end;

constructor TScriptThread.Create(AContainer: TScriptThreadContainer;
  AFileName, AEntryFunction: String);
begin
  FContainer := AContainer;
  FFileName := AFileName;
  FEntryFunction := AEntryFunction;

  inherited Create(False);
end;

procedure TScriptThread.Execute;
  procedure doExecute;
  var
    objModule, objScript, objContainer: IDispatch;
    script: IScript;
  begin
    try
      objModule := GetRawManager.Plugins['Script'];
      if not Assigned(objModule) then
      begin
        GetRawManager.Service['Log'].AddLog('Script.dll����ʧ��');
        Exit;
      end;

      case GetScriptLanguage(FFileName) of
        slJScript: objScript := Variant(objModule).GetJScript;
        slPython: objScript := Variant(objModule).GetPython;
      else
        GetRawManager.Service['Log'].AddLog('�޷�ʶ��Ľű�����');
        Exit;
      end;

      if not Assigned(objScript) then
      begin
        GetRawManager.Service['Log'].AddLog('�ű���������ʼ��ʧ��');
        Exit;
      end;

      if Supports(objScript, IScript, script) then
      begin
        objContainer := WrapperObject(FContainer, False);
        script.RegContainer(objContainer);
        script.RegFrame(FContainer.FFrame);
        script.RunModule(FFileName, FEntryFunction);
      end;
    finally
      script := nil;
      objContainer := nil;
      objScript := nil;
      objModule := nil;
    end;
  end;
begin
  if Terminated then
    Exit;

  CoInitialize(nil);
  try try
    //�������߼��ú������а�װ��Ŀ���Ƿ�ֹ��ִ�й��������ɵ���ʱ������
    //�ڵ���CoUninitialize֮��ű�����
    doExecute;
  except
    GetRawManager.Service['Log'].AddLog('TScriptThreadִ��ʧ��');
  end;
  finally
    CoUninitialize;
  end;
end;

procedure TScriptThread.Terminate;
begin
  TScriptThreadContainer.FLock.Enter;
  try
    inherited;
  finally
    TScriptThreadContainer.FLock.Leave;
  end;
end;

{ TCode }

type
  TCode = class
  private
    FCode: IMemoryBlock;
  public
    constructor Create(ACode: IMemoryBlock);
  end;

constructor TCode.Create(ACode: IMemoryBlock);
begin
  FCode := ACode;
end;

{ TScriptThreadContainer }

constructor TScriptThreadContainer.Create;
begin
  FThreads := TList.Create;
  FCachedFile := TStringList.Create;
  FCachedFile.Sorted := True;
  FTerminated := False;
end;

destructor TScriptThreadContainer.Destroy;
begin
  WaitFor;

  FreeAndNil(FThreads);
  FreeAndNil(FCachedFile);

  inherited;
end;

procedure TScriptThreadContainer.AddCacheFile(AFileName: String; ACode: IMemoryBlock);
begin
  FLock.Enter;
  try
    AFileName := AnsiReplaceStr(AFileName, '\', '/');
    if FCachedFile.IndexOf(AFileName) < 0 then
      FCachedFile.AddObject(AFileName, TCode.Create(ACode));
  finally
    FLock.Leave;
  end;
end;

procedure TScriptThreadContainer.GetCacheFile(AFileName: WideString;
  var ACode: IInterface; var ACached: WordBool);
var
  i: Integer;
  sFileName: String;
begin
  FLock.Enter;
  try
    sFileName := AnsiReplaceStr(AFileName, '\', '/');
    i := FCachedFile.IndexOf(sFileName);
    if i < 0 then
    begin
      ACode := nil;
      ACached := False;
    end
    else
    begin
      ACode := TCode(FCachedFile.Objects[i]).FCode;
      ACached := True;
    end;
  finally
    FLock.Leave;
  end;
end;

function TScriptThreadContainer.AddThread(AFileName, AEntryFunction: String): Integer;
begin
  FLock.Enter;
  try
    Result := FThreads.Add(TScriptThread.Create(Self, AFileName, AEntryFunction));
  finally
    FLock.Leave;
  end;
end;

procedure TScriptThreadContainer.RegFrame(AFrame: IDispatch);
begin
  FFrame := AFrame;
end;

procedure TScriptThreadContainer.Restart;
begin
  FTerminated := False;
end;

procedure TScriptThreadContainer.Resume(AIndex: Integer);
var
  i: Integer;
begin
  FLock.Enter;
  try
    for i := 0 to FThreads.Count - 1 do
    begin
      if (AIndex >= 0) and (AIndex <> i) then
        Continue;

      TThread(FThreads[i]).Resume;
    end;
  finally
    FLock.Leave;
  end;
end;

procedure TScriptThreadContainer.Terminate;
var
  i: Integer;
begin
  FLock.Enter;
  try
    FTerminated := True;

    for i := 0 to FThreads.Count - 1 do
      TScriptThread(FThreads[i]).Terminate;
  finally
    FLock.Leave;
  end;
end;

procedure TScriptThreadContainer.WaitFor;
var
  i: Integer;
  iWaitForResult: Cardinal;
  thd: TThread;
begin
  i := 0;
  thd := nil;
  while True do
  begin
    FLock.Enter;
    try
      if FThreads.Count = 0 then
        Break;

      if i >= FThreads.Count then
        i := 0;

      thd := TThread(FThreads[i]);
    finally
      FLock.Leave;
    end;

    iWaitForResult := WaitForSingleObject(thd.Handle, 100);
    Application.ProcessMessages;

    if iWaitForResult <> WAIT_TIMEOUT then
    begin
      FLock.Enter;
      try
        i := FThreads.IndexOf(thd);
        if i >= 0 then
        begin
          thd.Free;
          FThreads.Delete(i);
        end;
      finally
        FLock.Leave;
      end;
    end
    else
    begin
      Inc(i);
    end;
  end;

  FLock.Enter;
  try
    for i := FCachedFile.Count - 1 downto 0 do
      FCachedFile.Objects[i].Free;

    FCachedFile.Clear;
  finally
    FLock.Leave;
  end;
end;

class procedure TScriptThreadContainer.Init;
begin
  FLock := TCriticalSection.Create;
  FInstance := WrapperObject(TScriptThreadContainer.Create);
end;

class procedure TScriptThreadContainer.UnInit;
begin
  FInstance := nil;
  FreeAndNil(FLock);
end;

initialization
  TScriptThreadContainer.Init;

finalization
  TScriptThreadContainer.UnInit;

end.
