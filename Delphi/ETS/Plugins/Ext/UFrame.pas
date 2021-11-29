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
unit UFrame;

{$i UConfigure.inc}

interface

uses
  Windows, Classes, UInterface, UFrameBase;

function GetFrame(AManager: IManager; AID: String; AOwner: TComponent; var AResult: IChild): HRESULT; stdcall;

implementation

function GetFrame(AManager: IManager; AID: String; AOwner: TComponent; var AResult: IChild): HRESULT;
begin
  SetManager(AManager);
  AResult := TFrameBase.GetFrame(AID, AOwner);

  if Assigned(AResult) then
    Result := S_OK
  else
    Result := E_OUTOFMEMORY;
end;

end.
