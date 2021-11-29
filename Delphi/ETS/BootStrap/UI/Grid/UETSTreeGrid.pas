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
