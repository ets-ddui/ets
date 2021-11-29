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
unit UETSMenu;

interface

uses
  Classes, SysUtils, Forms, Menus, ToolsAPI, StructureViewAPI,
  UETSWizard, UETSCommon;

type
  TETSMenuManager = class
  private
    class var FManager: TETSMenuManager;
  public
    class function GetManager: TETSMenuManager;
    class procedure Init(const ABorlandIDEServices: IBorlandIDEServices;
      ARegisterProc: TWizardRegisterProc);
    class procedure UnInit;
  end;

implementation

uses
  Variants, TypInfo, Contnrs, UETSNotifier, UDUIWizard, UTool;

const
  CMenuDelimiter: Char = '/';

type
  TETSMenuWizard = class(TDUIWizard, IOTAMenuWizard)
  private
    //IOTAMenuWizard实现
    function GetMenuText: String;
  protected
    function GetName: String; override;
    procedure Execute; override;
  end;

  //自定义菜单的基类，提供查找、添加菜单的功能
  TETSMenu = class
  public
    function FindMenu(ARoot: TMenuItem; APath: String): TMenuItem;
    function AddMenu(ARoot: TMenuItem; APath: String; AIndex: Integer;
      AOnClick: TNotifyEvent = nil; ATag: Integer = 0): TMenuItem;
  end;

  //对释放时间的包装，使用此类，可保证业务对象不需从TComponent继承
  TOnNotification = procedure (AComponent: TComponent; AOperation: TOperation) of object;
  TFreeNotification = class(TComponent)
  private
    FOnNotification: TOnNotification;
  protected
    procedure Notification(AComponent: TComponent; AOperation: TOperation); override;
  public
    property OnNotification: TOnNotification read FOnNotification write FOnNotification;
  end;

  //对象事件注册类
  //在菜单项原来的弹出事件能正常执行的前提下，添加自定义的事件功能
  //实现逻辑是将原事件记录下来，然后，用TNotifyEventHook.NewNotifyEvent替换，
  //然后，在NewNotifyEvent中分别调用原事件和自定义的事件
  TOnBeforeNotifyEvent = procedure(ASender: TObject; var ACancel: Boolean) of object;
  TNotifyEventHook = class
  private
    FOldNotifyEvent: TNotifyEvent;
    FHookedComponent: TComponent;
    FHookedPropInfo: PPropInfo;
    FFreeNotification: TFreeNotification;
    FOnBeforeNotifyEvent: TOnBeforeNotifyEvent;
    FOnAfterNotifyEvent: TNotifyEvent;
    procedure NewNotifyEvent(ASender: TObject);
    procedure Notification(AComponent: TComponent; AOperation: TOperation);
    function GetHooked: Boolean;
  public
    constructor Create;
    destructor Destroy; override;
    function Hook(AComponent: TComponent; ANotifyName: String): Boolean;
    procedure UnHook;
    property Hooked: Boolean read GetHooked;
    property OnBeforeNotifyEvent: TOnBeforeNotifyEvent read FOnBeforeNotifyEvent write FOnBeforeNotifyEvent;
    property OnAfterNotifyEvent: TNotifyEvent read FOnAfterNotifyEvent write FOnAfterNotifyEvent;
  end;

  //对Delphi主菜单的包装
  TETSMainMenu = class(TETSMenu)
  private
    FETSMenu: TMenuItem;
    FFreeNotification: TFreeNotification;
    function GetMainMenu: TMainMenu;
    procedure Notification(AComponent: TComponent; AOperation: TOperation);
    procedure DoClickPrintProjectParameter(ASender: TObject);
    procedure DoClickPrintFormProperty(ASender: TObject);
    procedure DoShowStructureView(ASender: TObject);
  public
    constructor Create;
    destructor Destroy; override;
    property MainMenu: TMainMenu read GetMainMenu;
  end;

  //对Delphi弹出菜单的包装
  TETSPasPopupMenu = class(TETSMenu)
  private
    FIDENotifier: Integer;
    FFoldAll: TMenuItem;
    FFreeNotification: TFreeNotification;
    FNotifyEventHook: TNotifyEventHook;
    function GetPopupMenu: TPopupMenu;
    procedure DoIDENotifier(ANotifyCode: TOTAFileNotification; const AFileName: String);
    procedure Notification(AComponent: TComponent; AOperation: TOperation);
    procedure DoPopup(ASender: TObject);
    procedure DoClickFoldAll(ASender: TObject);
  public
    constructor Create;
    destructor Destroy; override;
    property PopupMenu: TPopupMenu read GetPopupMenu;
  end;

{ TETSMenuWizard }

procedure TETSMenuWizard.Execute;
begin
end;

function TETSMenuWizard.GetMenuText: String;
begin
  Result := 'ETS Help';
end;

