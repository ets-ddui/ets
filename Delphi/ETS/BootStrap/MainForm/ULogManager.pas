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
unit ULogManager;

{$i UConfigure.inc}

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, ExtCtrls, UMessageConst, UInterface, UTool;

type
  TFmLog = class(TForm, ICallBack)
    MmLog: TMemo;
    PnlControl: TPanel;
    BtnClear: TButton;
    procedure FormCreate(Sender: TObject);
    procedure BtnClearClick(Sender: TObject);
    procedure FormDestroy(Sender: TObject);
  private
    { ICallBack实现 }
    function CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT; stdcall;
  private
    FCallBack: Pointer;
    function GetLogManager: ILogManager;
  protected
    procedure WndProc(var AMessage: TMessage); override;
  end;

implementation

{$R *.dfm}

{ TFmLog }

procedure TFmLog.BtnClearClick(Sender: TObject);
begin
  GetLogManager.Clear;
end;

function TFmLog.CallBack(AMessage, AWParam, ALParam: Cardinal): HRESULT;
begin
  PostMessage(Handle, AMessage, AWParam, ALParam);
  Result := S_OK;
end;

procedure TFmLog.FormCreate(Sender: TObject);
var
  msg: TMessage;
begin
  msg.Msg := WM_SETCURSOR;
  WndProc(msg);

  FCallBack := GetLogManager.RegistCallBack(Self);
end;

procedure TFmLog.FormDestroy(Sender: TObject);
begin
  if Assigned(FCallBack) then
  begin
    GetLogManager.UnRegistCallBack(FCallBack);
    FCallBack := nil;
  end;
end;

function TFmLog.GetLogManager: ILogManager;
begin
  Result := IDispatch(GetRawManager.Service['Log']) as ILogManager;
end;

procedure TFmLog.WndProc(var AMessage: TMessage);
begin
  case AMessage.Msg of
    WM_SETCURSOR:
    begin
      if PtInRect(PnlControl.ClientRect, PnlControl.ScreenToClient(Mouse.CursorPos)) then
        PnlControl.Height := 31
      else
        PnlControl.Height := 3;
    end;
    SM_LOG_ADD:
    begin
      MmLog.Lines.Add(GetLogManager.GetLog(AMessage.LParam));
      BtnClear.Enabled := True;
    end;
    SM_LOG_CLEAR:
    begin
      MmLog.Lines.Clear;
      BtnClear.Enabled := False;
    end;
  end;

  inherited;
end;

end.
