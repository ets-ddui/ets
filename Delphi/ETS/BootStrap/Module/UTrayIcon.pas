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
unit UTrayIcon;

interface

uses
  Windows, Classes, SysUtils, ExtCtrls, Menus, SyncObjs, Forms, Messages,
  UInterface;

type
  {$IFDEF LAZARUS}
  {$M+}
  {$ELSE}
  {$METHODINFO ON}
  {$ENDIF}
  TTrayIconService = class(TComponent, ITrayIconEvent)
  private
    class var FLock: TCriticalSection;
    class var FRegisteredID: TBits;
    class procedure Init;
    class procedure UnInit;
  strict private
    FTrayIcon: TTrayIcon;
    FMenu: TPopupMenu;
    function CheckID(AID: Integer): Boolean;
    procedure DoTrayIconClick(ASender: TObject);
  private
    { ITrayIconEvent实现 }
    procedure DoClick(AID, ASubID: Integer);
  public
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
  published
    function RegisterID: Integer;
    function AppendMenuItem(ACaption: WideString; AID, ASubID: Integer;
      AParent: THandle; ATrayIconEvent: ITrayIconEvent): THandle;
    //如果ASubID传入-1，表示删除与AID关联的所有菜单
    procedure ClearMenuItem(AID, ASubID: Integer);
  end;
  {$IFDEF LAZARUS}
  {$M-}
  {$ELSE}
  {$METHODINFO OFF}
  {$ENDIF}

implementation

const
  CSystemID: Integer = 1024; //系统ID的范围，小于此值的ID不提供给扩展模块使用

type
  TTrayMenuItem = class(TMenuItem)
  strict private
    FID, FSubID: Integer;
    FTrayIconEvent: ITrayIconEvent;
    procedure DoMenuClick(ASender: TObject);
  public
    constructor Create(AOwner: TComponent; AID, ASubID: Integer;
      ATrayIconEvent: ITrayIconEvent); reintroduce;
    destructor Destroy; override;
    function IsEqual(AID, ASubID: Integer): Boolean;
  end;

{ TTrayMenuItem }

constructor TTrayMenuItem.Create(AOwner: TComponent; AID, ASubID: Integer;
  ATrayIconEvent: ITrayIconEvent);
begin
  inherited Create(AOwner);

  FID := AID;
  FSubID := ASubID;
  FTrayIconEvent := ATrayIconEvent;
  OnClick := DoMenuClick;
end;

destructor TTrayMenuItem.Destroy;
begin
  FTrayIconEvent := nil;
  inherited;
end;

procedure TTrayMenuItem.DoMenuClick(ASender: TObject);
begin
  FTrayIconEvent.DoClick(FID, FSubID);
end;

function TTrayMenuItem.IsEqual(AID, ASubID: Integer): Boolean;
begin
  Result := False;
  if FID <> AID then
    Exit;

  if (ASubID >= 0) and (FSubID <> ASubID) then
    Exit;

  Result := True;
end;

{ TTrayIconService }

function TTrayIconService.RegisterID: Integer;
begin
  FLock.Enter;
  try
    Result := FRegisteredID.OpenBit;
    if Result >= FRegisteredID.Size then
      FRegisteredID.Size := FRegisteredID.Size + SizeOf(Integer) * 8;
    FRegisteredID[Result] := True;
  finally
    FLock.Leave;
  end;

  Result := Result + CSystemID;
end;

function TTrayIconService.CheckID(AID: Integer): Boolean;
begin
  Result := False;
  if AID < CSystemID then
    Exit;

  FLock.Enter;
  try
    Result := FRegisteredID[AID - CSystemID];
  finally
    FLock.Leave;
  end;
end;

function TTrayIconService.AppendMenuItem(ACaption: WideString; AID, ASubID: Integer;
  AParent: THandle; ATrayIconEvent: ITrayIconEvent): THandle;
var
  mi: TTrayMenuItem;
begin
  Result := 0;
  if not CheckID(AID) or (ASubID < 0) then
    Exit;

  mi := TTrayMenuItem.Create(FMenu, AID, ASubID, ATrayIconEvent);
  mi.Caption := ACaption;
  if AParent = 0 then
    FMenu.Items.Insert(FMenu.Items.Count - 1, mi)
  else
    TTrayMenuItem(AParent).Insert(TTrayMenuItem(AParent).Count, mi);

  Result := THandle(mi);
end;

procedure TTrayIconService.ClearMenuItem(AID, ASubID: Integer);
  procedure doClear(AMenuItem: TMenuItem);
  var
    i: Integer;
  begin
    if AMenuItem is TTrayMenuItem then
    begin
      if TTrayMenuItem(AMenuItem).IsEqual(AID, ASubID) then
      begin
        AMenuItem.Free;
        Exit;
      end;
    end;

    for i := AMenuItem.Count - 1 downto 0 do
      doClear(AMenuItem[i]);
  end;
begin
  if not CheckID(AID) then
    Exit;

  doClear(FMenu.Items);
end;

constructor TTrayIconService.Create(AOwner: TComponent);
const
  cMenuItemName: array[0..2] of String = ('显示', '隐藏', '关闭');
var
  i: Integer;
  mi: TTrayMenuItem;
begin
  inherited;

  FMenu := TPopupMenu.Create(AOwner);
  FTrayIcon := TTrayIcon.Create(AOwner);
  FTrayIcon.PopupMenu := FMenu;
  FTrayIcon.OnClick := DoTrayIconClick;
  FTrayIcon.Icon.LoadFromResourceName(HInstance, 'MAINICON');
  FTrayIcon.Visible := True;

  for i := Low(cMenuItemName) to High(cMenuItemName) do
  begin
    mi := TTrayMenuItem.Create(FMenu, 0, i, Self);
    mi.Caption := cMenuItemName[i];
    FMenu.Items.Insert(FMenu.Items.Count, mi);
  end;
end;

destructor TTrayIconService.Destroy;
begin
  FreeAndNil(FTrayIcon);
  FreeAndNil(FMenu);
end;

procedure TTrayIconService.DoClick(AID, ASubID: Integer);
begin
  case ASubID of
    0: //显示
    begin
      if not Application.MainForm.Visible then
      begin
        Application.MainForm.Show;
        SetForegroundWindow(Application.MainForm.Handle);
        PostMessage(Application.MainForm.Handle, WM_SYSCOMMAND, SC_RESTORE, 0);
      end;
    end;
    1: //隐藏
    begin
      if Application.MainForm.Visible then
        Application.MainForm.Hide;
    end;
    2: //关闭
    begin
      Application.MainForm.Close;
    end;
  end;
end;

procedure TTrayIconService.DoTrayIconClick(ASender: TObject);
begin
  if Application.MainForm.Visible then
    DoClick(0, 1)
  else
    DoClick(0, 0);
end;

class procedure TTrayIconService.Init;
begin
  FLock := TCriticalSection.Create;
  FRegisteredID := TBits.Create;
end;

class procedure TTrayIconService.UnInit;
begin
  FreeAndNil(FRegisteredID);
  FreeAndNil(FLock);
end;

initialization
  TTrayIconService.Init;

finalization
  TTrayIconService.UnInit;

end.
