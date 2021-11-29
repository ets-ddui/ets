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
unit UETSWizard;

interface

uses
  Classes, Windows, Controls, ToolsAPI, UDUIWizard, UDUICreator;

type
  TETSProjectWizard = class(TDUIRepositoryWizard, IOTAProjectWizard)
  protected
    function GetGlyph: Cardinal; override;
    function GetName: String; override;
    procedure Execute; override;
  end;

  TETSFrameWizard = class(TDUIRepositoryWizard, IOTAProjectWizard, IOTAFormWizard)
  private
    FFrameClass: TComponentClass;
  protected
    function GetGlyph: Cardinal; override;
    function GetName: String; override;
    procedure Execute; override;
  public
    constructor Create(AFrameClass: TComponentClass);
  end;

implementation

{$R 'ETS.res'}

uses
  UETSCreator, UETSProjectCreator;

{ TETSProjectWizard }

procedure TETSProjectWizard.Execute;
var
  strPath, strFileName: String;
  bFrame, bModule: Boolean;
  eis: TExportInterfaces;
begin
  strPath := '';
  strFileName := '';
  bFrame := False;
  bModule := False;
  eis := [];

  with TFmProjectCreator.Create(nil) do
  begin
    if mrOk <> ShowModal then
      Exit;

    GetValue(strPath, strFileName, bFrame, bModule);
    if bFrame then
      Include(eis, eiFrame);
    if bModule then
      Include(eis, eiModule);
  end;

  CreateModule(TETSProjectCreator.Create(Self, strPath, strFileName, eis));
end;

function TETSProjectWizard.GetGlyph: Cardinal;
begin
  Result := LoadIcon(HInstance, 'Project');
end;

function TETSProjectWizard.GetName: String;
begin
  Result := 'ETS DLL Wizard';
end;

{ TETSFormWizard }

constructor TETSFrameWizard.Create(AFrameClass: TComponentClass);
begin
  FFrameClass := AFrameClass;
end;

procedure TETSFrameWizard.Execute;
begin
  CreateModule(TDUIFormCreator.Create(Self, FFrameClass, 'Template_Frame', HInstance));
end;

function TETSFrameWizard.GetGlyph: Cardinal;
begin
  Result := LoadIcon(HInstance, 'Frame');
end;

function TETSFrameWizard.GetName: String;
begin
  Result := 'ETS Frame';
end;

end.
