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
    {IInterface实现}
    FRefCount: Integer;
    function QueryInterface(const AIID: TGUID; out AResult): HResult; reintroduce; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  private
    {IChild实现}
    FIndex: Integer;
    FParent: IParent;
    procedure Init(AParent: IParent; AIndex: Integer);
    procedure Notify(ANotifyType: TNotifyType; var AResult: Boolean);
  private
    {IParent实现}
    //IParent的引用计数为空实现，仅在IChild上实现引用计数，
    //否则，在析构函数中释放FFrames时，其子对象对IParent的引用会导致重复调用Destroy，
    //导致析构报异常——
    //从测试结果看，IChild当引用计数Release到1时，也会触发Destroy，而Destroy因为释放
    //子对象导致IParent降为0，再次触发Destroy，导致异常
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
