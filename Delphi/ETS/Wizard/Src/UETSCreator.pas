{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)������չ���߼���

  ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
  �����˿��Ŀ����ϣ�������ã��������κα�֤��
  ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

  ��Դ��ַ: https://github.com/ets-ddui/ets
            https://gitee.com/ets-ddui/ets
  ��ԴЭ��: The MIT License (MIT)
  ��������: xinghun87@163.com
  �ٷ����ͣ�https://blog.csdn.net/xinghun61
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
    //IOTAProjectCreatorʵ��
    function GetFileName: String;
    function GetOptionFileName: String;
    function GetShowSource: Boolean;
    procedure NewDefaultModule;
    function NewOptionSource(const AProjectName: String): IOTAFile;
    procedure NewProjectResource(const AProject: IOTAProject);
    function NewProjectSource(const AProjectName: String): IOTAFile;
  private
    //IOTAProjectCreator50ʵ��
    procedure NewDefaultProjectModule(const AProject: IOTAProject);
  private
    //IOTAProjectCreator80ʵ��
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
    //IOTAModuleCreatorʵ��
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
  //�����ں������嵥Ԫ
  if eiFrame in FExportInterfaces then
    DUIRepositoryWizard.CreateModule(
      TETSUnitCreator.Create(DUIRepositoryWizard, 'Entry_Frame', FPath + 'UFrame.pas'));

  if eiModule in FExportInterfaces then
    DUIRepositoryWizard.CreateModule(
      TETSUnitCreator.Create(DUIRepositoryWizard, 'Entry_Module', FPath + 'UModule.pas'));

  //ͳһ�����ļ�����(�����������Ŀ¼�����Գ���·����)
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
