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
unit UETSCreator;

interface

uses
  Classes, SysUtils, ToolsAPI, UETSWizard, UDUIWizard, UDUICreator;

type
  TExportInterface = (eiFrame, eiModule);
  TExportInterfaces = set of TExportInterface;

  TETSProjectCreator = class(TDUICreator, IOTAProjectCreator, IOTAProjectCreator50, IOTAProjectCreator80)
  private
    //IOTAProjectCreator实现
    function GetFileName: String;
    function GetOptionFileName: String;
    function GetShowSource: Boolean;
    procedure NewDefaultModule;
    function NewOptionSource(const AProjectName: String): IOTAFile;
    procedure NewProjectResource(const AProject: IOTAProject);
    function NewProjectSource(const AProjectName: String): IOTAFile;
  private
    //IOTAProjectCreator50实现
    procedure NewDefaultProjectModule(const AProject: IOTAProject);
  private
    //IOTAProjectCreator80实现
    function GetProjectPersonality: String;
  protected
    function GetCreatorType: String; override;
    function GetOwner: IOTAModule; override;
  private
    FPath, FFileName: String;
    FExportInterfaces: TExportInterfaces;
  public
    constructor Create(ADUIRepositoryWizard: TDUIRepositoryWizard;
      APath, AFileName: String; AExportInterfaces: TExportInterfaces); reintroduce;
  end;

  TETSUnitCreator = class(TDUICreator, IOTAModuleCreator)
  private
    //IOTAModuleCreator实现
    function GetAncestorName: String;
    function GetImplFileName: String;
    function GetIntfFileName: String;
    function GetFormName: String;
    function GetMainForm: Boolean;
    function GetShowForm: Boolean;
    function GetShowSource: Boolean;
    function NewFormFile(const AFormIdent, AAncestorIdent: String): IOTAFile;
    function NewImplSource(const AModuleIdent, AFormIdent, AAncestorIdent: String): IOTAFile;
    function NewIntfSource(const AModuleIdent, AFormIdent, AAncestorIdent: String): IOTAFile;
    procedure FormCreated(const AFormEditor: IOTAFormEditor);
  private
    FResName, FFileName: String;
  protected
    function GetCreatorType: String; override;
    function GetOwner: IOTAModule; override;
  public
    constructor Create(ADUIRepositoryWizard: TDUIRepositoryWizard; AResName, AFileName: String); reintroduce;
  end;

implementation

uses
  UDUIFile, UETSCommon, UTool;

{ TETSProjectCreator }

constructor TETSProjectCreator.Create(ADUIRepositoryWizard: TDUIRepositoryWizard;
  APath, AFileName: String; AExportInterfaces: TExportInterfaces);
