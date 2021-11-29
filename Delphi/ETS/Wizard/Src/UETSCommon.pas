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
unit UETSCommon;

interface

uses
  Windows, Classes, SysUtils, ActiveX, ToolsAPI, UTool;

function GetActiveProjectGroup: IOTAProjectGroup;
//function GetActiveProject: IOTAProject;

function ReadFile(AFileName: String; AParameters: TStringList): String;
function ReadResource(AResName: String; AParameters: TStringList): String;
procedure WriteFile(AFileName, AContent: String);

implementation

function GetActiveProjectGroup: IOTAProjectGroup;
var
  i: Integer;
  ms: IOTAModuleServices;
begin
  ms := BorlandIDEServices as IOTAModuleServices;
  for i := 0 to ms.ModuleCount - 1 do
    if Succeeded(ms.Modules[i].QueryInterface(IOTAProjectGroup, Result)) then
      Exit;

  Result := nil;
end;

{
function GetActiveProject: IOTAProject;
var
  pg: IOTAProjectGroup;
begin
  pg := GetActiveProjectGroup;
  if Assigned(pg) then
    Result := pg.ActiveProject
  else
    Result := nil;
end;
}

function ReadFile(AFileName: String; AParameters: TStringList): String;
begin
  Result := LoadFile(AFileName);
  if (Result = '') or not Assigned(AParameters) then
    Exit;

  Result := FormatEh(Result, AParameters);
end;

function ReadResource(AResName: String; AParameters: TStringList): String;
var
  hRC: HRSRC;
  hData: THandle;
  iSize: Integer;
begin
  Result := '';
  if '' = AResName then
    Exit;

  hRC := FindResource(HInstance, @AResName[1], RT_RCDATA);
  if 0 = hRC then
    Exit;

  iSize := SizeofResource(HInstance, hRC);
  if 0 = iSize then
    Exit;

  hData := LoadResource(HInstance, hRC);

  if 0 >= hData then
    Exit;

  try
    SetLength(Result, iSize);
    Move(LockResource(hData)^, Result[1], iSize);
    UnlockResource(hData);
  finally
    FreeResource(hData);
  end;

  Result := FormatEh(Result, AParameters);
end;

procedure WriteFile(AFileName, AContent: String);
begin
  with TFileStream.Create(AFileName, fmCreate) do
    try
      Write(AContent[1], Length(AContent));
    finally
      Free;
    end;
end;

end.
