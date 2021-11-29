object FmMain: TFmMain
  Left = 225
  Top = 28
  Caption = #24037#20855#21253
  ClientHeight = 524
  ClientWidth = 880
  Color = clBtnFace
  DockSite = True
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnClose = FormClose
  OnCloseQuery = FormCloseQuery
  OnCreate = FormCreate
  OnResize = FormResize
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object ImgBackground: TDUIImage
    Left = 0
    Top = 0
    Width = 880
    Height = 524
    Align = alClient
    Picture.Childs = <>
    object BlMain: TDUIButtonList
      Left = 0
      Top = 0
      Width = 880
      Height = 30
      Align = alTop
      TextAlign = alClient
      OnChanging = BlMainChanging
    end
    object PnlMain: TDUIPanel
      Left = 0
      Top = 30
      Width = 880
      Height = 494
      Align = alClient
    end
  end
end