function TETSMenuWizard.GetName: String;
begin
  Result := 'ETS Help Menu';
end;

{ TETSMenu }

function TETSMenu.AddMenu(ARoot: TMenuItem; APath: String; AIndex: Integer;
  AOnClick: TNotifyEvent; ATag: Integer): TMenuItem;
var
  i: Integer;
  slst: TStringList;
  mi: TMenuItem;
begin
  Result := ARoot;

  if Trim(APath) <> '' then
  begin
    slst := TStringList.Create;
    try
      slst.StrictDelimiter := True;
      slst.Delimiter := CMenuDelimiter;
      slst.DelimitedText := Trim(APath);

      for i := 0 to slst.Count - 1 do
      begin
        mi := Result.Find(slst.Strings[i]);
        if Assigned(mi) then
        begin
          Result := mi;
          Continue;
        end;

        mi := TMenuItem.Create(nil);
        mi.Caption := slst.Strings[i];
        if i < slst.Count - 1 then //不是叶子节点，则创建菜单路径
          Result.Insert(Result.Count, mi)
        else if (AIndex < 0) or (AIndex > Result.Count) then //剩下的两个条件分支都是用于创建叶子节点的菜单
                                                             //前者在最后添加菜单，后者在指定位置添加菜单
          Result.Insert(Result.Count, mi)
        else
          Result.Insert(AIndex, mi);
        Result := mi;
      end;
    finally
      FreeAndNil(slst);
    end;
  end;

  Result.Tag := ATag;
  Result.OnClick := AOnClick;
end;

function TETSMenu.FindMenu(ARoot: TMenuItem; APath: String): TMenuItem;
var
  i: Integer;
  slst: TStringList;
begin
  Result := ARoot;
  if Trim(APath) = '' then
    Exit;

  slst := TStringList.Create;
  try
    slst.StrictDelimiter := True;
    slst.Delimiter := CMenuDelimiter;
    slst.DelimitedText := Trim(APath);

    Result := ARoot;
    for i := 0 to slst.Count - 1 do
    begin
      Result := Result.Find(slst.Strings[i]);
      if not Assigned(Result) then
        Exit;
    end;
  finally
    FreeAndNil(slst);
  end;
end;

{ TFreeNotification }

procedure TFreeNotification.Notification(AComponent: TComponent; AOperation: TOperation);
begin
  inherited;

  if Assigned(FOnNotification) then
    FOnNotification(AComponent, AOperation);
end;

{ TNotifyEventHook }

constructor TNotifyEventHook.Create;
begin
  FOldNotifyEvent := nil;
  FHookedComponent := nil;
  FHookedPropInfo := nil;

  FFreeNotification := TFreeNotification.Create(nil);
  FFreeNotification.OnNotification := Notification;
end;

procedure TNotifyEventHook.NewNotifyEvent(ASender: TObject);
var
  bCancel: Boolean;
begin
  bCancel := False;
  if Assigned(FOnBeforeNotifyEvent) then
    FOnBeforeNotifyEvent(ASender, bCancel);

  if bCancel then
    Exit;

  if Assigned(FOldNotifyEvent) then
    FOldNotifyEvent(ASender);

  if Assigned(FOnAfterNotifyEvent) then
    FOnAfterNotifyEvent(ASender);
end;

procedure TNotifyEventHook.Notification(AComponent: TComponent; AOperation: TOperation);
begin
  if opRemove <> AOperation then
    Exit;

  if AComponent = FHookedComponent then
  begin
    FHookedComponent := nil;
    FHookedPropInfo := nil;
    FOldNotifyEvent := nil;
  end;
end;

destructor TNotifyEventHook.Destroy;
begin
  UnHook;
  FreeAndNil(FFreeNotification);

  inherited;
end;

function TNotifyEventHook.GetHooked: Boolean;
begin
  Result := Assigned(FHookedComponent);
end;

function TNotifyEventHook.Hook(AComponent: TComponent; ANotifyName: String): Boolean;
var
  pi: PPropInfo;
  me: TMethod;
  ne: TNotifyEvent;
begin
  Result := False;

  if GetHooked then
    Exit;

  pi := GetPropInfo(AComponent, ANotifyName);
  if not Assigned(pi) then
    Exit;

  if pi.PropType^.Kind <> tkMethod then
    Exit;

  me := GetMethodProp(AComponent, pi);
  TMethod(FOldNotifyEvent) := me;

  ne := NewNotifyEvent;
  me := TMethod(ne);
  SetMethodProp(AComponent, pi, me);

  FHookedComponent := AComponent;
  FHookedPropInfo := pi;
  FFreeNotification.FreeNotification(AComponent);

  Result := True;
end;

procedure TNotifyEventHook.UnHook;
var
  me, meSelf: TMethod;
  ne: TNotifyEvent;
