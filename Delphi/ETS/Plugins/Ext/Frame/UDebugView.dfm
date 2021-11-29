object FrmDebugView: TFrmDebugView
  Left = 0
  Top = 0
  Width = 520
  Height = 343
  OnInit = FrameBaseInit
  OnNotify = FrameBaseNotify
  object PnlControls: TDUIPanel
    Left = 0
    Top = 0
    Width = 520
    Height = 25
    Align = alTop
    object BtnStart: TDUIButton
      Left = 0
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
      OnClick = DoClick
    end
    object BtnStop: TDUIButton
      Left = 25
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
      OnClick = DoClick
    end
    object BtnClear: TDUIButton
      Left = 50
      Top = 0
      Width = 25
      Height = 25
      Align = alLeft
      Shape.Width = 16
      Shape.Height = 16
      Shape.Picture.Childs = <
        item
          Name = #28165#38500'1'
          Left = 0
          Top = 64
          Width = 16
          Height = 16
        end
        item
          Name = #28165#38500'2'
          Left = 16
          Top = 64
          Width = 16
          Height = 16
        end>
      Shape.Picture.SkinName = 'SYSTEM['#28165#38500'1,'#28165#38500'2]'
      Shape.ShapeType = stPicture
      TextAlign = alNone
      OnClick = DoClick
    end
  end
  object TgData: TDUITreeGrid
    Left = 0
    Top = 25
    Width = 520
    Height = 318
    Align = alClient
    Columns = <
      item
        Caption = 'PID/'#26102#38388
        Width = 200
      end
      item
        Caption = #36827#31243#21517'/'#28040#24687
        Percent = True
        Width = -1
      end>
    Options = [goVertTitleLine, goHorzTitleLine, goVertLine, goHorzLine, goRangeSelect, goVertTitle]
  end
end
