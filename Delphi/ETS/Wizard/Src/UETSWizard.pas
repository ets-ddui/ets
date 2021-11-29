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
