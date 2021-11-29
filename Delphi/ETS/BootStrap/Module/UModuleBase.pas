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
unit UModuleBase;

interface

uses
  Classes, UInterface, UDispatchWrapper;

type
  TModuleBase = class(TInterfacedBase, IDispatch)
  private
    FDispatch: TDispatchWrapper;
    function GetDispatch: IDispatch;
  protected
    property DispatchImpl: IDispatch read GetDispatch implements IDispatch;
  public
    constructor Create; reintroduce; virtual;
    destructor Destroy; override;
  end;

function WrapperObject(AObject: TObject; AOwned: Boolean = True): IDispatch;

implementation

uses
  Windows, SysUtils, UTool;

{ TModuleBase }

constructor TModuleBase.Create;
begin
  inherited Create(nil);

  FDispatch := TDispatchWrapper.Create(Self, False, IInterfaceNoRefCount(IInterface(Self))); //将Self强转成IInterface，不会导致引用计数的变化
end;

destructor TModuleBase.Destroy;
begin
  FreeAndNil(FDispatch);

  inherited;
end;

function TModuleBase.GetDispatch: IDispatch;
begin
  Result := FDispatch;
end;

function WrapperObject(AObject: TObject; AOwned: Boolean): IDispatch;
begin
  Result := TDispatchWrapper.Create(AObject, AOwned) as IDispatch;
end;

end.
