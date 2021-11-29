object FrmSkinEditor: TFrmSkinEditor
  Left = 0
  Top = 0
  Width = 838
  Height = 483
  OnDropFiles = FrameBaseDropFiles
  OnAfterNotify = FrameBaseAfterNotify
  object TgConfig: TDUITreeGrid
    Left = 0
    Top = 0
    Width = 313
    Height = 483
    Align = alLeft
    Columns = <
      item
        Caption = #21517#31216
        Percent = True
        Width = -1
      end
      item
        Caption = #25928#26524#22270
        Width = 100
      end>
    Options = [goVertTitleLine, goHorzTitleLine, goVertLine, goHorzLine, goRangeSelect, goVertTitle]
    OnGetPaintControls = TgConfigGetPaintControls
    OnSelectCell = TgConfigSelectCell
  end
end
