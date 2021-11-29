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

uses
  UInterface;

type
  {$IFDEF LAZARUS}
  {$M+}
  {$ELSE}
  {$METHODINFO ON}
  {$ENDIF}
  TService = class
  strict private
    FLog, FMessageLoop, FSetting: IDispatch;
    FTrayIcon: TObject;
  public
    constructor Create(ATrayIcon: TObject); reintroduce;
  published
    function Log: IDispatch;
    function MessageLoop: IDispatch;
    function Setting: IDispatch;
    function TrayIcon: IDispatch;
  end;
  {$IFDEF LAZARUS}
  {$M-}
  {$ELSE}
  {$METHODINFO OFF}
  {$ENDIF}

procedure CoreInit;

implementation

uses
  Forms, Classes, SyncObjs, SysUtils, qjson, UAppInit, UModuleBase, UMessageConst;

type
  {$METHODINFO ON}
  TLogManager = class(TModuleBase, ILog, ILogManager)
  private
    FLock: TCriticalSection;
    FValue: TList;
    FCallBack: TCallBackList;
  public //只将ILog开放为IDispatch接口
    { ILog实现 }
    procedure AddLog(AMessage: WideString); stdcall;
  private
    { ILogManager实现 }
    procedure Clear; stdcall;
    function GetCount: Integer; stdcall;
    function GetLog(AIndex: Integer): WideString; stdcall;
    function RegistCallBack(ACallBack: ICallBack): Pointer; stdcall;
    procedure UnRegistCallBack(AID: Pointer); stdcall;
  public
    constructor Create; override;
    destructor Destroy; override;
  end;

  TMessageLoop = class(TModuleBase)
  public
    procedure Execute;
  end;

  TSettingManager = class(TModuleBase, ISetting, ISettingManager)
  private
    FLock: TCriticalSection;
    FIsDirty: Boolean;
    FSetting: TQJson;
    FRoot: TSettingManager;
    FCallBack: TCallBackList;
    procedure SetDirty;
    function WrapItem(AJson: TQJson): TSettingManager;
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
    constructor Create(ARoot: TSettingManager; ASetting: TQJson); reintroduce;
    destructor Destroy; override;
  end;
  {$METHODINFO OFF}

{ TLogManager }

type
  TLogItem = record
    FMessage: WideString;
  end;
  PLogItem = ^TLogItem;

constructor TLogManager.Create;
begin
  inherited;

  FValue := TList.Create;
  FCallBack := TCallBackList.Create;
  FLock := TCriticalSection.Create;
end;

destructor TLogManager.Destroy;
begin
  Clear;

  FreeAndNil(FValue);
  FreeAndNil(FCallBack);
  FreeAndNil(FLock);

  inherited;
end;

procedure TLogManager.AddLog(AMessage: WideString);
var
  pli: PLogItem;
  iIndex: Integer;
begin
  FLock.Enter;
  try
    New(pli);
    pli^.FMessage := AMessage;
    iIndex := FValue.Add(pli);

    FCallBack.CallBack(SM_LOG_ADD, 0, iIndex);
  finally
    FLock.Leave;
  end;
end;

procedure TLogManager.Clear;
var
  i: Integer;
begin
  FLock.Enter;
  try
    for i := FValue.Count - 1 downto 0 do
      Dispose(FValue[i]);
    FValue.Clear;
    FCallBack.CallBack(SM_LOG_CLEAR, 0, 0);
  finally
    FLock.Leave;
  end;
end;

function TLogManager.GetCount: Integer;
begin
  FLock.Enter;
  try
    Result := FValue.Count;
  finally
    FLock.Leave;
  end;
end;

function TLogManager.GetLog(AIndex: Integer): WideString;
begin
  FLock.Enter;
  try
    Result := PLogItem(FValue[AIndex])^.FMessage;
  finally
    FLock.Leave;
  end;
end;

