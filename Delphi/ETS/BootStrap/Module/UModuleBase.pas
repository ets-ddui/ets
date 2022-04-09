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

  FDispatch := TDispatchWrapper.Create(Self, False, IInterfaceNoRefCount(IInterface(Self))); //��Selfǿת��IInterface�����ᵼ�����ü����ı仯
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
