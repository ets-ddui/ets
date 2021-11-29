object FmProjectCreator: TFmProjectCreator
  Left = 0
  Top = 0
  Caption = 'ETS'#24037#31243#21019#24314#21521#23548
  ClientHeight = 185
  ClientWidth = 545
  Color = clBtnFace
  Constraints.MaxHeight = 223
  Constraints.MinHeight = 223
  Constraints.MinWidth = 560
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnClose = FormClose
  OnCreate = FormCreate
  DesignSize = (
    545
    185)
  PixelsPerInch = 96
  TextHeight = 13
  object LblName: TLabel
    Left = 30
    Top = 20
    Width = 48
    Height = 13
    Caption = #24037#31243#21517#31216
  end
  object LblPath: TLabel
    Left = 30
    Top = 60
    Width = 48
    Height = 13
    Caption = #24037#31243#36335#24452
  end
  object BtnOK: TButton
    Left = 120
    Top = 140
    Width = 75
    Height = 25
    Caption = #30830#23450
    TabOrder = 5
    OnClick = BtnOKClick
  end
  object BtnCancel: TButton
    Left = 350
    Top = 140
    Width = 75
    Height = 25
    Anchors = [akTop, akRight]
    Caption = #21462#28040
    ModalResult = 2
    TabOrder = 6
  end
  object EdName: TEdit
    Left = 100
    Top = 20
    Width = 300
    Height = 21
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 0
  end
  object EdPath: TEdit
    Left = 100
    Top = 60
    Width = 300
    Height = 21
    Anchors = [akLeft, akTop, akRight]
    TabOrder = 1
  end
  object BtnPath: TButton
    Left = 440
    Top = 60
    Width = 75
    Height = 25
    Anchors = [akTop, akRight]
    Caption = #27983#35272
    TabOrder = 2
    OnClick = BtnPathClick
  end
  object CbFrame: TCheckBox
    Left = 150
    Top = 100
    Width = 100
    Height = 17
    Caption = #28155#21152#31383#20307#20837#21475
    Checked = True
    State = cbChecked
    TabOrder = 3
  end
  object CbModule: TCheckBox
    Left = 295
    Top = 100
    Width = 100
    Height = 17
    Anchors = [akTop, akRight]
    Caption = #28155#21152#27169#22359#20837#21475
    Checked = True
    State = cbChecked
    TabOrder = 4
  end
end