function TLogManager.RegistCallBack(ACallBack: ICallBack): Pointer;
begin
  FLock.Enter;
  try
    Result := FCallBack.Add(ACallBack);
  finally
    FLock.Leave;
  end;
end;

procedure TLogManager.UnRegistCallBack(AID: Pointer);
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

{ TSettingManager }

const
  CConfFile: String = '.\Config\ETS.json';

constructor TSettingManager.Create(ARoot: TSettingManager; ASetting: TQJson);
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

destructor TSettingManager.Destroy;
begin
  if not Assigned(FRoot) then
  begin
    FreeAndNil(FSetting);
    FreeAndNil(FCallBack);
    FreeAndNil(FLock);
  end;

  inherited;
end;

function TSettingManager.GetCount: Integer;
begin
  FLock.Enter;
  try
    Result := FSetting.Count;
  finally
    FLock.Leave;
  end;
end;

function TSettingManager.GetItem(APath: String): IDispatch;
begin
  Result := _Read_GetItemByPath(APath) as IDispatch;
end;

function TSettingManager.GetObject: Cardinal;
begin
  Result := Cardinal(FSetting);
end;

function TSettingManager.GetType: Integer;
begin
  FLock.Enter;
  try
    Result := Ord(FSetting.DataType);
  finally
    FLock.Leave;
  end;
end;

function TSettingManager.GetValue: WideString;
begin
  FLock.Enter;
  try
    Result := FSetting.Value;
  finally
    FLock.Leave;
  end;
end;

function TSettingManager.GetValueByPath(APath, ADefault: WideString): WideString;
begin
  FLock.Enter;
  try
    Result := FSetting.ValueByPath(APath, ADefault);
  finally
    FLock.Leave;
  end;
end;

function TSettingManager.IsDirty: Boolean;
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

function TSettingManager.WrapItem(AJson: TQJson): TSettingManager;
begin
  if not Assigned(AJson) then
    Result := nil
  else if Assigned(FRoot) then
    Result := TSettingManager.Create(FRoot, AJson)
  else
    Result := TSettingManager.Create(Self, AJson);
end;

function TSettingManager._Read_GetItem(AIndex: Integer): ISetting;
begin
  FLock.Enter;
  try
    Result := WrapItem(FSetting[AIndex]);
  finally
    FLock.Leave;
  end;
end;

function TSettingManager._Read_GetItemByPath(APath: WideString): ISetting;
begin
  FLock.Enter;
  try
    Result := WrapItem(FSetting.ItemByPath(APath));
  finally
    FLock.Leave;
  end;
end;

function TSettingManager._Manager_GetItem(AIndex: Integer): ISettingManager;
begin
  Result := _Read_GetItem(AIndex) as ISettingManager;
end;

function TSettingManager._Manager_GetItemByPath(APath: WideString): ISettingManager;
begin
  Result := _Read_GetItemByPath(APath) as ISettingManager;
end;

procedure TSettingManager.Save;
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

procedure TSettingManager.SetDirty;
begin
  if Assigned(FRoot) then
    FRoot.SetDirty
  else
    FIsDirty := True;
end;

procedure TSettingManager.SetValue(AValue: WideString);
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

procedure TSettingManager.SetValueByPath(APath, AValue: WideString);
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

function TSettingManager.RegistCallBack(ACallBack: ICallBack): Pointer;
begin
  FLock.Enter;
  try
    Result := FCallBack.Add(ACallBack);
  finally
    FLock.Leave;
  end;
end;

procedure TSettingManager.UnRegistCallBack(AID: Pointer);
begin
  FLock.Enter;
  try
    FCallBack.Delete(AID);
  finally
    FLock.Leave;
  end;
end;

{ TModule }

constructor TService.Create(ATrayIcon: TObject);
begin
  FLog := TLogManager.Create;
  FMessageLoop := TMessageLoop.Create;
  FSetting := TSettingManager.Create(nil, nil);
  FTrayIcon := ATrayIcon;
end;

function TService.Log: IDispatch;
begin
  Result := FLog;
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
