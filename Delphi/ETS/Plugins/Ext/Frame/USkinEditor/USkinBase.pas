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
unit USkinBase;

interface

uses
  Windows, Controls, GDIHelper, UDUIForm, UFrameBase, USkinEditor, Classes,
  UDUICore, UDUIButton, UDUIPanel, UDUIImage, UDUIWinWrapper, UDUIEdit, UDUIShape;

type
  TFrmSkinBase = class(TDUIFrame, ISkinEditor)
  private
    FDirtyButtons: array[0..1] of TDUIButton;
    procedure DoClick(ASender: TObject);
    function GetHelper: TBaseHelper;
  protected
    procedure DoCommit; virtual; abstract;
    procedure DoUpdate; virtual; abstract;
    procedure SetDirty;
    procedure ChangeHelperType(AClass: TBaseHelperClass);
    property Helper: TBaseHelper read GetHelper;
  protected
    { ISkinEditor实现 }
    procedure Init(AParent: TFrameBase); virtual;
    procedure UnInit; virtual;
    procedure OnDropFile(AFileName: String; AIndex, ACount: Integer); virtual;
  end;

implementation

{$R *.dfm}

{ TFrmSkinBase }

procedure TFrmSkinBase.Init(AParent: TFrameBase);
begin
  DUIParent := AParent;
  Align := alClient;
  DoUpdate;
end;

procedure TFrmSkinBase.OnDropFile(AFileName: String; AIndex, ACount: Integer);
begin
end;

procedure TFrmSkinBase.UnInit;
begin
  DUIParent := nil;
end;

procedure TFrmSkinBase.ChangeHelperType(AClass: TBaseHelperClass);
begin
  TFrmSkinEditor(DUIParent).ChangeHelperType(AClass);
end;

procedure TFrmSkinBase.DoClick(ASender: TObject);
begin
  if ASender = FDirtyButtons[0] then
  begin
    DoCommit;
    TFrmSkinEditor(DUIParent).Commit;
  end
  else
  begin
    TFrmSkinEditor(DUIParent).Cancel;
    DoUpdate;
  end;

  FDirtyButtons[0].Visible := False;
  FDirtyButtons[1].Visible := False;
end;

function TFrmSkinBase.GetHelper: TBaseHelper;
begin
  Result := TFrmSkinEditor(DUIParent).Helper;
end;

procedure TFrmSkinBase.SetDirty;
const
  CButtonSize: Integer = 10;
begin
  if not Assigned(FDirtyButtons[0]) then
  begin
    FDirtyButtons[0] := TDUIButton.Create(Self);
    with FDirtyButtons[0] do
    begin
      DUIParent := Self;
      Width := 3 * CButtonSize div 2;
      Height := 3 * CButtonSize div 2;
      Shape.Width := CButtonSize;
      Shape.Height := CButtonSize;
      Shape.ShapeType := stCommit;
      Shape.LineWidth := 1;
      Anchors := [akTop, akRight];
      OnClick := DoClick;
    end;

    FDirtyButtons[1] := TDUIButton.Create(Self);
    with FDirtyButtons[1] do
    begin
      DUIParent := Self;
      Width := 3 * CButtonSize div 2;
      Height := 3 * CButtonSize div 2;
      Shape.Width := CButtonSize;
      Shape.Height := CButtonSize;
      Shape.ShapeType := stClose;
      Shape.LineWidth := 1;
      Anchors := [akTop, akRight];
      OnClick := DoClick;
    end;
  end;

  with FDirtyButtons[0] do
  begin
    Left := Self.Width - 2 * Width;
    Visible := True;
    BringToFront;
  end;

  with FDirtyButtons[1] do
  begin
    Left := Self.Width - Width;
    Visible := True;
    BringToFront;
  end;
end;

end.
