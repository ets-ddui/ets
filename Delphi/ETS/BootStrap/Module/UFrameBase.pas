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
unit UFrameBase;

interface

uses
  Windows, Classes, Forms, Controls, SysUtils, Messages, ShellAPI,
  UInterface, UDUIForm;

type
  TOnInit = procedure(AParent: IParent; AIndex: Integer) of object;
  //���AResult���Ӵ����л�ʱʹ�ã���ʾ�Ƿ������Ӵ����л���True: �����л� False: ͣ���ڵ�ǰ����
  TOnNotify = procedure(ANotifyType: TNotifyType; var AResult: Boolean) of object;
  TOnAfterNotify = procedure(ANotifyType: TNotifyType) of object;
  //AIndex��1��ʼ����
  TOnDropFiles = procedure(AFileName: String; AIndex, ACount: Integer) of object;
  TFrameBase = class;
  TFrameBaseClass = class of TFrameBase;
  TFrameBase = class(TDUIFrame, IChild)
  private
    {IChildʵ��}
    FIndex: Integer;
    FRefCount: Integer;
    FParent: IParent;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
    procedure Init(AParent: IParent; AIndex: Integer);
    procedure Notify(ANotifyType: TNotifyType; var AResult: Boolean);
  private
    FOnInit: TOnInit;
    FOnNotify: TOnNotify;
    FOnAfterNotify: TOnAfterNotify;
    FAcceptDropFiles: Boolean;
    FOnDropFiles: TOnDropFiles;
    function GetParam: TParam;
    procedure SetAcceptDropFiles(const AValue: Boolean);
    procedure CommonNotify(ANotifyType: TNotifyType; var AResult: Boolean);
    procedure WMDropFiles(var AMessage: TMessage); message WM_DROPFILES;
  private
    class var
      FFrameList: TInterfaceList;
      FForm: TDUIForm;
    class procedure InitClass;
    class procedure UnInitClass;
  public
    class procedure RegFrame(AFrameBaseClass: TFrameBaseClass);
    class function GetFrame(AID: String; AOwner: TComponent): IChild;
    class function GetMainForm: TDUIForm;
  public
    property AcceptDropFiles: Boolean read FAcceptDropFiles write SetAcceptDropFiles;
  published
    property Enabled;
    property Param: TParam read GetParam;
    property OnDropFiles: TOnDropFiles read FOnDropFiles write FOnDropFiles;
    property OnInit: TOnInit read FOnInit write FOnInit;
    property OnNotify: TOnNotify read FOnNotify write FOnNotify;
    property OnAfterNotify: TOnAfterNotify read FOnAfterNotify write FOnAfterNotify;
    property OnResize;
  end;

implementation

type
  TChildCreator = class(TInterfacedObject, IChildCreator)
  private
    FFrameBaseClass: TFrameBaseClass;
  private
    {IChildCreatorʵ��}
    function GetID: WideString;
    function CreateChild(AOwner: TComponent): IChild;
  public
    constructor Create(AFrameBaseClass: TFrameBaseClass);
  end;

{ TChildCreator }

constructor TChildCreator.Create(AFrameBaseClass: TFrameBaseClass);
begin
  FFrameBaseClass := AFrameBaseClass;
end;

function TChildCreator.CreateChild(AOwner: TComponent): IChild;
begin
  Result := FFrameBaseClass.Create(AOwner);
end;

function TChildCreator.GetID: WideString;
begin
  Result := FFrameBaseClass.ClassName;
end;

{ TFrameBase }

function TFrameBase.GetParam: TParam;
begin
  Result := FParent.Param[FIndex];
end;

procedure TFrameBase.Init(AParent: IParent; AIndex: Integer);
begin
  FParent := AParent;
  FIndex := AIndex;

  if Assigned(FOnInit) then
    FOnInit(AParent, AIndex);
end;

procedure TFrameBase.Notify(ANotifyType: TNotifyType; var AResult: Boolean);
begin
  AResult := True;

  if Assigned(FOnNotify) then
    FOnNotify(ANotifyType, AResult);

  case ANotifyType of
    ntActive, ntDeActive:
    begin
      if AResult then
        CommonNotify(ANotifyType, AResult);

      if AResult and FAcceptDropFiles then
      begin
        case ANotifyType of
          ntActive: DragAcceptFiles(GetParam.FHandle, True);
          ntDeActive: DragAcceptFiles(GetParam.FHandle, False);
        end;
      end;
    end;
  else
    CommonNotify(ANotifyType, AResult);
  end;

  if Assigned(FOnAfterNotify) then
    FOnAfterNotify(ANotifyType);
end;

procedure TFrameBase.SetAcceptDropFiles(const AValue: Boolean);
begin
  FAcceptDropFiles := AValue;

  if not (csDesigning in ComponentState) and (0 <> GetParam.FHandle) then
    DragAcceptFiles(GetParam.FHandle, FAcceptDropFiles);
