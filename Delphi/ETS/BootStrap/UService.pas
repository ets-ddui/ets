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
unit UService;

{$i UConfigure.inc}

interface

function CreateService(ATrayIcon: TObject): TObject;
procedure CoreInit;

implementation

uses
  Windows, Forms, Classes, SyncObjs, SysUtils, Controls, Messages,
  qjson, UAppInit, UModuleBase, UInterface, UBindEvent;

type
  TCallBack = class
  strict private
    FLock: TCriticalSection;
    FCallBack: TStringList;
  public
    constructor Create;
    destructor Destroy; override;
    function Add(ACallBack: IDispatch): Integer;
    procedure Delete(AID: Integer);
    procedure Execute(AMessage: String);
  end;

  TLogList = class
  strict private
    FLock: TCriticalSection;
    FCallBack: TCallBack;
    FValue: TList;
    function GetCount: Integer;
  public
    constructor Create;
    destructor Destroy; override;
    procedure DoCallBack(AMessage: String);
    procedure AddLog(AMessage: WideString);
    procedure Clear;
    function GetLog(AIndex: Integer): WideString;
    property CallBack: TCallBack read FCallBack;
    property Count: Integer read GetCount;
  end;

  TSettingCore = class
  strict private
    FLock: TCriticalSection;
    FCallBack: TCallBack;
    FConfig: TQJson;
    FDirty: Boolean;
  public
    constructor Create;
    destructor Destroy; override;
    procedure DoCallBack(AMessage: String);
    procedure Enter;
    procedure Leave;
    procedure Save;
    property CallBack: TCallBack read FCallBack;
    property Config: TQJson read FConfig;
  end;

  {$METHODINFO ON}
  TService = class;

  TLog = class(TModuleBase)
  strict private
    FService: TService;
  public
    constructor Create(AService: TService); reintroduce;
  public
    procedure AddLog(AMessage: WideString);
  end;

  TLogManager = class(TModuleBase)
  strict private
    FService: TService;
    function GetCount: Integer;
  public
    constructor Create(AService: TService); reintroduce;
  public
    procedure Clear;
    function GetLog(AIndex: Integer): WideString;
    function RegistCallBack(ACallBack: IDispatch): Integer;
    procedure UnRegistCallBack(AID: Integer);
  published
    property Count: Integer read GetCount;
  end;

  TSetting = class(TModuleBase)
  private
    FService: TService;
    FNode: TQJson;
    function GetCount: Integer;
    function GetValue: String;
  public
    constructor Create(AService: TService; ANode: TQJson = nil); reintroduce;
  public
    function GetItem(APath: String): IDispatch;
    function GetValueByPath(APath, ADefault: String): String;
  published
    property Count: Integer read GetCount;
    property Value: String read GetValue;
  end;

  TSettingManager = class(TSetting)
  strict private
    function GetValue: String;
    procedure SetValue(AValue: String);
  public
    function GetItem(APath: String): IDispatch;
    procedure Save;
    procedure SetValueByPath(APath, AValue: String);
    function RegistCallBack(ACallBack: IDispatch): Integer;
    procedure UnRegistCallBack(AID: Integer);
  published
    property Value: String read GetValue write SetValue;
  end;

  TMessageLoop = class(TModuleBase)
  public
    procedure Execute;
  end;

  TService = class
  strict private
    FLogList: TLogList;
    FSettingCore: TSettingCore;
    FMessageLoop: IDispatch;
    FTrayIcon: TObject;
    FSync: HWND;
    FLastMessage: Integer;
    procedure DoCallBack(var AMessage: TMessage);
  private
    procedure CallBack(AMessage: Integer);
    property LogList: TLogList read FLogList;
    property SettingCore: TSettingCore read FSettingCore;
  public
    constructor Create(ATrayIcon: TObject); reintroduce;
    destructor Destroy; override;
  public
    function Log: IDispatch;
    function LogManager: IDispatch;
    function Setting: IDispatch;
    function SettingManager: IDispatch;
    function MessageLoop: IDispatch;
    function TrayIcon: IDispatch;
  end;
  {$METHODINFO OFF}

