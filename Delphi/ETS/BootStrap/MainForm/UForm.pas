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
unit UForm;

{$i UConfigure.inc}

interface

uses
  Windows, Messages, SysUtils, Classes, Forms, Buttons, ComCtrls, Controls,
  UMessageConst, UInterface, ExtCtrls, UDUIForm, UManager, UTrayIcon, UDUIButton,
  UDUICore, UDUIPanel, UDUIImage;

type
  TFmMain = class(TDUIForm, IParent)
    BlMain: TDUIButtonList;
    PnlMain: TDUIPanel;
    ImgBackground: TDUIImage;
    procedure FormCreate(Sender: TObject);
    procedure FormClose(ASender: TObject; var AAction: TCloseAction);
    procedure FormShow(Sender: TObject);
    procedure FormResize(Sender: TObject);
    procedure FormCloseQuery(ASender: TObject; var ACanClose: Boolean);
    procedure BlMainChanging(ASender: TObject; AOldIndex, ANewIndex: Integer;
      var ACanChange: Boolean);
  private
    FConfig, FLog: TDUIButton;
    FFrames: TInterfaceList;
    FFormList: array[TPinStyle] of TForm;
    FSplitter: array[TPinStyle] of TSplitter;
    FTrayIconService: TTrayIconService;
    procedure WMSysCommand(var AMessage: TWMSysCommand); message WM_SYSCOMMAND;
    procedure WMEraseBkgnd(var AMessage: TWmEraseBkgnd); message WM_ERASEBKGND;
    procedure CMIdle(var AMessage: TMessage); message CM_IDLE;
    procedure ETSMMainThreadLock(var AMessage: TMessage); message ETSM_MAIN_THREAD_LOCK;
  protected
    procedure DoClick(ASender: TObject); override;
    procedure Resizing(AState: TWindowState); override;
  private
    {IParent实现}
    procedure AddChild(ACaption: WideString; AChild: IChild);
    function GetParam(AIndex: Integer): TParam;
  end;

implementation

uses
  IniFiles, Math, StrUtils, Variants, ActiveX, TypInfo, qjson, qstring,
  UFrameBase, UChild, USettingManager, ULogManager, UTool, ULibraryManager, UAppInit,
  UService, UDUIShape, UScript;

{$R *.dfm}

{ TFmMain }

procedure TFmMain.BlMainChanging(ASender: TObject; AOldIndex,
  ANewIndex: Integer; var ACanChange: Boolean);
begin
  if AOldIndex >= 0 then
  begin
    //1.0 释放老窗口的资源
    IChild(FFrames[AOldIndex]).Notify(ntDeActive, ACanChange);
    if not ACanChange then
      Exit;
  end;

  if ANewIndex >= 0 then
  begin
    //2.0 进入新窗口
    IChild(FFrames[ANewIndex]).Notify(ntActive, ACanChange);

    //3.0 新窗口进入失败，则回退到老窗口
    if not ACanChange and (AOldIndex >= 0) then
    begin
      IChild(FFrames[AOldIndex]).Notify(ntActive, ACanChange);
      ACanChange := False;
      Exit;
    end;
  end;

  ACanChange := True;
end;

procedure TFmMain.CMIdle(var AMessage: TMessage);
var
  bResult: Boolean;
begin
  if BlMain.ActiveButtonIndex >= 0 then
  begin
    bResult := True;
    IChild(FFrames[BlMain.ActiveButtonIndex]).Notify(ntIdle, bResult);
  end;
end;

procedure TFmMain.DoClick(ASender: TObject);
const
  cAlign: array[TPinStyle] of TAlign = (alNone, alLeft, alTop, alRight, alBottom, alNone);
var
  ps: TPinStyle;
