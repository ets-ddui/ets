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
unit USkinEditor;

interface

uses
  Classes, SysUtils, StrUtils, Controls, Dialogs, IGDIPlus, GDIHelper,
  UDUICore, UDUIForm, UFrameBase, UInterface, UDUIGrid, UDUITreeGrid,
  UDUISkin, UDUIUtils;

type
  THelperType = (htBrush, htColor, htFont, htImage, htPen);

  ISkinEditor = interface
    procedure Init(AParent: TFrameBase);
    procedure UnInit;
    procedure OnDropFile(AFileName: String; AIndex, ACount: Integer);
  end;

  TFrmSkinEditor = class(TFrameBase)
    TgConfig: TDUITreeGrid;
    procedure FrameBaseAfterNotify(ANotifyType: TNotifyType);
    function TgConfigGetPaintControls(ASender: TObject; const ACol, ARow: TDUIRowColID): TControlsList;
    procedure TgConfigSelectCell(ASender: TObject; const ACol, ARow: TDUIRowColID; AX, AY: Integer);
    procedure FrameBaseDropFiles(AFileName: String; AIndex, ACount: Integer);
  private
    FSelectedType: THelperType;
    FSelectedNode: TDUITreeNode;
    FPaintControls: TControlsList;
    FSkin: IRawSkin;
    FCurrentEditor: ISkinEditor;
    FEditor: array[THelperType] of ISkinEditor;
    function GetHelper: TBaseHelper;
  public
    destructor Destroy; override;
    procedure Commit;
    procedure Cancel;
    procedure ChangeHelperType(AClass: TBaseHelperClass);
    property Helper: TBaseHelper read GetHelper;
  end;

implementation

uses
  qjson, UDUICoordinateTransform,
  USkinImage;

{$R *.dfm}

type
  TData = class
  strict private
    FHelper: array[THelperType] of TBaseHelper;
    FSkin: TQJson;
    FPath: String;
  private
    function GetHelper(AIndex: THelperType): TBaseHelper;
    procedure SetHelper(AIndex: THelperType; const AValue: TBaseHelper);
  public
    constructor Create(AJson: TQJson);
    destructor Destroy; override;
    procedure Commit(AType: THelperType);
    procedure Cancel(AType: THelperType);
    property Helper[AIndex: THelperType]: TBaseHelper read GetHelper write SetHelper;
  end;

  TShowControl = class(TDUIBase)
  private
    class var
      FBrush: IGPSolidBrush;
      FColor: TGPColor;
      FFocus: IGPPen;
    class procedure Init;
  private
    FHelperType: THelperType;
    FHelper: TBaseHelper;
    FSelected: Boolean;
    procedure DoPaint(AGPCanvas: IGPGraphics); override;
    function IsTransparent: Boolean; override;
  public
    procedure SetHelper(AHelperType: THelperType; AHelper: TBaseHelper; ASelected: Boolean);
  end;

const
  GHelperName: array[THelperType] of String = (
    '__brush__', '__color__', '__font__', '__image__', '__pen__');

{ TData }

constructor TData.Create(AJson: TQJson);
begin
  FSkin := AJson.Root;
  FPath := AJson.Path;
end;

destructor TData.Destroy;
var
  ht: THelperType;
begin
  for ht := Low(THelperType) to High(THelperType) do
    FreeAndNil(FHelper[ht]);

  inherited;
end;

procedure TData.Commit(AType: THelperType);
var
  js, jsHelper: TQJson;
begin
  js := FSkin.ItemByPath(FPath);
  js.Delete(GHelperName[AType]);
  jsHelper := js.Add(GHelperName[AType], jdtObject);
  ComponentToJson(jsHelper, FHelper[AType]);
end;

procedure TData.Cancel(AType: THelperType);
var
  js, jsHelper: TQJson;
begin
  FreeAndNil(FHelper[AType]);

  js := FSkin.ItemByPath(FPath);
  if js.HasChild(GHelperName[AType], jsHelper) then
    JsonToComponent(TObject(FHelper[AType]), jsHelper, nil);
end;