begin
  inherited Create(ADUIRepositoryWizard);

  FPath := APath;
  if ('' <> FPath) and ('\' <> FPath[Length(FPath)]) and ('/' <> FPath[Length(FPath)]) then
    FPath := FPath + '\';
  FFileName := AFileName;
  if 0 <> CompareText('.dpr', ExtractFileExt(FFileName)) then
    FFileName := FFileName + '.dpr';

  FExportInterfaces := AExportInterfaces;
end;

function TETSProjectCreator.GetCreatorType: String;
begin
  Result := sLibrary;
end;

function TETSProjectCreator.GetFileName: String;
begin
  Result := FPath + FFileName;
end;

function TETSProjectCreator.GetOptionFileName: String;
begin
  Result := '';
end;

function TETSProjectCreator.GetOwner: IOTAModule;
begin
  Result := GetActiveProjectGroup;
end;

function TETSProjectCreator.GetProjectPersonality: String;
begin
  Result := sDelphiPersonality;
end;

function TETSProjectCreator.GetShowSource: Boolean;
begin
  Result := True;
end;

procedure TETSProjectCreator.NewDefaultModule;
begin
end;

procedure TETSProjectCreator.NewDefaultProjectModule(const AProject: IOTAProject);
var
  opt: IOTAOptions;
begin
  //添加入口函数定义单元
  if eiFrame in FExportInterfaces then
    DUIRepositoryWizard.CreateModule(
      TETSUnitCreator.Create(DUIRepositoryWizard, 'Entry_Frame', FPath + 'UFrame.pas'));

  if eiModule in FExportInterfaces then
    DUIRepositoryWizard.CreateModule(
      TETSUnitCreator.Create(DUIRepositoryWizard, 'Entry_Module', FPath + 'UModule.pas'));

  //统一工程文件设置(包括编译输出目录、调试程序路径等)
  opt := AProject.GetProjectOptions;
  opt.SetOptionValue('HostApplication', '$(Tool)ETS\ETS.exe');
  opt.SetOptionValue('DebugCWD', '$(Tool)ETS\');
  opt.SetOptionValue('MapFile', 3);
  opt.SetOptionValue('OutputDir', '$(Tool)ETS\$(PluginPath)');
  opt.SetOptionValue('UnitOutputDir',
    '$(Tool)Temp\ETS\$(PluginPath)\' + ExtractNakedFileName(FFileName));
//  opt.SetOptionValue('UnitDir', '$(MyWork);$(MyWork)ActiveX');
//  opt.SetOptionValue('ObjDir', '$(MyWork);$(MyWork)ActiveX');
//  opt.SetOptionValue('SrcDir', '$(MyWork);$(MyWork)ActiveX');
//  opt.SetOptionValue('ResDir', '$(MyWork);$(MyWork)ActiveX');
  opt.SetOptionValue('UsePackages', True);
  opt.SetOptionValue('Packages', 'rtl;vcl;vclx');
end;

function TETSProjectCreator.NewOptionSource(const AProjectName: String): IOTAFile;
begin
  Result := nil;
end;

procedure TETSProjectCreator.NewProjectResource(const AProject: IOTAProject);
begin
end;

function TETSProjectCreator.NewProjectSource(const AProjectName: String): IOTAFile;
var
  slst: TStringList;
begin
  slst := TStringList.Create;
  try
    slst.StrictDelimiter := True;
    slst.DelimitedText := 'Library=,Export=';

    slst.Values['Library'] := ExtractNakedFileName(FFileName);
    if [] = FExportInterfaces then
      //slst.Values['Export'] := ''
    else if [eiFrame, eiModule] = FExportInterfaces then
      slst.Values['Export'] := 'exports'#$D#$A'  GetModule, GetFrame;'
    else if [eiFrame] = FExportInterfaces then
      slst.Values['Export'] := 'exports'#$D#$A'  GetFrame;'
    else
      slst.Values['Export'] := 'exports'#$D#$A'  GetModule;';

    Result := TDUIFile.Create(HInstance, 'Template_Project', slst);
  finally
    FreeAndNil(slst);
  end;
end;

{ TETSUnitCreator }

constructor TETSUnitCreator.Create(ADUIRepositoryWizard: TDUIRepositoryWizard;
  AResName, AFileName: String);
begin
  inherited Create(ADUIRepositoryWizard);

  FResName := AResName;
  FFileName := AFileName;
end;

procedure TETSUnitCreator.FormCreated(const AFormEditor: IOTAFormEditor);
begin
end;

function TETSUnitCreator.GetAncestorName: String;
begin
  Result := '';
end;

function TETSUnitCreator.GetCreatorType: String;
begin
  Result := sUnit;
end;

function TETSUnitCreator.GetFormName: String;
begin
  Result := '';
end;

function TETSUnitCreator.GetImplFileName: String;
begin
  Result := FFileName;
end;

function TETSUnitCreator.GetIntfFileName: String;
begin
  Result := '';
end;

function TETSUnitCreator.GetMainForm: Boolean;
begin
  Result := False;
end;

function TETSUnitCreator.GetOwner: IOTAModule;
begin
  Result := GetActiveProject;
end;

function TETSUnitCreator.GetShowForm: Boolean;
begin
  Result := False;
end;

function TETSUnitCreator.GetShowSource: Boolean;
begin
  Result := False;
end;

function TETSUnitCreator.NewFormFile(const AFormIdent,
  AAncestorIdent: String): IOTAFile;
begin
  Result := nil;
end;

function TETSUnitCreator.NewImplSource(const AModuleIdent, AFormIdent,
  AAncestorIdent: String): IOTAFile;
begin
  Result := TDUIFile.Create(HInstance, FResName, nil);
end;

function TETSUnitCreator.NewIntfSource(const AModuleIdent, AFormIdent,
  AAncestorIdent: String): IOTAFile;
begin
  Result := nil;
end;

end.
