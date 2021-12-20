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
  Forms, Classes, SyncObjs, SysUtils, Controls, Messages,
  qjson, UAppInit, UModuleBase, UMessageConst;

type
  {$METHODINFO ON}
  TLogCore = class(TModuleBase, ILog, ILogManager)
  private
    FLock: TCriticalSection;
    FValue: TList;
    FCallBack: TCallBackList;
  public //ֻ��ILog����ΪIDispatch�ӿ�
    { ILogʵ�� }
    procedure AddLog(AMessage: WideString); stdcall;
  private
    { ILogManagerʵ�� }
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
    { ISettingʵ�� }
    function _Read_GetItem(AIndex: Integer): ISetting; stdcall;
    function _Read_GetItemByPath(APath: WideString): ISetting; stdcall;
    function ISetting.GetItem = _Read_GetItem;
    function ISetting.GetItemByPath = _Read_GetItemByPath;
  public //IDispatch�ӿ�
    function GetCount: Integer; stdcall;
    function GetType: Integer; stdcall;
    function GetValue: WideString; stdcall;
    function GetValueByPath(APath: WideString; ADefault: WideString): WideString; stdcall;

    function GetItem(APath: String): IDispatch;
  private
    { ISettingManagerʵ�� }
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
  {$METHODINFO OFF}

  //TService���ڶ��̻߳�����ʹ�ã�ͨ����Ϣѭ������֤�̰߳�ȫ
  TSync = class(TWinControl)
  protected
    procedure WndProc(var AMessage: TMessage); override;
  end;

{ TLogCore }

type
  TLogItem = record
    FMessage: WideString;
  end;
  PLogItem = ^TLogItem;

constructor TLogCore.Create;
begin
  inherited;

  FValue := TList.Create;
  FCallBack := TCallBackList.Create;
  FLock := TCriticalSection.Create;
end;

destructor TLogCore.Destroy;
begin
  Clear;

  FreeAndNil(FValue);
  FreeAndNil(FCallBack);
  FreeAndNil(FLock);

  inherited;
end;

procedure TLogCore.AddLog(AMessage: WideString);
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

procedure TLogCore.Clear;
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

function TLogCore.GetCount: Integer;
begin
  FLock.Enter;
  try
    Result := FValue.Count;
  finally
    FLock.Leave;
  end;
end;

function TLogCore.GetLog(AIndex: Integer): WideString;
begin
  FLock.Enter;
  try
    Result := PLogItem(FValue[AIndex])^.FMessage;
  finally
    FLock.Leave;
  end;
end;

function TLogCore.RegistCallBack(ACallBack: ICallBack): Pointer;
begin
  FLock.Enter;
  try
    Result := FCallBack.Add(ACallBack);
  finally
    FLock.Leave;
  end;
end;

procedure TLogCore.UnRegistCallBack(AID: Pointer);
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

{ TSync }

procedure TSync.WndProc(var AMessage: TMessage);
begin
  inherited;
end;

{ TService }

constructor TService.Create(ATrayIcon: TObject);
begin
  FLog := TLogCore.Create;
  FMessageLoop := TMessageLoop.Create;
  FSetting := TSettingCore.Create(nil, nil);
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
  ReportMemoryLeaksOnShutdown := True; //�����ڴ�й¶�ļ��
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