begin
  if ASender = FConfig then
    ps := psRight
  else if ASender = FLog then
    ps := psBottom
  else
  begin
    inherited;
    Exit;
  end;

  if FFormList[ps].Showing then
  begin
    FFormList[ps].Hide;
    FSplitter[ps].Visible := False;
  end
  else
  begin
    if FFormList[ps].Floating then
    begin
      FFormList[ps].Align := cAlign[ps];
      FSplitter[ps].Align := cAlign[ps];
      FFormList[ps].ManualDock(Self, nil, CAlign[ps]);
    end;
    FFormList[ps].Show;
    FSplitter[ps].Visible := True;

    case ps of
      psRight:
      begin
        FFormList[ps].Left := 0;
        FSplitter[ps].Left := 0;
      end;
      psBottom:
      begin
        FFormList[ps].Top := 0;
        FSplitter[ps].Top := 0;
      end;
    end;
  end;
end;

procedure TFmMain.FormClose(ASender: TObject; var AAction: TCloseAction);
var
  ps: TPinStyle;
begin
  //1.0 释放日志及参数配置窗口
  for ps := Low(TPinStyle) to High(TPinStyle) do
  begin
    FreeAndNil(FFormList[ps]);
    FreeAndNil(FSplitter[ps]);
  end;

  //2.0 释放子窗口
  FreeAndNil(FFrames);

  //3.0 释放所有DLL，这样可保证DLL中的静态对象引用的FManager被释放(例如，UQueueManager.TQueue.FManager)
  TLibraryManager.UnInit(True);

  //4.0 释放FManager及相关资源
  FreeAndNil(FTrayIconService);
  SetManager(nil);
end;

procedure TFmMain.FormCloseQuery(ASender: TObject; var ACanClose: Boolean);
begin
  BlMainChanging(BlMain, BlMain.ActiveButtonIndex, -1, ACanClose);
end;

procedure TFmMain.FormCreate(Sender: TObject);
begin
  FConfig := AddSysButton(stSetting);
  with FConfig.Shape do
  begin
    Width := 15;
    Height := 15;
  end;
  FLog := AddSysButton(stLog);
  FLog.Shape.Width := 9;

  ImgBackground.Picture.SkinName := 'SYSTEM.BACKGROUND';

  FFrames := TInterfaceList.Create;

  FTrayIconService := TTrayIconService.Create(nil);

  SetManager(TManager.Create(nil, TService.Create(FTrayIconService)));

  FFormList[psRight] := TFmSetting.Create(nil);
  FFormList[psRight].Tag := Ord(psRight);
  FSplitter[psRight] := TSplitter.Create(nil);
  FSplitter[psRight].Parent := Self;
  FSplitter[psRight].Visible := False;

  FFormList[psBottom] := TFmLog.Create(nil);
  FFormList[psBottom].Tag := Ord(psBottom);
  FSplitter[psBottom] := TSplitter.Create(nil);
  FSplitter[psBottom].Parent := Self;
  FSplitter[psBottom].Visible := False;
end;

procedure TFmMain.FormShow(Sender: TObject);
begin
  if (BlMain.ButtonCount > 0) and (BlMain.ActiveButtonIndex < 0) then
    BlMain.ActiveButtonIndex := 0;
end;

procedure TFmMain.FormResize(Sender: TObject);
var
  bResult: Boolean;
begin
  if BlMain.ActiveButtonIndex >= 0 then
  begin
    bResult := True;
    IChild(FFrames[BlMain.ActiveButtonIndex]).Notify(ntResize, bResult);
  end;
end;

procedure TFmMain.WMEraseBkgnd(var AMessage: TWmEraseBkgnd);
begin
  AMessage.Result := 1;
end;

procedure TFmMain.ETSMMainThreadLock(var AMessage: TMessage);
begin
  TETSLock(AMessage.LParam).MainThreadWait;
end;

procedure TFmMain.WMSysCommand(var AMessage: TWMSysCommand);
begin
  inherited;

  if AMessage.CmdType and $FFF0 = SC_MINIMIZE then
    Hide;
end;

procedure TFmMain.Resizing(AState: TWindowState);
begin
  inherited;

  if AState = wsMinimized then
    Hide;
end;

procedure TFmMain.AddChild(ACaption: WideString; AChild: IChild);
begin
  if not Assigned(AChild) then
    Exit;

  BlMain.AddButton(ACaption, nil);

  AChild.Init(Self, BlMain.ButtonCount - 1);
  FFrames.Add(AChild);