end;

procedure TFrameBase.WMDropFiles(var AMessage: TMessage);
const
  cMaxSize = $FFFFFFFF;
var
  i, iCount, iSize: Integer;
  str: String;
begin
  try
    if Assigned(FOnDropFiles) then
    begin
      iCount := DragQueryFile(AMessage.WParam, cMaxSize, nil, 0);
      for i := 0 to iCount - 1 do
      begin
        iSize := DragQueryFile(AMessage.WParam, i, nil, 0);
        if iSize > 0 then
        begin
          SetLength(str, iSize + 1); //�������Ŀս�β��
          DragQueryFile(AMessage.WParam, i, @str[1], iSize + 1);
          SetLength(str, iSize);
          FOnDropFiles(str, i + 1, iCount);
        end;
      end;
    end;
  finally
    DragFinish(AMessage.WParam);
    AMessage.Result := 0;
  end;
end;

function TFrameBase._AddRef: Integer;
begin
  Result := InterlockedIncrement(FRefCount);
end;

function TFrameBase._Release: Integer;
begin
  Result := InterlockedDecrement(FRefCount);
  if Result = 0 then
    Destroy;
end;

class procedure TFrameBase.RegFrame(AFrameBaseClass: TFrameBaseClass);
begin
  FFrameList.Add(TChildCreator.Create(AFrameBaseClass));
end;

class function TFrameBase.GetFrame(AID: String; AOwner: TComponent): IChild;
var
  i: Integer;
  cc: IChildCreator;
begin
  Result := nil;

  for i := 0 to FFrameList.Count - 1 do
  begin
    cc := FFrameList[i] as IChildCreator;
    if cc.GetID = AID then
    begin
      Result := cc.CreateChild(AOwner);
      Exit;
    end;
  end;
end;

class function TFrameBase.GetMainForm: TDUIForm;
begin
  Result := FForm;
end;

procedure TFrameBase.CommonNotify(ANotifyType: TNotifyType; var AResult: Boolean);
begin
  AResult := True;

  case ANotifyType of
    ntActive:
    begin
//      if Assigned(GetParentForm(GetParam.FParent)) then
      begin
        {$IFDEF DESIGNTIME}
        {$ELSE}
        DUIParent := GetParam.FParent;
        {$ENDIF}
        Align := alClient;
        Visible := True;
        Exit;
      end;

      Parent := FForm;
      Align := alClient;
      Visible := True;

      FForm.Show; //���������ô��ڳߴ�֮ǰ���ã��������õĳߴ粻��Ч

      with GetParam do
      begin
        FForm.ParentWindow := FHandle;
        FForm.SetBounds(FRect.Left, FRect.Top, FRect.Right - FRect.Left, FRect.Bottom - FRect.Top);
      end;
    end;
    ntToggle:
    begin
      if not Assigned(DUIParent) then
        //�״δ�������ʱ���⴦��(��ʱVisibleΪTrue)
        DUIParent := GetParam.FParent
      else
        Visible := not Visible;
    end;
    ntDeActive:
    begin
      if Parent <> FForm then
      begin
        Visible := False;
        Exit;
      end;

      //�����FForm.ParentWindow���㣬���������ø�����ʱ���ᴥ�������ؽ�������
      //���������������Ϣ���������޷���������
      //if AFrame.Parent = FForm then
      //begin
      //  FForm.Hide;
      //  FForm.ParentWindow := 0;
      //end;

      //���ﲻ��ֱ���ͷ�AFrame.Parent������AFrame�������о���Ŀؼ����ᱻ�ͷţ�
      //����IChild.Notify������߼��޷�����ִ��(���磬TFrmPressTest.Notify��ɾ�����ڵ�Ĵ���)
      //AFrame.Parent := nil;
      Visible := False;
      FForm.Hide;
    end;
    ntResize:
    begin
      if Parent = FForm then
        with GetParam do
          FForm.SetBounds(FRect.Left, FRect.Top, FRect.Right - FRect.Left, FRect.Bottom - FRect.Top);
    end;
    {$IFDEF PACKAGE_COMPILE_MODE}
    ntIdle:
    begin
      if GetCurrentThreadId = MainThreadID then
        CheckSynchronize;
    end;
    {$ENDIF}
  end;
end;

class procedure TFrameBase.InitClass;
begin
  FFrameList := TInterfaceList.Create;

  //{$IFDEF PACKAGE_COMPILE_MODE}
  FForm := TDUIForm.CreateNew(nil);
  //{$ENDIF}
end;

class procedure TFrameBase.UnInitClass;
begin
  FreeAndNil(FFrameList);

  //{$IFDEF PACKAGE_COMPILE_MODE}
  FreeAndNil(FForm);
  //{$ENDIF}
end;

initialization
  TFrameBase.InitClass;

finalization
  TFrameBase.UnInitClass;

end.
