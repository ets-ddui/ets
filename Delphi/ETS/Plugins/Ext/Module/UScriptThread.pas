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
  //TScriptThreadContainer为单态实现
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
    //脚本中禁止调用此函数(因为执行脚本的线程也在WaitFor的范围之内)
    procedure WaitFor;
    procedure Restart;
  published
    procedure AddCacheFile(AFileName: String; ACode: IMemoryBlock);
    //返回值可用于Terminate等需要AIndex入参的函数
    function AddThread(AFileName, AEntryFunction: String): Integer;
    procedure GetCacheFile(AFileName: WideString; var ACode: IInterface; var ACached: WordBool);
    procedure RegFrame(AFrame: IDispatch);
    //AIndex含义说明：
    //-1：所有线程
    //其他值：指定某个线程
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
        GetRawManager.Service['Log'].AddLog('Script.dll加载失败');
        Exit;
      end;

      case GetScriptLanguage(FFileName) of
        slJScript: objScript := Variant(objModule).GetJScript;
        slPython: objScript := Variant(objModule).GetPython;
      else
        GetRawManager.Service['Log'].AddLog('无法识别的脚本类型');
        Exit;
      end;

      if not Assigned(objScript) then
      begin
        GetRawManager.Service['Log'].AddLog('脚本解释器初始化失败');
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
    //将核心逻辑用函数进行包装的目的是防止在执行过程中生成的临时变量，
    //在调用CoUninitialize之后才被清理
    doExecute;
  except
    GetRawManager.Service['Log'].AddLog('TScriptThread执行失败');
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
