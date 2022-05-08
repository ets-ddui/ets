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
unit UPressTest;

{$i UConfigure.inc}

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, StrUtils, ExtCtrls, Grids, Buttons, ComCtrls,
  UInterface, UFrameBase, UScriptThread, UDUICore, UDUIForm, UDUIWinWrapper,
  Scintilla, UMemoryBlock, UDUIButton, UDUIPanel, UDUIGrid, UDUITreeGrid;

type
  TFrmPressTest = class(TFrameBase)
    WwMain: TDUIWinContainer;
    PnlControls: TDUIPanel;
    BtnStart: TDUIButton;
    BtnStop: TDUIButton;
    BtnRefresh: TDUIButton;
    BtnNew: TDUIButton;
    TgFile: TDUITreeGrid;
    PnlTitle: TPanel;
    EdScript: TScintilla;
    procedure DoControlButtonClick(ASender: TObject);
    procedure TgFileSelectCellDouble(ASender: TObject; const ACol,
      ARow: TDUIRowColID; AX, AY: Integer);
    procedure FrameBaseInit(AParent: IParent; AIndex: Integer);
    procedure FrameBaseDropFiles(AFileName: string; AIndex, ACount: Integer);
  private
    FThreadContainer: Variant;
    FFrame: TDUIPanel;
  end;

implementation

uses
  ShellAPI, UModuleBase, UTool;

{$R *.dfm}

{ TFrmPressTest }

procedure TFrmPressTest.FrameBaseInit(AParent: IParent; AIndex: Integer);
begin
  FThreadContainer := Null;

  BtnStart.Enabled := True;
  BtnStop.Enabled := False;

  AcceptDropFiles := True;

  if not Assigned(FFrame) then
  begin
    FFrame := TDUIPanel.Create(Self);
    FFrame.Visible := False;
    FFrame.Align := alBottom;
    FFrame.DUIParent := Self;
  end;

  if FileExists('Config\Style.json') then
    EdScript.View.StyleFile := 'Config\Style.json';

  DoControlButtonClick(BtnRefresh);
  DoControlButtonClick(BtnNew);
end;

procedure TFrmPressTest.TgFileSelectCellDouble(ASender: TObject; const ACol,
  ARow: TDUIRowColID; AX, AY: Integer);
var
  tn: TDUITreeNode;
begin
  tn := TDUITreeNode(ARow.FIndex);
  if tn.Cells[2] = '1' then
    Exit;

  PnlTitle.Caption := tn.Cells[1];
  EdScript.Text.LoadFromFile(tn.Cells[1]);
  FFrame.Visible := False;
end;

procedure TFrmPressTest.DoControlButtonClick(ASender: TObject);
  procedure doRefresh;
    function createTreeNode(AParentNode: TDUITreeNode;
      AFileName: String; AIsDirectory: Boolean): TDUITreeNode;
    begin
      Result := AParentNode.AddChild(ExtractFileName(AFileName));
      Result.Collapsed := True;
      Result.Cells[1] := AFileName;
      Result.Cells[2] := IfThen(AIsDirectory, '1', '0');
    end;

    procedure enumDirectory(AParentNode: TDUITreeNode; ADirectory: String);
    var
      sr: TSearchRec;
    begin
      if SysUtils.FindFirst(ADirectory + '*.*', faAnyFile, sr) = 0 then
        try
          repeat
            if (sr.Name <> '.') and (sr.Name <> '..') then
            begin
              if sr.Attr and faDirectory <> 0 then
                enumDirectory(createTreeNode(AParentNode, ADirectory + sr.Name, True),
                  ADirectory + sr.Name + '\')
              else
                createTreeNode(AParentNode, ADirectory + sr.Name, False);
            end;
          until SysUtils.FindNext(sr) <> 0;
        finally
          SysUtils.FindClose(sr);
        end;
    end;

  begin
    TgFile.RootNode.Clear;
    enumDirectory(TgFile.RootNode, '.\Script\');
  end;

  procedure doRun;
  var
    i: Integer;
  begin
    if not VarIsNull(FThreadContainer) then
    begin
      BtnStop.Enabled := False;
      FThreadContainer.Terminate;
      Exit;
    end;

    BtnStart.Enabled := False;
    BtnStop.Enabled := True;
    try
      for i := FFrame.ControlCount - 1 downto 0 do
        FFrame.Controls[i].Free;

      FThreadContainer := GetScriptThreadContainer;
      FThreadContainer.Restart;
      FThreadContainer.RegFrame(WrapperObject(FFrame, False));
      if 0 = CompareText(ExtractFileExt(PnlTitle.Caption), '.py') then
        FThreadContainer.AddCacheFile(PnlTitle.Caption, IInterface(TMemoryBlock.Create(EdScript.Text.Value, etUtf8)))
      else
        FThreadContainer.AddCacheFile(PnlTitle.Caption, IInterface(TMemoryBlock.Create(EdScript.Text.Value, etGbk)));
      FThreadContainer.AddThread(PnlTitle.Caption, 'main()');
      FThreadContainer.WaitFor;
    finally
      FThreadContainer := Null;

      BtnStart.Enabled := True;
      BtnStop.Enabled := False;
    end;
  end;

begin
  case TDUIButton(ASender).Tag of
    1: //�½�
    begin
      PnlTitle.Caption := 'Untitled.js';
      EdScript.Text.LoadFromFile('Lib/Default.js');
    end;
    2: //ˢ��
    begin
      doRefresh;
    end;
    3, 4: //���С�ֹͣ
    begin
      doRun;
    end;
  end;
end;

procedure TFrmPressTest.FrameBaseDropFiles(AFileName: String; AIndex, ACount: Integer);
begin
  if AIndex = 1 then //���ѡ���˶���ļ���ֻȡ��1��
  begin
    PnlTitle.Caption := AFileName;
    EdScript.Text.LoadFromFile(AFileName);
    FFrame.Visible := False;
  end;
end;

initialization
  TFrameBase.RegFrame(TFrmPressTest);

end.
