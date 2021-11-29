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
unit UETSProjectCreator;

interface

{$WARN UNIT_PLATFORM OFF}

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, FileCtrl;

type
  TFmProjectCreator = class(TForm)
    LblName: TLabel;
    LblPath: TLabel;
    BtnOK: TButton;
    BtnCancel: TButton;
    EdName: TEdit;
    EdPath: TEdit;
    BtnPath: TButton;
    CbFrame: TCheckBox;
    CbModule: TCheckBox;
    procedure BtnPathClick(ASender: TObject);
    procedure FormClose(ASender: TObject; var AAction: TCloseAction);
    procedure BtnOKClick(Sender: TObject);
    procedure FormCreate(Sender: TObject);
  private
    class var FRecentSelectPath: String;
  public
    procedure GetValue(var APath, AFileName: String; var AFrame, AModule: Boolean);
  end;

implementation

{$R *.dfm}

procedure TFmProjectCreator.BtnOKClick(Sender: TObject);
begin
  if '' = Trim(EdName.Text) then
  begin
    ShowMessage(Format('%s不能为空', [LblName.Caption]));
    Exit;
  end;

  if '' = Trim(EdPath.Text) then
  begin
    ShowMessage(Format('%s不能为空', [LblPath.Caption]));
    Exit;
  end;

  if not DirectoryExists(EdPath.Text) then
  begin
    if mrYes <> MessageDlg('工程路径不存在，是否创建?', mtConfirmation, [mbYes, mbCancel], 0) then
      Exit;

    ForceDirectories(EdPath.Text);
  end;

  FRecentSelectPath := EdPath.Text;
  ModalResult := mrOk;
end;

procedure TFmProjectCreator.BtnPathClick(ASender: TObject);
var
  str: String;
begin
  str := EdPath.Text;
  if SelectDirectory(str, [sdAllowCreate, sdPerformCreate], 0) then
    EdPath.Text := str;
end;

procedure TFmProjectCreator.FormClose(ASender: TObject; var AAction: TCloseAction);
begin
  AAction := caFree;
end;

procedure TFmProjectCreator.FormCreate(Sender: TObject);
begin
  EdPath.Text := FRecentSelectPath;
end;

procedure TFmProjectCreator.GetValue(var APath, AFileName: String; var AFrame, AModule: Boolean);
begin
  APath := EdPath.Text;
  AFileName := EdName.Text;
  AFrame := CbFrame.Checked;
  AModule := CbModule.Checked;
end;

end.