begin
  if not GetHooked then
    Exit;

  FFreeNotification.RemoveFreeNotification(FHookedComponent);

  me := GetMethodProp(FHookedComponent, FHookedPropInfo);
  ne := NewNotifyEvent;
  meSelf := TMethod(ne);
  {TODO: 推测CnPack在ETSP之后注册，其内部记录了ETSP注册的回调地址，当ETSP重新加载后，会导致CnPack访问野指针}
  if (me.Data <> meSelf.Data) or (me.Code <> meSelf.Code) then
    Exit;

  SetMethodProp(FHookedComponent, FHookedPropInfo, TMethod(FOldNotifyEvent));
  FHookedComponent := nil;
  FHookedPropInfo := nil;
  FOldNotifyEvent := nil;
end;

{ TETSMainMenu }

constructor TETSMainMenu.Create;
var
  miRoot, miTool: TMenuItem;
  iIndex: Integer;
begin
  FFreeNotification := TFreeNotification.Create(nil);
  FFreeNotification.OnNotification := Notification;

  //在IDE主菜单的Tools菜单后面添加ETS菜单项
  miRoot := GetMainMenu.Items;
  Assert(Assigned(miRoot), 'Delphi主菜单获取失败');

  miTool := FindMenu(miRoot, 'Tools');
  Assert(Assigned(miTool), '无法获取Tools菜单项');

  iIndex := miRoot.IndexOf(miTool) + 1;

  FETSMenu := AddMenu(miRoot, 'ETS', iIndex);
  FFreeNotification.FreeNotification(FETSMenu);

  AddMenu(FETSMenu, 'Debug/打印工程参数', -1, DoClickPrintProjectParameter);
  AddMenu(FETSMenu, 'Debug/打印代码窗口属性', -1, DoClickPrintFormProperty);
  AddMenu(FETSMenu, 'Debug/打印Structure数据', -1, DoShowStructureView);
end;

destructor TETSMainMenu.Destroy;
begin
  FreeAndNil(FFreeNotification);
  FreeAndNil(FETSMenu);

  inherited;
end;

procedure TETSMainMenu.DoClickPrintFormProperty(ASender: TObject);
var
  i: Integer;
  fm: TForm;
begin
  for i := 0 to Screen.FormCount - 1 do
  begin
    fm := Screen.Forms[i];
    if 0 = CompareText('TEditWindow', fm.ClassName) then
    begin
      WriteView('弹出菜单地址: (%s)%X', [fm.Name, Integer(fm.PopupMenu)]);
      Exit;
    end;
  end;
end;

procedure TETSMainMenu.DoClickPrintProjectParameter(ASender: TObject);
  function getValue(AOptions: IOTAOptions; AName: TOTAOptionName): String;
  var
    v: Variant;
  begin
    v := AOptions.GetOptionValue(AName.Name);

    case AName.Kind of
      tkInteger, tkLString, tkEnumeration: Result := VarToStr(v);
      tkSet: Result := '集合类型';
      tkClass: Result := TObject(VarToInt(v)).ClassName;
    else
      Result := '无法识别的类型';
    end;
  end;
const
  cTypeName: array[TTypeKind] of String = ('tkUnknown', 'tkInteger', 'tkChar',
    'tkEnumeration', 'tkFloat',
    'tkString', 'tkSet', 'tkClass', 'tkMethod', 'tkWChar', 'tkLString', 'tkWString',
    'tkVariant', 'tkArray', 'tkRecord', 'tkInterface', 'tkInt64', 'tkDynArray');
var
  i: Integer;
  prj: IOTAProject;
  opt: IOTAOptions;
  ona: TOTAOptionNameArray;
begin
  prj := GetActiveProject;
  if not Assigned(prj) then
    Exit;

  opt := prj.GetProjectOptions;
  ona := opt.GetOptionNames;
  WriteView('工程选项个数: %d', [Length(ona)]);
  for i := 0 to Length(ona) - 1 do
    WriteView('选项(%.3d) - 名称: %-30s 类型: %-16s 值: %s',
      [i, ona[i].Name, cTypeName[ona[i].Kind], getValue(opt, ona[i])]);
end;

procedure TETSMainMenu.DoShowStructureView(ASender: TObject);
  procedure printNode(ANode: IOTAStructureNode; ALevel: Integer);
  var
    i: Integer;
  begin
    WriteView(Format('%%%ds%%s', [ALevel * 4]), ['', ANode.Caption]);
    for i := 0 to ANode.ChildCount - 1 do
      printNode(ANode.Child[i], ALevel + 1);
  end;
var
  sv: IOTAStructureView;
  sc: IOTAStructureContext;
  i: Integer;
begin
  sv := BorlandIDEServices as IOTAStructureView;
  sc := sv.GetStructureContext;
  for i := 0 to sc.RootNodeCount - 1 do
    printNode(sc.GetRootStructureNode(i), 0);
