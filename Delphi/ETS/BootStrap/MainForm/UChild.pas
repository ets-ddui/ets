{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)������չ���߼���

  ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
  �����˿��Ŀ����ϣ�������ã��������κα�֤��
  ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

  ��Դ��ַ: https://github.com/ets-ddui/ets
            https://gitee.com/ets-ddui/ets
  ��ԴЭ��: The MIT License (MIT)
  ��������: xinghun87@163.com
  �ٷ����ͣ�https://blog.csdn.net/xinghun61
}
unit UChild;

{$i UConfigure.inc}

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, ComCtrls, UInterface, UDUICore, UDUIForm, UDUIButton, UDUIPanel;

type
  TFrmChild = class(TDUIFrame, IInterface, IChild, IParent)
    BlMain: TDUIButtonList;
    PnlMain: TDUIPanel;
    procedure BlMainChanging(ASender: TObject; AOldIndex, ANewIndex: Integer;
      var ACanChange: Boolean);
  private
    FFrames: TInterfaceList;
  private
    {IInterfaceʵ��}
    FRefCount: Integer;
    function QueryInterface(const AIID: TGUID; out AResult): HResult; reintroduce; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  private
    {IChildʵ��}
    FIndex: Integer;
    FParent: IParent;
    procedure Init(AParent: IParent; AIndex: Integer);
    procedure Notify(ANotifyType: TNotifyType; var AResult: Boolean);
  private
    {IParentʵ��}
    //IParent�����ü���Ϊ��ʵ�֣�����IChild��ʵ�����ü�����
    //�����������������ͷ�FFramesʱ�����Ӷ����IParent�����ûᵼ���ظ�����Destroy��
    //�����������쳣����
    //�Ӳ��Խ������IChild�����ü���Release��1ʱ��Ҳ�ᴥ��Destroy����Destroy��Ϊ�ͷ�
    //�Ӷ�����IParent��Ϊ0���ٴδ���Destroy�������쳣
    function _AddRef_Parent: Integer; stdcall;
    function _Release_Parent: Integer; stdcall;
    procedure AddChild(ACaption: WideString; AChild: IChild);
    function GetParam(AIndex: Integer): TParam;
    function IParent._AddRef = _AddRef_Parent;
    function IParent._Release = _Release_Parent;
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
  end;

implementation

{$R *.dfm}

{ TFrmTools }

procedure TFrmChild.BlMainChanging(ASender: TObject; AOldIndex,
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

constructor TFrmChild.Create(AOwner: TComponent);
begin
  inherited;

  FFrames := TInterfaceList.Create;
end;

destructor TFrmChild.Destroy;
begin
  FreeAndNil(FFrames);

  inherited;
end;

procedure TFrmChild.Init(AParent: IParent; AIndex: Integer);
begin
  FParent := AParent;
  FIndex := AIndex;
end;

procedure TFrmChild.Notify(ANotifyType: TNotifyType; var AResult: Boolean);
begin
  case ANotifyType of
    ntActive:
    begin
      DUIParent := FParent.Param[FIndex].FParent;
      Align := alClient;
      Visible := True;

      if (BlMain.ButtonCount > 0) and (BlMain.ActiveButtonIndex < 0) then
        BlMain.ActiveButtonIndex := 0
      else if BlMain.ActiveButtonIndex >= 0 then
        IChild(FFrames[BlMain.ActiveButtonIndex]).Notify(ANotifyType, AResult);
    end;
    ntDeActive:
    begin
      if BlMain.ActiveButtonIndex >= 0 then
      begin
        IChild(FFrames[BlMain.ActiveButtonIndex]).Notify(ANotifyType, AResult);
        if not AResult then
          Exit;
      end;

      Visible := False;
    end;
  end;
end;

procedure TFrmChild.AddChild(ACaption: WideString; AChild: IChild);
begin
  if not Assigned(AChild) then
    Exit;

  BlMain.AddButton(ACaption, nil);

  AChild.Init(Self, BlMain.ButtonCount - 1);
  FFrames.Add(AChild);
end;

function TFrmChild.GetParam(AIndex: Integer): TParam;
begin
  with FParent.Param[FIndex] do
  begin
    Result.FHandle := FHandle;
    Result.FRect := Rect(FRect.Left, FRect.Top + PnlMain.Top, FRect.Right, FRect.Bottom);
    Result.FParent := PnlMain;
    Result.FConfig := FConfig;
  end;
end;

function TFrmChild.QueryInterface(const AIID: TGUID; out AResult): HResult;
begin
  Result := inherited QueryInterface(AIID, AResult);
end;

function TFrmChild._AddRef: Integer; stdcall;
begin
  Result := InterlockedIncrement(FRefCount);
end;

function TFrmChild._Release: Integer; stdcall;
begin
  Result := InterlockedDecrement(FRefCount);
  if Result = 0 then
    Destroy;
end;

function TFrmChild._AddRef_Parent: Integer; stdcall;
begin
  Result := -1;
end;

function TFrmChild._Release_Parent: Integer; stdcall;
begin
  Result := -1;
end;

end.
