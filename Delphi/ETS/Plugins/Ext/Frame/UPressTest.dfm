object FrmPressTest: TFrmPressTest
  Left = 0
  Top = 0
  Width = 720
  Height = 501
  OnDropFiles = FrameBaseDropFiles
  OnInit = FrameBaseInit
  object WwMain: TDUIWinContainer
    Left = 200
    Top = 25
    Width = 520
    Height = 476
    Align = alClient
    object PnlTitle: TPanel
      Left = 0
      Top = 0
      Width = 520
      Height = 24
      Align = alTop
      BevelOuter = bvNone
      TabOrder = 0
    end
    object EdScript: TScintilla
      Left = 0
      Top = 24
      Width = 520
      Height = 452
      Align = alClient
      View.FoldIndicator = True
      View.ShowLineNumber = True
      View.StyleFile = 'embed:DefaultStyle'
      View.TabSize = 4
      Text.UseTab = False
    end
  end
  object PnlControls: TDUIPanel
    Left = 0
    Top = 0
    Width = 720
    Height = 25
    Align = alTop
    AlignOrder = 2
    object BtnStart: TDUIButton
      Tag = 3
      Left = 50
      Top = 0
      Width = 25
      Height = 25
      Align = alLeft
      Shape.Width = 16
      Shape.Height = 16
      Shape.Picture.Childs = <
        item
          Name = #24320#21551'1'
          Left = 0
          Top = 32
          Width = 16
          Height = 16
        end
        item
          Name = #24320#21551'2'
          Left = 16
          Top = 32
          Width = 16
          Height = 16
        end>
      Shape.Picture.SkinName = 'SYSTEM['#24320#21551'1,'#24320#21551'2]'
      Shape.ShapeType = stPicture
      TextAlign = alNone
      OnClick = DoControlButtonClick
    end
    object BtnStop: TDUIButton
      Tag = 4
      Left = 75
      Top = 0
      Width = 25
      Height = 25
      Align = alLeft
      Shape.Width = 16
      Shape.Height = 16
      Shape.Picture.Childs = <
        item
          Name = #31105#27490'1'
          Left = 0
          Top = 16
          Width = 16
          Height = 16
        end
        item
          Name = #31105#27490'2'
          Left = 16
          Top = 16
          Width = 16
          Height = 16
        end>
      Shape.Picture.SkinName = 'SYSTEM['#31105#27490'1,'#31105#27490'2]'
      Shape.ShapeType = stPicture
      TextAlign = alNone
      Enabled = False
      OnClick = DoControlButtonClick
    end
    object BtnRefresh: TDUIButton
      Tag = 2
      Left = 25
      Top = 0
      Width = 25
      Height = 25
      Align = alLeft
      Shape.Width = 16
      Shape.Height = 16
      Shape.Picture.Childs = <
        item
          Name = #21047#26032'1'
          Left = 0
          Top = 160
          Width = 16
          Height = 16
        end
        item
          Name = #21047#26032'2'
          Left = 16
          Top = 160
          Width = 16
          Height = 16
        end>
      Shape.Picture.SkinName = 'SYSTEM['#21047#26032'1,'#21047#26032'2]'
      Shape.ShapeType = stPicture
      TextAlign = alNone
      OnClick = DoControlButtonClick
    end
    object BtnNew: TDUIButton
      Tag = 1
      Left = 0
      Top = 0
      Width = 25
      Height = 25
      Align = alLeft
      Shape.Width = 16
      Shape.Height = 16
      Shape.Picture.Childs = <
        item
          Name = #28155#21152'1'
          Left = 0
          Top = 192
          Width = 16
          Height = 16
        end
        item
          Name = #28155#21152'2'
          Left = 16
          Top = 192
          Width = 16
          Height = 16
        end>
      Shape.Picture.SkinName = 'SYSTEM['#28155#21152'1,'#28155#21152'2]'
      Shape.ShapeType = stPicture
      TextAlign = alNone
      OnClick = DoControlButtonClick
    end
  end
  object TgFile: TDUITreeGrid
    Left = 0
    Top = 25
    Width = 200
    Height = 476
    Align = alLeft
    AlignOrder = 1
    Columns = <
      item
        Caption = #33050#26412#28165#21333
        Percent = True
        Width = -1
      end
      item
        Caption = #25991#20214#36335#24452
        Visible = False
        Width = -1
      end
      item
        Caption = #30446#24405#26631#24535
        Visible = False
        Width = -1
      end>
    Options = [goVertTitleLine, goHorzTitleLine, goVertLine, goHorzLine, goRangeSelect, goVertTitle]
    OnSelectCellDouble = TgFileSelectCellDouble
  end
end
