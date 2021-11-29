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
    { ICallBackʵ�� }
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
