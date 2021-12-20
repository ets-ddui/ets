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
    FFrames: TInterfaceList;
    FSysButtons: TInterfaceList;
    FTrayIconService: TTrayIconService;
    function AddButtonChild(AChild: IChild): TDUIButton;
    procedure WMSysCommand(var AMessage: TWMSysCommand); message WM_SYSCOMMAND;
    procedure WMEraseBkgnd(var AMessage: TWmEraseBkgnd); message WM_ERASEBKGND;
    procedure CMIdle(var AMessage: TMessage); message CM_IDLE;
    procedure ETSMMainThreadLock(var AMessage: TMessage); message ETSM_MAIN_THREAD_LOCK;
  protected
    procedure DoClick(ASender: TObject); override;
    procedure Resizing(AState: TWindowState); override;
  private
    {IParentʵ��}
    procedure AddChild(ACaption: WideString; AChild: IChild);
    function GetParam(AIndex: Integer): TParam;
  end;

implementation

uses
  IniFiles, Math, StrUtils, Variants, ActiveX, TypInfo, qjson, qstring,
  UFrameBase, UChild, USettingManager, ULogManager, UTool, ULibraryManager,
  UAppInit, UService, UDUIShape, UDUIUtils, UScript;

{$R *.dfm}

const
  CSysButtons: Integer = 100000; //SysButtons����ʼ���

{ TFmMain }

procedure TFmMain.BlMainChanging(ASender: TObject; AOldIndex,
  ANewIndex: Integer; var ACanChange: Boolean);
begin
  if AOldIndex >= 0 then
  begin
    //1.0 �ͷ��ϴ��ڵ���Դ
    IChild(FFrames[AOldIndex]).Notify(ntDeActive, ACanChange);
    if not ACanChange then
      Exit;
  end;

  if ANewIndex >= 0 then
  begin
    //2.0 �����´���
    IChild(FFrames[ANewIndex]).Notify(ntActive, ACanChange);

    //3.0 �´��ڽ���ʧ�ܣ�����˵��ϴ���
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
var
  iIndex: Integer;
  bCanChange: Boolean;
begin
  iIndex := TDUIButton(ASender).Tag - CSysButtons;
  if (iIndex < 0) or (iIndex >= FSysButtons.Count) then
  begin
    inherited;
    Exit;
  end;

  bCanChange := True;
  IChild(FSysButtons[iIndex]).Notify(ntToggle, bCanChange);
end;

procedure TFmMain.FormClose(ASender: TObject; var AAction: TCloseAction);
begin
  //1.0 �ͷ��Ӵ���
  FreeAndNil(FFrames);
  FreeAndNil(FSysButtons);

  //2.0 �ͷ�����DLL�������ɱ�֤DLL�еľ�̬�������õ�FManager���ͷ�(���磬UQueueManager.TQueue.FManager)
  TLibraryManager.UnInit(True);

  //3.0 �ͷ�FManager�������Դ
  FreeAndNil(FTrayIconService);
  SetManager(nil);
end;

procedure TFmMain.FormCloseQuery(ASender: TObject; var ACanClose: Boolean);
begin
  BlMainChanging(BlMain, BlMain.ActiveButtonIndex, -1, ACanClose);
end;

procedure TFmMain.FormCreate(Sender: TObject);
begin
  ImgBackground.Picture.SkinName := 'SYSTEM.BACKGROUND';

  FFrames := TInterfaceList.Create;
  FSysButtons := TInterfaceList.Create;

  FTrayIconService := TTrayIconService.Create(nil);

  SetManager(TManager.Create(nil, TService.Create(FTrayIconService)));
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

function TFmMain.AddButtonChild(AChild: IChild): TDUIButton;
begin
  Result := nil;
  if not Assigned(AChild) then
    Exit;

  Result := AddSysButton(stNone);
  Result.Tag := FSysButtons.Count + CSysButtons;
  AChild.Init(Self, FSysButtons.Count + CSysButtons);
  FSysButtons.Add(AChild);
end;

var
  GConfig: Pointer;

function TFmMain.GetParam(AIndex: Integer): TParam;
begin
  Result.FHandle := Handle;
  if AIndex >= CSysButtons then
  begin
    Result.FRect := ImgBackground.BoundsRect;
    Result.FParent := ImgBackground;
  end
  else
  begin
    Result.FRect := PnlMain.BoundsRect;
    Result.FParent := PnlMain;
  end;
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
    {IChildʵ��}
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

  if (ANotifyType in [ntActive, ntToggle]) and not Assigned(FChild) then
  begin
    strType := FConfig.ValueByName('Type', '');
    if CompareText(strType, 'Script') = 0 then
      FChild := TFrmScriptFrame.Create(nil)
    else if CompareText(strType, 'Dll') = 0 then
      FChild := TLibraryManager.LoadFrame(FConfig.ValueByName('File', ''), FConfig.ValueByName('Class', ''));

    if not Assigned(FChild) then
    begin
      GetRawManager.Service['Log'].AddLog(Format('�ӿؼ�(%s)����ʧ��', [FConfig.ValueByName('Class', '')]));
      Exit;
    end;

    GConfig := FConfig;
    FChild.Init(FParent, FIndex);
  end;

  AResult := True; //����Ӵ��ڴ������ɹ���������Notify���ɹ��������������޷��رմ��ڵ�����
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
  procedure initSysButtons(AForm: TFmMain; const AConfig: TQJson);
  var
    i: Integer;
    js: TQJson;
    obj: TObject;
  begin
    if not Assigned(AConfig) or not AConfig.IsArray then
      Exit;

    for i := 0 to AConfig.Count - 1 do
    begin
      js := AConfig[i];
      obj := AForm.AddButtonChild(TDelayLoaded.Create(js)).Shape;
      JsonToComponent(obj, js, nil);
    end;
  end;
  procedure initFrames(AParent: IParent; const AConfig: TQJson);
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
        initFrames(frmChild, js.ItemByName('Frames'));

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
    initSysButtons(fm, jsConfig.ItemByName('SysButtons'));
    initFrames(fm, jsConfig.ItemByName('Frames'));
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

