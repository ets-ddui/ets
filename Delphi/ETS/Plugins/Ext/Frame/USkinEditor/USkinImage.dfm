inherited FrmSkinImage: TFrmSkinImage
  object BlChildImage: TDUIButtonList
    Left = 0
    Top = 280
    Width = 500
    Height = 70
    Align = alBottom
    ButtonWidth = 70
  end
  object ImgMain: TDUIImage
    Left = 0
    Top = 0
    Width = 500
    Height = 128
    Align = alClient
    Picture.Childs = <>
    object BtnTransparent: TDUIButton
      Left = 0
      Top = 0
      Height = 25
      Shape.Visible = False
      TextAlign = alClient
      Caption = #36879#26126#33394
      OnClick = BtnTransparentClick
    end
  end
  object PnlChild: TDUIPanel
    Left = 0
    Top = 128
    Width = 500
    Height = 152
    Align = alBottom
    object ImgChild: TDUIImage
      Left = 80
      Top = 25
      Width = 340
      Height = 102
      Align = alClient
      Picture.Childs = <>
    end
    object EdLeft: TDUIEdit
      Left = 0
      Top = 63
      Width = 80
      Height = 25
      Hint = #24038
      Align = alLeft
      AlignKeepSize = True
      ArcBorder = False
    end
    object EdHeight: TDUIEdit
      Left = 420
      Top = 63
      Width = 80
      Height = 25
      Hint = #39640
      Align = alRight
      AlignKeepSize = True
      ArcBorder = False
    end
    object EdTop: TDUIEdit
      Left = 210
      Top = 0
      Width = 80
      Height = 25
      Hint = #19978
      Align = alTop
      AlignKeepSize = True
      ArcBorder = False
    end
    object EdWidth: TDUIEdit
      Left = 210
      Top = 127
      Width = 80
      Height = 25
      Hint = #23485
      Align = alBottom
      AlignKeepSize = True
      ArcBorder = False
    end
  end
end
