program ETS;

{$i UConfigure.inc}

uses
  Windows;

{$IFDEF LAZARUS}
{$R ETS_Lazarus.res}
{$ELSE}
{$R ETS_Delphi.res}
{$ENDIF}

procedure PackageInit;
type
  TProcedure = procedure;
var
  hCore: HMODULE;
  funcInit: TProcedure;
  funcEntry: TProcedure;
begin
  hCore := LoadLibrary('Core.bpl');
  if hCore = 0 then
  begin
    OutputDebugString('Core.bpl不存在');
    Exit;
  end;

  try
    @funcInit := GetProcAddress(hCore, 'Initialize');
    if not Assigned(funcInit) then
    begin
      OutputDebugString('Initialize入口函数不存在');
      Exit;
    end;
    funcInit;

    @funcEntry := GetProcAddress(hCore, 'CoreInit');
    if not Assigned(funcEntry) then
    begin
      OutputDebugString('CoreInit入口函数不存在');
      Exit;
    end;
    funcEntry;
  finally
    FreeLibrary(hCore);
  end;
end;

begin
  //添加依赖库的查找路径
  Windows.SetEnvironmentVariable('PATH', '.\Dll\Common;.\Dll\Boost;.\Dll\KDPlugins;.\Lib\JScript;.\Lib\Python');

  //加载核心包(代码从LoadPackage拷贝而来，精简了Unit重复检查的逻辑)
  PackageInit;
end.
