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
  qjson, UAppInit, UModuleBase, UMessageConst, UInterface, UBindEvent;

type
  TLogList = class
  strict private
    FLock: TCriticalSection;
    FValue: TList;
  private
    procedure AddLog(AMessage: WideString);
    procedure Clear;
    function GetCount: Integer;
    function GetLog(AIndex: Integer): WideString;
  public
    constructor Create; reintroduce;
    destructor Destroy; override;
  end;

  TSettingCore = class(TModuleBase, ISetting, ISettingManager)
  private
    FLock: TCriticalSection;
    FIsDirty: Boolean;
    FSetting: TQJson;
    FRoot: TSettingCore;
    FCallBack: TCallBackList;
    procedure SetDirty;
    function WrapItem(AJson: TQJson): TSettingCore;
  private
    { ISetting实现 }
    function _Read_GetItem(AIndex: Integer): ISetting; stdcall;
    function _Read_GetItemByPath(APath: WideString): ISetting; stdcall;
    function ISetting.GetItem = _Read_GetItem;
    function ISetting.GetItemByPath = _Read_GetItemByPath;
  public //IDispatch接口
    function GetCount: Integer; stdcall;
    function GetType: Integer; stdcall;
    function GetValue: WideString; stdcall;
    function GetValueByPath(APath: WideString; ADefault: WideString): WideString; stdcall;

    function GetItem(APath: String): IDispatch;
  private
    { ISettingManager实现 }
    function _Manager_GetItem(AIndex: Integer): ISettingManager; stdcall;
    function _Manager_GetItemByPath(APath: WideString): ISettingManager; stdcall;
    function ISettingManager.GetItem = _Manager_GetItem;
    function ISettingManager.GetItemByPath = _Manager_GetItemByPath;
    function GetObject: Cardinal; stdcall;
    procedure SetValue(AValue: WideString); stdcall;
    procedure SetValueByPath(APath: WideString; AValue: WideString); stdcall;
    function IsDirty: Boolean; stdcall;
    procedure Save; stdcall;
    function RegistCallBack(ACallBack: ICallBack): Pointer; stdcall;
    procedure UnRegistCallBack(AID: Pointer); stdcall;
  public
    constructor Create(ARoot: TSettingCore; ASetting: TQJson); reintroduce;
    destructor Destroy; override;
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
    FLock: TCriticalSection;
    FCallBack: TStringList;
  private
    procedure DoCallBack(AMessage: WideString);
  public
    constructor Create(AService: TService); reintroduce;
    destructor Destroy; override;
  public
    procedure Clear;
    function GetCount: Integer;
    function GetLog(AIndex: Integer): WideString;
    function RegistCallBack(ACallBack: IDispatch): Integer;
    procedure UnRegistCallBack(AID: Integer);
  end;

  TMessageLoop = class(TModuleBase)
  public
    procedure Execute;
  end;

  TService = class
  strict private
    FMessageLoop, FSetting: IDispatch;
    FLog: TLog;
    FLogManager: TLogManager;
    FLogList: TLogList;
    FTrayIcon: TObject;
    FSync: TWinControl;
    FLastMessage: Integer;
    procedure DoCallBack(var AMessage: TMessage);
  private
    procedure CallBack(AMessage: Integer);
    property LogList: TLogList read FLogList;
  public
    constructor Create(ATrayIcon: TObject); reintroduce;
    destructor Destroy; override;
  public
    function Log: IDispatch;
    function LogManager: IDispatch;
    function MessageLoop: IDispatch;
    function Setting: IDispatch;
    function TrayIcon: IDispatch;
  end;
  {$METHODINFO OFF}

const
  LOG_ADD = CM_BASE - 1000;
  LOG_CLEAR = CM_BASE - 1001;

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
end;

destructor TLogList.Destroy;
begin
  Clear;

  FreeAndNil(FValue);
  FreeAndNil(FLock);

  inherited;
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
  FService := AService;
end;

procedure TLog.AddLog(AMessage: WideString);
begin
  FService.LogList.AddLog(AMessage);
  FService.CallBack(LOG_ADD);
end;

{ TLogManager }

type
  TOnCallBack = procedure (AMessage: WideString) of object;
  TCallBack = class(TComponent)
  private
    FOnCallBack: TOnCallBack;
  published
    property OnCallBack: TOnCallBack read FOnCallBack write FOnCallBack;
  end;

