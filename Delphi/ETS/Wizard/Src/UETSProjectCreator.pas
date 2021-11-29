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
    ShowMessage(Format('%s����Ϊ��', [LblName.Caption]));
    Exit;
  end;

  if '' = Trim(EdPath.Text) then
  begin
    ShowMessage(Format('%s����Ϊ��', [LblPath.Caption]));
    Exit;
  end;

  if not DirectoryExists(EdPath.Text) then
  begin
    if mrYes <> MessageDlg('����·�������ڣ��Ƿ񴴽�?', mtConfirmation, [mbYes, mbCancel], 0) then
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
