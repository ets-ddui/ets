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
unit UETSTreeGrid;

interface

uses
  UDUIGrid, UDUITreeGrid;

type
  {$METHODINFO ON}
  TETSTreeColumns = class(TDUITreeColumns)
  public
    function Add(ACaption: String): TDUITreeColumn;
    procedure Delete(AIndex: Integer);
    function GetItems(AIndex: Integer): TDUITreeColumn;
  published
    property Count;
  end;

  TETSTreeNode = class(TDUITreeNode)
  public
    function AddChild(ACaption: String; AFirst: Boolean): TDUITreeNode;
    procedure Clear;
    function GetCell(AColIndex: Integer): String;
    procedure SetCell(AColIndex: Integer; AValue: String);
    function GetItems(AIndex: Integer): TDUITreeNode;
  published
    property Caption;
    property ChildCount;
    property Collapsed;
    property Height;
    property Index;
    property Level;
    property Prior;
    property Next;
  end;

  TETSTreeGrid = class(TDUITreeGrid)
  protected
    procedure DoCreate(var AColumns: TDUITreeColumns; var ARootNode: TDUITreeNode); override;
  published
    property RootNode;
  end;
  {$METHODINFO OFF}

implementation

{ TETSTreeColumns }

function TETSTreeColumns.Add(ACaption: String): TDUITreeColumn;
begin
  Result := inherited Add(ACaption);
end;

procedure TETSTreeColumns.Delete(AIndex: Integer);
begin
  if (AIndex < 0) or (AIndex >= Count) then
    Exit;

  inherited Delete(AIndex);
end;

function TETSTreeColumns.GetItems(AIndex: Integer): TDUITreeColumn;
begin
  Result := Items[AIndex];
end;

{ TETSTreeNode }

function TETSTreeNode.AddChild(ACaption: String; AFirst: Boolean): TDUITreeNode;
begin
  Result := inherited AddChild(ACaption, AFirst);
end;

procedure TETSTreeNode.Clear;
begin
  inherited;
end;

function TETSTreeNode.GetCell(AColIndex: Integer): String;
var
  tg: TDUITreeGrid;
begin
  Result := '';
  tg := GetGrid;
  if not Assigned(tg) then
    Exit;

  if (AColIndex < 0) or (AColIndex >= tg.Columns.Count) then
    Exit;

  Result := tg.Cells[tg.Columns[AColIndex], Self];
end;

procedure TETSTreeNode.SetCell(AColIndex: Integer; AValue: String);
var
  tg: TDUITreeGrid;
begin
  tg := GetGrid;
  if not Assigned(tg) then
    Exit;

  if (AColIndex < 0) or (AColIndex >= tg.Columns.Count) then
    Exit;

  tg.Cells[tg.Columns[AColIndex], Self] := AValue;
end;

function TETSTreeNode.GetItems(AIndex: Integer): TDUITreeNode;
begin
  Result := Childs[AIndex];
end;

{ TETSTreeGrid }

procedure TETSTreeGrid.DoCreate(var AColumns: TDUITreeColumns; var ARootNode: TDUITreeNode);
begin
  Options := Options - [goHorzTitle];

  AColumns := TETSTreeColumns.Create(Self, TDUITreeColumn);
  AColumns.Add('');

  ARootNode := TETSTreeNode.Create(TETSTreeNode);
end;

end.