constructor TLogManager.Create(AService: TService);
begin
  FService := AService;
  FLock := TCriticalSection.Create;
  FCallBack := TStringList.Create;
end;

destructor TLogManager.Destroy;
var
  i: Integer;
begin
  for i := FCallBack.Count - 1 downto 0 do
    FCallBack.Objects[i].Free;
  FreeAndNil(FCallBack);

  FreeAndNil(FLock);

  inherited;
end;

procedure TLogManager.Clear;
begin
  FService.LogList.Clear;
  FService.CallBack(LOG_CLEAR);
end;

procedure TLogManager.DoCallBack(AMessage: WideString);
var
  i: Integer;
  obj: TCallBack;
begin
  i := 0;
  while True do
  begin
    FLock.Enter;
    try
      if i >= FCallBack.Count then
        Exit;

      obj := TCallBack(FCallBack.Objects[i]);
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

function TLogManager.GetCount: Integer;
begin
  Result := FService.LogList.GetCount;
end;

function TLogManager.GetLog(AIndex: Integer): WideString;
begin
  Result := FService.LogList.GetLog(AIndex);
end;

function TLogManager.RegistCallBack(ACallBack: IDispatch): Integer;
var
  obj: TCallBack;
begin
  FLock.Enter;
  try
    obj := TCallBack.Create(nil);
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

procedure TLogManager.UnRegistCallBack(AID: Integer);
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

{ TSettingCore }

const
  CConfFile: String = '.\Config\ETS.json';

constructor TSettingCore.Create(ARoot: TSettingCore; ASetting: TQJson);
begin
  inherited Create;

  FIsDirty := False;
  if not Assigned(ARoot) then
  begin
    FRoot := nil;
    FSetting := TQJson.Create;
    FSetting.LoadFromFile(CConfFile);
    FCallBack := TCallBackList.Create;
    FLock := TCriticalSection.Create;
  end
  else
  begin
    FRoot := ARoot;
    FSetting := ASetting;
    FCallBack := ARoot.FCallBack;
    FLock := ARoot.FLock;
  end;
end;

destructor TSettingCore.Destroy;
begin
  if not Assigned(FRoot) then
  begin
    FreeAndNil(FSetting);
    FreeAndNil(FCallBack);
    FreeAndNil(FLock);
  end;

  inherited;
end;

function TSettingCore.GetCount: Integer;
begin
  FLock.Enter;
  try
    Result := FSetting.Count;
  finally
    FLock.Leave;
  end;
end;

function TSettingCore.GetItem(APath: String): IDispatch;
begin
  Result := _Read_GetItemByPath(APath) as IDispatch;
end;

function TSettingCore.GetObject: Cardinal;
begin
  Result := Cardinal(FSetting);
end;

function TSettingCore.GetType: Integer;
begin
  FLock.Enter;
  try
    Result := Ord(FSetting.DataType);
  finally
    FLock.Leave;
  end;
end;

function TSettingCore.GetValue: WideString;
begin
  FLock.Enter;
  try
    Result := FSetting.Value;
  finally
    FLock.Leave;
  end;
end;

function TSettingCore.GetValueByPath(APath, ADefault: WideString): WideString;
begin
  FLock.Enter;
  try
    Result := FSetting.ValueByPath(APath, ADefault);
  finally
    FLock.Leave;
  end;
end;

function TSettingCore.IsDirty: Boolean;
begin
  FLock.Enter;
  try
    if Assigned(FRoot) then
      Result := FRoot.IsDirty
    else
      Result := FIsDirty;
  finally
    FLock.Leave;
  end;
end;

function TSettingCore.WrapItem(AJson: TQJson): TSettingCore;
begin
  if not Assigned(AJson) then
    Result := nil
  else if Assigned(FRoot) then
    Result := TSettingCore.Create(FRoot, AJson)
  else
    Result := TSettingCore.Create(Self, AJson);
end;

function TSettingCore._Read_GetItem(AIndex: Integer): ISetting;
begin
  FLock.Enter;
  try
    Result := WrapItem(FSetting[AIndex]);
  finally
    FLock.Leave;
  end;
end;