end;

function TETSMainMenu.GetMainMenu: TMainMenu;
var
  ntas: INTAServices;
begin
  Result := nil;

  ntas := BorlandIDEServices as INTAServices;
  if not Assigned(ntas) then
    Exit;

  Result := ntas.MainMenu;
end;

procedure TETSMainMenu.Notification(AComponent: TComponent; AOperation: TOperation);
begin
  if opRemove <> AOperation then
    Exit;

  if AComponent = FETSMenu then
    FETSMenu := nil;
end;

{ TETSPasPopupMenu }

constructor TETSPasPopupMenu.Create;
begin
  FIDENotifier := TETSNotifierManager.GetManager.AddIDENotifier(DoIDENotifier);

  FFreeNotification := TFreeNotification.Create(nil);
  FFreeNotification.OnNotification := Notification;

  FNotifyEventHook := TNotifyEventHook.Create;
  FNotifyEventHook.OnAfterNotifyEvent := DoPopup;
end;

destructor TETSPasPopupMenu.Destroy;
begin
  if FIDENotifier >= 0 then
  begin
    TETSNotifierManager.GetManager.RemoveIDENotifier(FIDENotifier);
    FIDENotifier := -1;
  end;

  FreeAndNil(FFreeNotification);
  FreeAndNil(FFoldAll);

  FNotifyEventHook.UnHook;
  FreeAndNil(FNotifyEventHook);

  inherited;
end;

procedure TETSPasPopupMenu.DoClickFoldAll(ASender: TObject);
var
  miRoot, mi: TMenuItem;
begin
  miRoot := GetPopupMenu.Items;

  mi := FindMenu(miRoot, 'Fold/Types');
  if Assigned(mi) then
    mi.Click;

  mi := FindMenu(miRoot, 'Fold/Nested Methods');
  if Assigned(mi) then
    mi.Click;

  mi := FindMenu(miRoot, 'Fold/Methods');
  if Assigned(mi) then
    mi.Click;
end;

procedure TETSPasPopupMenu.DoIDENotifier(ANotifyCode: TOTAFileNotification;
  const AFileName: String);
var
  pm: TPopupMenu;
begin
  if (ANotifyCode <> ofnFileOpened)
    or (0 <> CompareText('.pas', ExtractFileExt(AFileName)))
    or FNotifyEventHook.Hooked then
    Exit;

  pm := GetPopupMenu;
  if not Assigned(pm) then
    Exit;

  FNotifyEventHook.Hook(pm, 'OnPopup');
end;

procedure TETSPasPopupMenu.DoPopup(ASender: TObject);
var
  miRoot: TMenuItem;
begin
  miRoot := TPopupMenu(ASender).Items;

  FFoldAll := FindMenu(miRoot, 'Fold/All');
  if Assigned(FFoldAll) then
    Exit;

  FFoldAll := AddMenu(miRoot, 'Fold/All', -1, DoClickFoldAll);
  FFreeNotification.FreeNotification(FFoldAll);
end;

function TETSPasPopupMenu.GetPopupMenu: TPopupMenu;
var
  i: Integer;
  fm: TForm;
begin
  Result := nil;

  for i := 0 to Screen.FormCount - 1 do
  begin
    fm := Screen.Forms[i];
    if 0 = CompareText('TEditWindow', fm.ClassName) then
    begin
      Result := fm.FindComponent('EditorLocalMenu') as TPopupMenu;
      Exit;
    end;
  end;
end;

procedure TETSPasPopupMenu.Notification(AComponent: TComponent; AOperation: TOperation);
begin
  if opRemove <> AOperation then
    Exit;

  if AComponent = FFoldAll then
      FFoldAll := nil;
end;

{ TETSMenuManager }

var
  GETSMainMenu: TETSMainMenu;
  GETSPasPopupMenu: TETSPasPopupMenu;

class function TETSMenuManager.GetManager: TETSMenuManager;
begin
  Result := FManager;
end;

class procedure TETSMenuManager.Init(const ABorlandIDEServices: IBorlandIDEServices;
  ARegisterProc: TWizardRegisterProc);
begin
  if not Assigned(FManager) then
  begin
    FManager := TETSMenuManager.Create;
    ARegisterProc(TETSMenuWizard.Create); //TETSMenuWizard仅用于演示目的

    GETSMainMenu := TETSMainMenu.Create;
    GETSPasPopupMenu := TETSPasPopupMenu.Create;
  end;
end;

class procedure TETSMenuManager.UnInit;
begin
  FreeAndNil(FManager);
  FreeAndNil(GETSMainMenu);
  FreeAndNil(GETSPasPopupMenu);
end;

initialization
  TETSMenuManager.FManager := nil;
  GETSMainMenu := nil;
  GETSPasPopupMenu := nil;

finalization
  TETSMenuManager.UnInit;

end.
