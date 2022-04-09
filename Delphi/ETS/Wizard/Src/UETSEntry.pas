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
unit UETSEntry;

interface

{$IFDEF PACKAGE_MODE}
procedure Register;
{$ENDIF}

implementation

uses
  Forms, SysUtils, ToolsAPI, DesignIntf, DesignEditors,
  UETSWizard, UFrameBase, UETSMenu, UETSNotifier;

function DoInitializeWizard(const ABorlandIDEServices: IBorlandIDEServices;
  ARegisterProc: TWizardRegisterProc): Boolean;
begin
  //ע�Ṥ�̡�������
  ARegisterProc(TETSProjectWizard.Create);
  RegisterCustomModule(TFrameBase, TCustomModule);
  ARegisterProc(TETSFrameWizard.Create(TFrameBase));

  //��ʼ��֪ͨ��
  TETSNotifierManager.Init(ABorlandIDEServices, ARegisterProc);

  //��ʼ���˵�
  TETSMenuManager.Init(ABorlandIDEServices, ARegisterProc);

  Result := True;
end;

{$IFDEF PACKAGE_MODE}
var
  GWizard: array of Integer;

function RegisterProc(const AWizard: IOTAWizard): Boolean;
begin
  SetLength(GWizard, Length(GWizard) + 1);
  GWizard[Length(GWizard) - 1] := (BorlandIDEServices as IOTAWizardServices).AddWizard(AWizard);

  Result := True;
end;

procedure UnRegisterProc;
var
  i: Integer;
begin
  for i := 0 to Length(GWizard) - 1 do
    if GWizard[i] > 0 then
      (BorlandIDEServices as IOTAWizardServices).RemoveWizard(GWizard[i]);
  SetLength(GWizard, 0);

  TETSNotifierManager.UnInit;
end;

procedure Register;
begin
  DoInitializeWizard(BorlandIDEServices, @RegisterProc);
end;
{$ENDIF}

{$IFDEF DLL_MODE}
procedure UnRegisterProc;
begin
  //dllģʽ���漰���ͷŵĴ���

  TETSNotifierManager.UnInit;
end;

function InitWizard(const ABorlandIDEServices: IBorlandIDEServices;
  ARegisterProc: TWizardRegisterProc;
  var ATerminate: TWizardTerminateProc): Boolean; stdcall;
begin
  Result := Assigned(ABorlandIDEServices);
  DoInitializeWizard(ABorlandIDEServices, ARegisterProc);
end;

exports
  InitWizard name WizardEntryPoint;
{$ENDIF}

initialization

finalization
  UnRegisterProc;

end.
