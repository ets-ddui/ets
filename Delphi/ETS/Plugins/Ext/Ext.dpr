library Ext;

{$i UConfigure.inc}

{ Important note about DLL memory management: ShareMem must be the
  first unit in your library's USES clause AND your project's (select
  Project-View Source) USES clause if your DLL exports any procedures or
  functions that pass strings as parameters or function results. This
  applies to all strings passed to and from your DLL--even those that
  are nested in records and classes. ShareMem is the interface unit to
  the BORLNDMM.DLL shared memory manager, which must be deployed along
  with your DLL. To avoid using BORLNDMM.DLL, pass string information
  using PChar or ShortString parameters. }

uses
  {$IFDEF LAZARUS}
  {$IFDEF UNIX}{$IFDEF UseCThreads}
  cthreads,
  {$ENDIF}{$ENDIF}
  Interfaces, // this includes the LCL widgetset
  {$ENDIF}
  SysUtils,
  Classes,
  UAppInit,
  UQueueManager in 'Module\UQueueManager.pas',
  UModule in 'UModule.pas',
  UFrame in 'UFrame.pas',
  UDebugView in 'Frame\UDebugView.pas' {FrmDebugView: TFrame},
  UPressTest in 'Frame\UPressTest.pas' {FrmPressTest: TFrame},
  USkinEditor in 'Frame\USkinEditor.pas' {FrmSkinEditor: TFrameBase},
  USkinBase in 'Frame\USkinEditor\USkinBase.pas' {FrmSkinBase: TDUIFrame},
  USkinImage in 'Frame\USkinEditor\USkinImage.pas' {FrmSkinImage: TDUIFrame},
  UScriptThread in 'Module\UScriptThread.pas',
  UFileReader in 'Module\UFileReader.pas',
  UCmd in 'Module\UCmd.pas',
  UTls in 'Module\UTls.pas';

{$IFDEF LAZARUS}
//{$R Ext_Lazarus.res}
{$ELSE}
{$R Ext_Delphi.res}
{$ENDIF}

exports
  GetModule, GetFrame;

begin
  TAppInit.Init(True);
end.
