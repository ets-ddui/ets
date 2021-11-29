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
    OutputDebugString('Core.bpl������');
    Exit;
  end;

  try
    @funcInit := GetProcAddress(hCore, 'Initialize');
    if not Assigned(funcInit) then
    begin
      OutputDebugString('Initialize��ں���������');
      Exit;
    end;
    funcInit;

    @funcEntry := GetProcAddress(hCore, 'CoreInit');
    if not Assigned(funcEntry) then
    begin
      OutputDebugString('CoreInit��ں���������');
      Exit;
    end;
    funcEntry;
  finally
    FreeLibrary(hCore);
  end;
end;

begin
  //���������Ĳ���·��
  Windows.SetEnvironmentVariable('PATH', '.\Dll\Common;.\Dll\Boost;.\Dll\KDPlugins;.\Lib\JScript;.\Lib\Python');

  //���غ��İ�(�����LoadPackage����������������Unit�ظ������߼�)
  PackageInit;
end.
