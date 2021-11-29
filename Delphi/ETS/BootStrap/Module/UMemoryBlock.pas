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
    //IMemoryBlock实现
    function GetEncoding(var AResult: TEncodingType): HRESULT; stdcall;
    function SetEncoding(AValue: TEncodingType): HRESULT; stdcall;
    function GetSize(var AResult: Cardinal): HRESULT; stdcall;
    function SetSize(AValue: Cardinal): HRESULT; stdcall;
    //APosition从0开始计算，和String类型的起始序号有差异
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
  {TODO: 添加编码集转换的代码}
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