function TData.GetHelper(AIndex: THelperType): TBaseHelper;
begin
  Result := FHelper[AIndex];
end;

procedure TData.SetHelper(AIndex: THelperType; const AValue: TBaseHelper);
begin
  FreeAndNil(FHelper[AIndex]);
  FHelper[AIndex] := AValue;
end;

{ TShowControl }

class procedure TShowControl.Init;
begin
  FColor := GetSkin.GetColor('TEXT');
  FBrush := TGPSolidBrush.Create(FColor);
  FFocus := TGPPen.Create(FColor, 2);
end;

procedure TShowControl.DoPaint(AGPCanvas: IGPGraphics);
const
  CText: String = '强';
  CIndent: Integer = 2;
var
  si: TGPSizeF;
begin
  if FSelected then
    AGPCanvas.DrawRectangle(FFocus, MakeRect(0, 0, Width, Height));

  if not Assigned(FHelper) then
    Exit;

  case FHelperType of
    htBrush:
    begin
      AGPCanvas.FillRectangle(TBrushHelper(FHelper).Brush,
        MakeRect(CIndent, CIndent, Width - 2 * CIndent, Height - 2 * CIndent));
    end;
    htColor:
    begin
      FBrush.Color := TColorHelper(FHelper).Color;
      AGPCanvas.FillRectangle(FBrush,
        MakeRect(CIndent, CIndent, Width - 2 * CIndent, Height - 2 * CIndent));
    end;
    htFont:
    begin
      FBrush.Color := FColor;
      si := AGPCanvas.MeasureStringF(CText, TFontHelper(FHelper).Font);
      AGPCanvas.DrawStringF(CText, TFontHelper(FHelper).Font,
        MakePointF((Width - 2 * CIndent - si.Width) / 2, (Height - 2 * CIndent - si.Height) / 2), FBrush);
    end;
    htImage:
    begin
      if Assigned(TImageHelper(FHelper).Image) then
        AGPCanvas.DrawImage(TImageHelper(FHelper).Image,
          MakeRect(CIndent, CIndent, Width - 2 * CIndent, Height - 2 * CIndent));
    end;
    htPen:
    begin
      AGPCanvas.DrawPolygon(TPenHelper(FHelper).Pen,
        MakePoints(MakeDUICoords([CIndent, CIndent,
          Width - CIndent, Height - CIndent,
          Width - CIndent, CIndent,
          CIndent, Height - CIndent])));
    end;
  end;
end;

function TShowControl.IsTransparent: Boolean;
begin
  Result := True;
end;

procedure TShowControl.SetHelper(AHelperType: THelperType; AHelper: TBaseHelper; ASelected: Boolean);
begin
  FHelperType := AHelperType;
  FHelper := AHelper;
  FSelected := ASelected;
end;

{ TFrmSkinEditor }

destructor TFrmSkinEditor.Destroy;
var
  i: Integer;
  ht: THelperType;
begin
  for i := FPaintControls.Count - 1 downto 0 do
    TDUIBase(FPaintControls[i]).Free;
  FreeAndNil(FPaintControls);

  for ht := Low(THelperType) to High(THelperType) do
    FEditor[ht] := nil;
  FCurrentEditor := nil;

  inherited;
end;

procedure TFrmSkinEditor.ChangeHelperType(AClass: TBaseHelperClass);
begin
  if not Assigned(Helper) or (Helper.ClassType <> AClass) then
    TData(FSelectedNode.ObjectData).Helper[FSelectedType] := AClass.Create(Self);
end;

procedure TFrmSkinEditor.Commit;
begin
  TData(FSelectedNode.ObjectData).Commit(FSelectedType);
  FSkin.SaveData;
end;

procedure TFrmSkinEditor.Cancel;
begin
  TData(FSelectedNode.ObjectData).Cancel(FSelectedType);
end;

function TFrmSkinEditor.GetHelper: TBaseHelper;
begin
  Result := TData(FSelectedNode.ObjectData).Helper[FSelectedType];
end;