const
  LOG_ADD = CM_BASE - 1000;
  LOG_CLEAR = CM_BASE - 1001;
  SETTING_CHANGED = CM_BASE - 1002;

{ TCallBack }

type
  TOnCallBack = procedure (AMessage: String) of object;
  TCallBackItem = class(TComponent)
  private
    FOnCallBack: TOnCallBack;
  published
    property OnCallBack: TOnCallBack read FOnCallBack write FOnCallBack;
  end;

constructor TCallBack.Create;
begin
  FLock := TCriticalSection.Create;
  FCallBack := TStringList.Create;
end;

destructor TCallBack.Destroy;
var
  i: Integer;
begin
  for i := FCallBack.Count - 1 downto 0 do
    FCallBack.Objects[i].Free;
  FreeAndNil(FCallBack);
  FreeAndNil(FLock);

  inherited;
end;

function TCallBack.Add(ACallBack: IDispatch): Integer;
var
  obj: TCallBackItem;
begin
  FLock.Enter;
  try
    obj := TCallBackItem.Create(nil);
    if not TBindEvent.Create(obj, 'OnCallBack', ACallBack).Valid then
    begin
      Result := -1;
      FreeAndNil(obj);
      Exit;
    end;

    for Result := FCallBack.Count - 1 downto 0 do
      if not Assigned(FCallBack.Objects[Result]) then
      begin
        FCallBack.Objects[Result] := obj;
        Exit;
      end;

    Result:= FCallBack.Count;
    FCallBack.AddObject('', obj);
  finally
    FLock.Leave;
  end;
end;

procedure TCallBack.Delete(AID: Integer);
begin
  FLock.Enter;
  try
    if (AID < 0) or (AID >= FCallBack.Count) then
      Exit;

    FCallBack.Objects[AID].Free;
    FCallBack.Objects[AID] := nil;
  finally
    FLock.Leave;
  end;
end;

procedure TCallBack.Execute(AMessage: String);
var
  i: Integer;
  obj: TCallBackItem;
begin
  i := 0;
  while True do
  begin
    FLock.Enter;
    try
      if i >= FCallBack.Count then
        Exit;

      obj := TCallBackItem(FCallBack.Objects[i]);
    finally
      FLock.Leave;
    end;

    if Assigned(obj) and Assigned(obj.OnCallBack) then
    begin
      obj.OnCallBack(AMessage);
    end;

    Inc(i);
  end;
end;

{ TLogCore }

type
  TLogItem = record
    FMessage: WideString;
  end;
  PLogItem = ^TLogItem;

constructor TLogList.Create;
begin
  FValue := TList.Create;
  FLock := TCriticalSection.Create;
  FCallBack := TCallBack.Create;
end;

destructor TLogList.Destroy;
begin
  Clear;

  FreeAndNil(FCallBack);
  FreeAndNil(FValue);
  FreeAndNil(FLock);

  inherited;
end;

procedure TLogList.DoCallBack(AMessage: String);
begin
  FCallBack.Execute(AMessage);
end;

procedure TLogList.AddLog(AMessage: WideString);
var
  pli: PLogItem;
begin
  FLock.Enter;
  try
    New(pli);
    pli^.FMessage := AMessage;
    FValue.Add(pli);
  finally
    FLock.Leave;
  end;
end;

procedure TLogList.Clear;
var
  i: Integer;
begin
  FLock.Enter;
  try
    for i := FValue.Count - 1 downto 0 do
      Dispose(FValue[i]);
    FValue.Clear;
  finally
    FLock.Leave;
  end;
end;

function TLogList.GetCount: Integer;
begin
  FLock.Enter;
  try
    Result := FValue.Count;
  finally
    FLock.Leave;
  end;
end;

