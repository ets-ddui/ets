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
  //如果将AResult定义为函数的返回值，Delphi会将返回值转换为函数的第1个入参传入函数(会多出一个形参)，
  //导致在与C++交互时产生问题，因此，将返回值调整为形参形式
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
    //1.0 按原始路径查找
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

    //2.0 在没有扩展名的情况下，查找exe和dll文件
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