procedure TFrmSkinEditor.FrameBaseAfterNotify(ANotifyType: TNotifyType);
  function isHelperNode(var AType: THelperType; AName: String): Boolean;
  var
    ht: THelperType;
  begin
    Result := True;

    for ht := Low(THelperType) to High(THelperType) do
      if CompareText(AName, GHelperName[ht]) = 0 then
      begin
        AType := ht;
        Exit;
      end;

    Result := False;
  end;
  procedure initTree(ANode: TDUITreeNode; const ASkin: TQJson);
  var
    i: Integer;
    js: TQJson;
    nd: TDUITreeNode;
    ht: THelperType;
    obj: TBaseHelper;
  begin
    for i := 0 to ASkin.Count - 1 do
    begin
      js := ASkin[i];

      if isHelperNode(ht, js.Name) then
      begin
        obj := nil;
        if JsonToComponent(TObject(obj), js, nil) then
          TData(ANode.ObjectData).Helper[ht] := obj;

        Continue;
      end;

      if ASkin.IsArray then
        nd := ANode.AddChild(Format('[%d]', [i]))
      else
        nd := ANode.AddChild(js.Name);
      nd.ObjectData := TData.Create(js); //TDUITreeGrid会接管TData的所有权，因此，无需主动释放
      initTree(nd, js);
    end;
  end;
begin
  if (ANotifyType <> ntActive) then
    Exit;

  if Assigned(FSkin) then
    Exit;

  if not Supports(GetSkin, IRawSkin, FSkin) then
    Exit;

  FEditor[htBrush] := nil;
  FEditor[htColor] := nil;
  FEditor[htFont] := nil;
  FEditor[htImage] := TFrmSkinImage.Create(Self);
  FEditor[htPen] := nil;

  TgConfig.RootNode.Clear;
  initTree(TgConfig.RootNode, FSkin.GetRootNode);
end;

procedure TFrmSkinEditor.FrameBaseDropFiles(AFileName: String; AIndex, ACount: Integer);
begin
  if Assigned(FCurrentEditor) then
    FCurrentEditor.OnDropFile(AFileName, AIndex, ACount);
end;

function TFrmSkinEditor.TgConfigGetPaintControls(ASender: TObject; const ACol, ARow: TDUIRowColID): TControlsList;
var
  ht: THelperType;
begin
  Result := nil;
  if ACol.FIndex <> TgConfig.Columns[1] then
    Exit;

  if not Assigned(FPaintControls) then
  begin
    FPaintControls := TControlsList.Create;
    for ht := Low(THelperType) to High(THelperType) do
      FPaintControls.Add(TShowControl.Create(Self));
  end;

  for ht := Low(THelperType) to High(THelperType) do
    with TShowControl(FPaintControls[Ord(ht)]) do
    begin
      Width := TgConfig.Columns[1].Width div (Ord(High(THelperType)) + 1);
      Align := alLeft;
      SetHelper(ht, TData(TDUITreeNode(ARow.FIndex).ObjectData).Helper[ht],
        (FSelectedNode = ARow.FIndex) and (FSelectedType = ht));
    end;

  Result := FPaintControls;
end;

procedure TFrmSkinEditor.TgConfigSelectCell(ASender: TObject; const ACol, ARow: TDUIRowColID; AX, AY: Integer);
var
  iWidth, iIndex: Integer;
begin
  if ACol.FIndex <> TgConfig.Columns[1] then
    Exit;

  iWidth := TgConfig.Columns[1].Width div (Ord(High(THelperType)) + 1);
  iIndex := AX div iWidth;
  if (iIndex >= Ord(Low(THelperType))) and (iIndex <= Ord(High(THelperType))) then
  begin
    if (FSelectedNode = ARow.FIndex) and (FSelectedType = THelperType(iIndex)) then
      Exit;

    FSelectedNode := ARow.FIndex;
    FSelectedType := THelperType(iIndex);
    TgConfig.Invalidate;

    if Assigned(FCurrentEditor) then
      FCurrentEditor.UnInit;

    FCurrentEditor := FEditor[FSelectedType];

    if Assigned(FCurrentEditor) then
      FCurrentEditor.Init(Self);
  end;
end;

initialization
  TShowControl.Init;
  TFrameBase.RegFrame(TFrmSkinEditor);

end.
