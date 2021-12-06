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
unit UETSComboBox;

interface

uses
  UDUIComboBox;

type
  {$METHODINFO ON}
  TETSComboBox = class(TDUIComboBox)
  public
    function AddData(AValue: String): Integer;
    procedure Clear;
  end;
  {$METHODINFO OFF}

implementation

{ TETSComboBox }

function TETSComboBox.AddData(AValue: String): Integer;
begin
  Result := inherited AddData(AValue);
end;

procedure TETSComboBox.Clear;
begin
  inherited;
end;

end.
