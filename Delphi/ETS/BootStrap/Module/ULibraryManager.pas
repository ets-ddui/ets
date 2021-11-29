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
unit ULibraryManager;

{$i UConfigure.inc}

interface

uses
  Classes, SyncObjs, UInterface;

type
  TLibraryManager = class
  private
    class var FPlugins: TStringList;
    class var FLock: TCriticalSection;
    class var FPluginSearchPath: String;
    class function DoLoad(AFileName: String): TObject;
  public
    class procedure Init;
    class procedure UnInit(AOnlyPlugins: Boolean);
    class function LoadFrame(AFileName, AFrameID: String): IChild;
    class function LoadModule(AFileName: String): IDispatch;
    class function GetPluginSearchPath: String;
    class procedure SetPluginSearchPath(AValue: String);
  end;

implementation

uses
  Windows, SysUtils, UTool, UAppInit;

function TranslateFileName(AFileName: String): String;
begin
  {$IFDEF LAZARUS}
  Result := FormatEh(AFileName, 'CompilerFlag=L');
  {$ELSE}
  Result := FormatEh(AFileName, 'CompilerFlag=D');
  {$ENDIF}
end;

type
  TFrameEntry = function(AManager: IManager; AID: String; AOwner: TComponent; var AResult: IChild): HRESULT; stdcall;
  //�����AResult����Ϊ�����ķ���ֵ��Delphi�Ὣ����ֵת��Ϊ�����ĵ�1����δ��뺯��(����һ���β�)��
  //��������C++����ʱ�������⣬��ˣ�������ֵ����Ϊ�β���ʽ
  TModuleEntry = function(AManager: IManager; var AResult: IDispatch): HRESULT; stdcall;
  TPluginObject = class
  private
    FFileHandle: HMODULE;
    FFileName: String;
    FFrameEntry: TFrameEntry;
    FModuleEntry: TModuleEntry;
    FModuleObject: IDispatch;
  public
    constructor Create(AFileName: String);
    destructor Destroy; override;
    function Load: Boolean;
  end;

{ TPluginObject }

constructor TPluginObject.Create(AFileName: String);
begin
  FFileHandle := 0;
  FFileName := AFileName;
  FFrameEntry := nil;
  FModuleEntry := nil;
  FModuleObject := nil;
end;

destructor TPluginObject.Destroy;
var
  iHandle: HMODULE;
begin
  if FFileHandle <> 0 then
  begin
    FModuleObject := nil;

    iHandle := FFileHandle;
    FFileHandle := 0;
    Windows.FreeLibrary(iHandle);
  end;
end;

function TPluginObject.Load: Boolean;
begin
  FFileHandle := Windows.LoadLibrary(PAnsiChar(FFileName));
  Result := FFileHandle <> 0;
  if Result then
  begin
    FFrameEntry := Windows.GetProcAddress(FFileHandle, 'GetFrame');
    if Assigned(FFrameEntry) then
    begin
      TAppInit.AddControlAtom(Format('ControlOfs%.8X%.8X', [FFileHandle, GetCurrentThreadID]));
    end;

    FModuleEntry := Windows.GetProcAddress(FFileHandle, 'GetModule');
    if Assigned(FModuleEntry) then
    begin
      if Failed(FModuleEntry(GetManager, FModuleObject)) then
      begin
        FModuleEntry := nil;
        FModuleObject := nil;
      end;
    end;
  end;
end;

{ TLibraryManager }

class function TLibraryManager.DoLoad(AFileName: String): TObject;
  function searchFileName(AFileName: String): String;
  const
    cExt: array[0..1] of String = ('.exe', '.dll');
  var
    i: Integer;
  begin
    //1.0 ��ԭʼ·������
    Result := AFileName;
    if FileExists(Result) then
      Exit;

    Result := TranslateFileName(Format('%s/%s', [FPluginSearchPath, AFileName]));
    if FileExists(Result) then
      Exit;

    if LastDelimiter('.', AFileName) > 0 then
    begin
      Result := '';
      Exit;
    end;

    //2.0 ��û����չ��������£�����exe��dll�ļ�
    for i := Low(cExt) to High(cExt) do
    begin
      Result := AFileName + cExt[i];
      if FileExists(Result) then
        Exit;

      Result := TranslateFileName(Format('%s/%s%s', [FPluginSearchPath, AFileName, cExt[i]]));
      if FileExists(Result) then
        Exit;
    end;

    Result := '';
  end;
var
  i: Integer;
  strFileName: String;
  po: TPluginObject;
begin
  Result := nil;

  strFileName := searchFileName(AFileName);
  if strFileName = '' then
    Exit;

  FLock.Enter;
  try
    i := FPlugins.IndexOf(strFileName);
    if i < 0 then
    begin
      po := TPluginObject.Create(strFileName);
      if not po.Load then
      begin
        FreeAndNil(po);
        Exit;
      end;
      i := FPlugins.AddObject(strFileName, po);
    end;
  finally
    FLock.Leave;
  end;

  Result := FPlugins.Objects[i];
end;

class function TLibraryManager.GetPluginSearchPath: String;
begin
  Result := FPluginSearchPath;
end;

class procedure TLibraryManager.Init;
begin
  FLock := SyncObjs.TCriticalSection.Create;
  FPlugins := TStringList.Create;
  FPlugins.Sorted := True;
  FPluginSearchPath := 'Dll/Plugin.{CompilerFlag}';
end;

class function TLibraryManager.LoadFrame(AFileName, AFrameID: String): IChild;
var
  po: TPluginObject;
begin
  Result := nil;
  po := TPluginObject(DoLoad(AFileName));
  if not Assigned(po) or not Assigned(po.FFrameEntry) then
    Exit;

  if Failed(po.FFrameEntry(GetManager, AFrameID, nil, Result)) then
    Result := nil;
end;

class function TLibraryManager.LoadModule(AFileName: String): IDispatch;
var
  po: TPluginObject;
begin
  Result := nil;
  po := TPluginObject(DoLoad(AFileName));
  if not Assigned(po) then
    Exit;

  Result := po.FModuleObject;
end;

class procedure TLibraryManager.SetPluginSearchPath(AValue: String);
begin
  FPluginSearchPath := AValue;
end;

class procedure TLibraryManager.UnInit(AOnlyPlugins: Boolean);
var
  i: Integer;
begin
  FLock.Enter;
  try
    for i := FPlugins.Count - 1 downto 0 do
      FPlugins.Objects[i].Free;
    FPlugins.Clear;
  finally
    FLock.Leave;
  end;

  if AOnlyPlugins then
    Exit;

  FreeAndNil(FPlugins);
  FreeAndNil(FLock);
end;

initialization
  TLibraryManager.Init;

finalization
  TLibraryManager.UnInit(False);

end.
