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
unit USettingManager;

{$i UConfigure.inc}

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  StdCtrls, ExtCtrls, UMessageConst, UInterface;

type
  TFmSetting = class(TForm, ICallBack)
    PnlControl: TPanel;
    BtnSave: TButton;
    procedure FormCreate(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
    procedure BtnSaveClick(Sender: TObject);
  private
    { ICallBack实现 }
    function CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT; stdcall;
  private
    FCallBack: Pointer;
    function GetSettingManager: ISettingManager;
  protected
    procedure WndProc(var AMessage: TMessage); override;
  end;

implementation

uses
  qjson, UTool;

{$R *.dfm}

{ TFmSetting }

procedure TFmSetting.BtnSaveClick(Sender: TObject);
begin
  GetSettingManager.Save;
  BtnSave.Enabled := False;
end;

function TFmSetting.CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT;
begin
  PostMessage(Handle, AMessage, AWParam, ALParam);
  Result := S_OK;
end;

procedure TFmSetting.FormCreate(Sender: TObject);
var
  msg: TMessage;
begin
  msg.Msg := WM_SETCURSOR;
  WndProc(msg);

  FCallBack := GetSettingManager.RegistCallBack(Self);
end;

procedure TFmSetting.FormDestroy(Sender: TObject);
begin
  if Assigned(FCallBack) then
  begin
    GetSettingManager.UnRegistCallBack(FCallBack);
    FCallBack := nil;
  end;
end;

function TFmSetting.GetSettingManager: ISettingManager;
begin
  Result := IDispatch(GetRawManager.Service['Setting']) as ISettingManager;
end;

procedure TFmSetting.WndProc(var AMessage: TMessage);
var
  json: TQJson;
begin
  case AMessage.Msg of
    WM_SETCURSOR:
    begin
      if PtInRect(PnlControl.ClientRect, PnlControl.ScreenToClient(Mouse.CursorPos)) then
        PnlControl.Height := 31
      else
        PnlControl.Height := 3;
    end;
    SM_SETTING_CHANGED:
    begin
      json := TQJson(AMessage.LParam);
      if 0 = CompareText(json.Path, 'System.AutoCreateDump') then
      begin
        if json.AsBoolean then
          LoadExceptionDeal(True, False)
        else
          UnLoadExceptionDeal;
      end;  
    end;
  end;

  inherited;
end;

end.