end;

var
  GConfig: Pointer;

function TFmMain.GetParam(AIndex: Integer): TParam;
begin
  Result.FHandle := Handle;
  Result.FRect := PnlMain.BoundsRect;
  Result.FParent := PnlMain;
  Result.FConfig := GConfig;
end;

type
  TDelayLoaded = class(TInterfacedBase, IChild)
  private
    FChild: IChild;
    FConfig: TQJson;
  public
    constructor Create(AConfig: TQJson); reintroduce;
  private
    {IChild实现}
    FIndex: Integer;
    FParent: IParent;
    procedure Init(AParent: IParent; AIndex: Integer);
    procedure Notify(ANotifyType: TNotifyType; var AResult: Boolean);
  end;

{ TDelayLoaded }

constructor TDelayLoaded.Create(AConfig: TQJson);
begin
  inherited Create(nil);

  FConfig := AConfig.Copy;
end;

procedure TDelayLoaded.Init(AParent: IParent; AIndex: Integer);
begin
  FParent := AParent;
  FIndex := AIndex;
end;

procedure TDelayLoaded.Notify(ANotifyType: TNotifyType; var AResult: Boolean);
var
  strType: String;
begin
  AResult := False;

  if (ANotifyType = ntActive) and not Assigned(FChild) then
  begin
    strType := FConfig.ValueByName('Type', '');
    if CompareText(strType, 'Script') = 0 then
      FChild := TFrmScriptFrame.Create(nil)
    else if CompareText(strType, 'Dll') = 0 then
      FChild := TLibraryManager.LoadFrame(FConfig.ValueByName('File', ''), FConfig.ValueByName('Class', ''));

    if not Assigned(FChild) then
    begin
      GetRawManager.Service['Log'].AddLog(Format('子控件(%s)创建失败', [FConfig.ValueByName('Class', '')]));
      Exit;
    end;

    GConfig := FConfig;
    FChild.Init(FParent, FIndex);
  end;

  AResult := True; //如果子窗口创建不成功，则所有Notify按成功处理，解决出错后无法关闭窗口的问题
  if not Assigned(FChild) then
    Exit;

  FChild.Notify(ANotifyType, AResult);
end;

procedure CreateForm;
  procedure initMainForm(AForm: TFmMain; const AConfig: TQJson);
  var
    i: Integer;
  begin
    if not Assigned(AConfig) then
      Exit;

    for i := 0 to AConfig.Count - 1 do
      SetPropValue(AForm, AConfig[i].Name, AConfig[i].Value);
  end;
  procedure initManager(AParent: IParent; const AConfig: TQJson);
  var
    i: Integer;
    js: TQJson;
    frmChild: TFrmChild;
  begin
    if not Assigned(AConfig) or not AConfig.IsArray then
      Exit;

    for i := 0 to AConfig.Count - 1 do
    begin
      js := AConfig[i];

      if CompareText(js.ValueByName('Type', ''), 'Frames') = 0 then
      begin
        frmChild := TFrmChild.Create(nil);
        AParent.AddChild(js.ValueByName('Caption', ''), frmChild);
        initManager(frmChild, js.ItemByName('Frames'));

        Continue;
      end;

      AParent.AddChild(js.ValueByName('Caption', ''), TDelayLoaded.Create(js));
    end;
  end;
var
  fm: TFmMain;
  jsConfig: TQJson;
begin
  Application.CreateForm(TFmMain, fm);

  jsConfig := TQJson.Create;
  try
    jsConfig.LoadFromFile('.\Config\Framework.json');

    initMainForm(fm, jsConfig.ItemByName('MainForm'));
    initManager(fm, jsConfig.ItemByName('Frames'));
  finally
    FreeAndNil(jsConfig);
  end;
end;

procedure DoIdle(ASender: TObject);
begin
  if Assigned(ASender) and (ASender is TForm) then
    TForm(ASender).Perform(CM_IDLE, 0, 0);
end;

initialization
  TAppInit.RegCreateForm(CreateForm, DoIdle);

end.