function TLogList.GetLog(AIndex: Integer): WideString;
begin
  FLock.Enter;
  try
    Result := PLogItem(FValue[AIndex])^.FMessage;
  finally
    FLock.Leave;
  end;
end;

{ TLog }

constructor TLog.Create(AService: TService);
begin
  inherited Create;

  FService := AService;
end;

procedure TLog.AddLog(AMessage: WideString);
begin
  FService.LogList.AddLog(AMessage);
  FService.CallBack(LOG_ADD);
end;

{ TLogManager }

constructor TLogManager.Create(AService: TService);
begin
  inherited Create;

  FService := AService;
end;

procedure TLogManager.Clear;
begin
  FService.LogList.Clear;
  FService.CallBack(LOG_CLEAR);
end;

function TLogManager.GetCount: Integer;
begin
  Result := FService.LogList.Count;
end;

function TLogManager.GetLog(AIndex: Integer): WideString;
begin
  Result := FService.LogList.GetLog(AIndex);
end;

function TLogManager.RegistCallBack(ACallBack: IDispatch): Integer;
begin
  Result := FService.LogList.CallBack.Add(ACallBack);
end;

procedure TLogManager.UnRegistCallBack(AID: Integer);
begin
  FService.LogList.CallBack.Delete(AID);
end;

{ TSettingItem }

const
  CConfFile: String = '.\Config\ETS.json';

constructor TSettingCore.Create;
begin
  inherited Create;

  FDirty := False;
  FConfig := TQJson.Create;
  FConfig.LoadFromFile(CConfFile);
  FCallBack := TCallBack.Create;
  FLock := TCriticalSection.Create;
end;

destructor TSettingCore.Destroy;
begin
  FreeAndNil(FLock);
  FreeAndNil(FCallBack);
  FreeAndNil(FConfig);

  inherited;
end;

procedure TSettingCore.DoCallBack(AMessage: String);
begin
  FDirty := True;
  FCallBack.Execute(AMessage);
end;

procedure TSettingCore.Enter;
begin
  FLock.Enter;
end;

procedure TSettingCore.Leave;
begin
  FLock.Leave;
end;

procedure TSettingCore.Save;
begin
  FLock.Enter;
  try
    if not FDirty then
      Exit;

    FConfig.SaveToFile(CConfFile);
    FDirty := False;
  finally
    FLock.Leave;
  end;
end;

{ TSetting }

constructor TSetting.Create(AService: TService; ANode: TQJson);
begin
  inherited Create;

  FService := AService;
  if Assigned(ANode) then
    FNode := ANode
  else
    FNode := FService.SettingCore.Config;
end;

function TSetting.GetCount: Integer;
begin
  FService.SettingCore.Enter;
  try
    Result := FNode.Count;
  finally
    FService.SettingCore.Leave;
  end;
end;

function TSetting.GetItem(APath: String): IDispatch;
var
  js: TQJson;
begin
  FService.SettingCore.Enter;
  try
    js := FNode.ItemByPath(APath);

    if not Assigned(js) then
      Result := nil
    else
      Result := TSetting.Create(FService, js);
  finally
    FService.SettingCore.Leave;
  end;
end;

function TSetting.GetValue: String;
begin
  FService.SettingCore.Enter;
  try
    Result := FNode.Value;
  finally
    FService.SettingCore.Leave;
  end;
end;

function TSetting.GetValueByPath(APath, ADefault: String): String;
begin
  FService.SettingCore.Enter;
  try
    Result := FNode.ValueByPath(APath, ADefault);
  finally
    FService.SettingCore.Leave;
  end;
end;

{ TSettingManager }

function TSettingManager.GetItem(APath: String): IDispatch;
var
  js: TQJson;
begin
  FService.SettingCore.Enter;
  try
    js := FNode.ItemByPath(APath);

    if not Assigned(js) then
      Result := nil
    else
      Result := TSettingManager.Create(FService, js);
  finally
    FService.SettingCore.Leave;
  end;
end;

