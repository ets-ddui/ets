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
unit UMemoryBlock;

interface

uses
  Windows, UInterface;

type
  TMemoryBlock = class(TInterfacedBase, IMemoryBlock)
  private
    FBuffer: String;
    FEncoding: TEncodingType;
  public
    constructor Create(AValue: String; AEncoding: TEncodingType); reintroduce; virtual;
  public
    //IMemoryBlockʵ��
    function GetEncoding(var AResult: TEncodingType): HRESULT; stdcall;
    function SetEncoding(AValue: TEncodingType): HRESULT; stdcall;
    function GetSize(var AResult: Cardinal): HRESULT; stdcall;
    function SetSize(AValue: Cardinal): HRESULT; stdcall;
    //APosition��0��ʼ���㣬��String���͵���ʼ����в���
    function Read(APosition: Cardinal; var AValue: Byte; var ALength: Cardinal): HRESULT; stdcall;
    function Write(APosition: Cardinal; var AValue: Byte; ALength: Cardinal): HRESULT; stdcall;
  end;

implementation

{ TMemoryBlock }

constructor TMemoryBlock.Create(AValue: String; AEncoding: TEncodingType);
begin
  FBuffer := AValue;
  FEncoding := AEncoding;
end;

function TMemoryBlock.GetEncoding(var AResult: TEncodingType): HRESULT;
begin
  AResult := FEncoding;

  Result := S_OK;
end;

function TMemoryBlock.GetSize(var AResult: Cardinal): HRESULT;
begin
  AResult := Length(FBuffer);

  Result := S_OK;
end;

function TMemoryBlock.Read(APosition: Cardinal; var AValue: Byte; var ALength: Cardinal): HRESULT;
begin
  if Integer(APosition) >= Length(FBuffer) then
  begin
    Result := E_INVALIDARG;
    Exit;
  end;

  if Integer(APosition + ALength) >= Length(FBuffer) then
    ALength := Cardinal(Length(FBuffer)) - APosition;

  Move(FBuffer[APosition + 1], AValue, ALength);

  Result := S_OK;
end;

function TMemoryBlock.SetEncoding(AValue: TEncodingType): HRESULT;
begin
  {TODO: ��ӱ��뼯ת���Ĵ���}
  FEncoding := AValue;

  Result := S_OK;
end;

function TMemoryBlock.SetSize(AValue: Cardinal): HRESULT;
begin
  SetLength(FBuffer, AValue);

  Result := S_OK;
end;

function TMemoryBlock.Write(APosition: Cardinal; var AValue: Byte; ALength: Cardinal): HRESULT;
begin
  if Integer(APosition + ALength) >= Length(FBuffer) then
  begin
    Result := E_INVALIDARG;
    Exit;
  end;

  Move(AValue, FBuffer[APosition + 1], ALength);

  Result := S_OK;
end;

end.
