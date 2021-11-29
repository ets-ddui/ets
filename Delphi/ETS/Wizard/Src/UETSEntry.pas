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
  //注册工程、窗口向导
  ARegisterProc(TETSProjectWizard.Create);
  RegisterCustomModule(TFrameBase, TCustomModule);
  ARegisterProc(TETSFrameWizard.Create(TFrameBase));

  //初始化通知器
  TETSNotifierManager.Init(ABorlandIDEServices, ARegisterProc);

  //初始化菜单
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
  //dll模式不涉及向导释放的处理

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