function TSettingCore._Read_GetItemByPath(APath: WideString): ISetting;
begin
  FLock.Enter;
  try
    Result := WrapItem(FSetting.ItemByPath(APath));
  finally
    FLock.Leave;
  end;
end;

function TSettingCore._Manager_GetItem(AIndex: Integer): ISettingManager;
begin
  Result := _Read_GetItem(AIndex) as ISettingManager;
end;

function TSettingCore._Manager_GetItemByPath(APath: WideString): ISettingManager;
begin
  Result := _Read_GetItemByPath(APath) as ISettingManager;
end;

procedure TSettingCore.Save;
begin
  FLock.Enter;
  try
    if Assigned(FRoot) then
      FRoot.Save
    else
    begin
      if not FIsDirty then
        Exit;

      FSetting.SaveToFile(CConfFile);
      FIsDirty := False;
    end;
  finally
    FLock.Leave;
  end;
end;

procedure TSettingCore.SetDirty;
begin
  if Assigned(FRoot) then
    FRoot.SetDirty
  else
    FIsDirty := True;
end;

procedure TSettingCore.SetValue(AValue: WideString);
begin
  FLock.Enter;
  try
    if FSetting.Value <> AValue then
    begin
      FSetting.Value := AValue;
      SetDirty;

      FCallBack.CallBack(SM_SETTING_CHANGED, 0, Integer(FSetting));
    end;
  finally
    FLock.Leave;
  end;
end;

procedure TSettingCore.SetValueByPath(APath, AValue: WideString);
var
  json: TQJson;
begin
  FLock.Enter;
  try
    json := FSetting.ItemByPath(APath);
    if Assigned(json) and (json.Value <> AValue) then
    begin
      json.Value := AValue;
      SetDirty;

      FCallBack.CallBack(SM_SETTING_CHANGED, 0, Integer(json));
    end;
  finally
    FLock.Leave;
  end;
end;

function TSettingCore.RegistCallBack(ACallBack: ICallBack): Pointer;
begin
  FLock.Enter;
  try
    Result := FCallBack.Add(ACallBack);
  finally
    FLock.Leave;
  end;
end;

procedure TSettingCore.UnRegistCallBack(AID: Pointer);
begin
  FLock.Enter;
  try
    FCallBack.Delete(AID);
  finally
    FLock.Leave;
  end;
end;

{ TMessageLoop }

procedure TMessageLoop.Execute;
begin
  Application.ProcessMessages;
end;

{ TService }

constructor TService.Create(ATrayIcon: TObject);
begin
  FSync := TWinControl.Create(nil);
  FSync.WindowProc := DoCallBack;
  FLogList := TLogList.Create;

  //TService内部以对象的形式管理FLog、FLogManager，但以接口形式对外暴露接口
  //为防止引用计数归零导致对象被释放，这里将引用计数加1
  //析构时使用FreeAndNil释放对象
  FLog := TLog.Create(Self);
  IInterface(FLog)._AddRef;
  FLogManager := TLogManager.Create(Self);
  IInterface(FLogManager)._AddRef;

  FMessageLoop := TMessageLoop.Create;
  FSetting := TSettingCore.Create(nil, nil);
  FTrayIcon := ATrayIcon;

  FLastMessage := 0;
end;

destructor TService.Destroy;
begin
  FreeAndNil(FLog);
  FreeAndNil(FLogManager);
  FreeAndNil(FLogList);
  FreeAndNil(FSync);

  inherited;
end;

procedure TService.DoCallBack(var AMessage: TMessage);
begin
  InterlockedExchange(FLastMessage, 0);

  case AMessage.Msg of
    LOG_ADD:
    begin
      FLogManager.DoCallBack('LOG_ADD');
    end;
    LOG_CLEAR:
    begin
      FLogManager.DoCallBack('LOG_CLEAR');
    end;  
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
    PostMessage(FSync.Handle, AMessage, 0, 0);
  end;
end;

function TService.Log: IDispatch;
begin
  Result := FLog;
end;

function TService.LogManager: IDispatch;
begin
  Result := FLogManager;
end;

function TService.MessageLoop: IDispatch;
begin
  Result := FMessageLoop;
end;

function TService.Setting: IDispatch;
begin
  Result := FSetting;
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