procedure TSettingManager.Save;
begin
  FService.SettingCore.Save;
end;

procedure TSettingManager.SetValueByPath(APath, AValue: String);
var
  json: TQJson;
begin
  FService.SettingCore.Enter;
  try
    json := FNode.ItemByPath(APath);
    if Assigned(json) and (json.Value <> AValue) then
    begin
      json.Value := AValue;
      FService.CallBack(SETTING_CHANGED);
    end;
  finally
    FService.SettingCore.Leave;
  end;
end;

function TSettingManager.GetValue: String;
begin
  Result := inherited GetValue;
end;

procedure TSettingManager.SetValue(AValue: String);
begin
  FService.SettingCore.Enter;
  try
    if FNode.Value <> AValue then
    begin
      FNode.Value := AValue;
      FService.CallBack(SETTING_CHANGED);
    end;
  finally
    FService.SettingCore.Leave;
  end;
end;

function TSettingManager.RegistCallBack(ACallBack: IDispatch): Integer;
begin
  Result := FService.SettingCore.CallBack.Add(ACallBack);
end;

procedure TSettingManager.UnRegistCallBack(AID: Integer);
begin
  FService.SettingCore.CallBack.Delete(AID);
end;

{ TMessageLoop }

procedure TMessageLoop.Execute;
begin
  Application.ProcessMessages;
end;

{ TService }

constructor TService.Create(ATrayIcon: TObject);
begin
  FLogList := TLogList.Create;
  FSettingCore := TSettingCore.Create;
  FMessageLoop := TMessageLoop.Create;
  FTrayIcon := ATrayIcon;

  FSync := Classes.AllocateHWnd(DoCallBack);
  FLastMessage := 0;
end;

destructor TService.Destroy;
begin
  FreeAndNil(FLogList);
  FreeAndNil(FSettingCore);

  Classes.DeallocateHWnd(FSync);
  FSync := 0;

  inherited;
end;

procedure TService.DoCallBack(var AMessage: TMessage);
begin
  InterlockedExchange(FLastMessage, 0);

  case AMessage.Msg of
    LOG_ADD: FLogList.DoCallBack('LOG_ADD');
    LOG_CLEAR: FLogList.DoCallBack('LOG_CLEAR');
    SETTING_CHANGED: FSettingCore.DoCallBack('SETTING_CHANGED');
  end;
end;

procedure TService.CallBack(AMessage: Integer);
var
  msg: TMessage;
begin
  if InterlockedExchange(FLastMessage, AMessage) = AMessage then
    Exit;

  if GetCurrentThreadID = MainThreadID then
  begin
    FillChar(msg, SizeOf(msg), 0);
    msg.Msg := AMessage;

    DoCallBack(msg);
  end
  else
  begin
    PostMessage(FSync, AMessage, 0, 0);
  end;
end;

function TService.Log: IDispatch;
begin
  Result := TLog.Create(Self);
end;

function TService.LogManager: IDispatch;
begin
  Result := TLogManager.Create(Self);
end;

function TService.MessageLoop: IDispatch;
begin
  Result := FMessageLoop;
end;

function TService.Setting: IDispatch;
begin
  Result := TSetting.Create(Self);
end;

function TService.SettingManager: IDispatch;
begin
  Result := TSettingManager.Create(Self);
end;

function TService.TrayIcon: IDispatch;
begin
  Result := WrapperObject(FTrayIcon, False);
end;

function CreateService(ATrayIcon: TObject): TObject;
begin
  Result := TService.Create(ATrayIcon);
end;

procedure CoreInit;
begin
  {$IFDEF LAZARUS}
  RequireDerivedFormResource := True;
  {$ELSE}
  ReportMemoryLeaksOnShutdown := True; //开启内存泄露的检查
  {$ENDIF}

  Application.Initialize;
  TAppInit.Init(False);
  try
    Application.Run;
  finally
    TAppInit.UnInit;
  end;
end;

exports
  CoreInit;

end.
