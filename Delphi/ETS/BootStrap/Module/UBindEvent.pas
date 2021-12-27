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
unit UBindEvent;

interface

uses
  Windows, Classes, SysUtils, Variants, TypInfo, UDispatchWrapper;

type
  PContext = ^TContext;
  TContext = packed record
    EDI: DWORD;
    ESI: DWORD;
    EBP: DWORD;
    ESP: DWORD;
    EBX: DWORD;
    EDX: DWORD;
    ECX: DWORD;
    EAX: DWORD;
  end;

  TBindEvent = class(TComponent)
  private
    FMethodData: PTypeData;
    FCallBack: IDispatch;
    procedure DoExecute(AContext: PContext);
    procedure Execute;
    function GetParam(AContext: PContext; AIndex, AMax: DWORD): DWORD;
    function GetValid: Boolean;
  public
    constructor Create(AOwner: TComponent; APropertyName: String; ACallBack: IDispatch); reintroduce;
    property Valid: Boolean read GetValid;
  end;

implementation

uses
  Types, ActiveX, UModuleBase, UTool;

type
  TCache = class
  private
    FObjects: TStringList;

    type
      TDispatch = class
        FValue: IDispatch;
      end;
      TWideString = class
        FValue: WideString;
      end;
  public
    constructor Create;
    destructor Destroy; override;
    procedure Assign(var ADest: TVariantArg; AValue: TObject); overload;
    procedure Assign(var ADest: TVariantArg; AValue: String); overload;
  end;

{ TCache }

procedure TCache.Assign(var ADest: TVariantArg; AValue: TObject);
var
  obj: TDispatch;
begin
  obj := TDispatch.Create;
  obj.FValue := WrapperObject(AValue, False);
  ADest.dispVal := Pointer(obj.FValue);

  FObjects.AddObject('', obj);
end;

procedure TCache.Assign(var ADest: TVariantArg; AValue: String);
var
  obj: TWideString;
begin
  obj := TWideString.Create;
  obj.FValue := AValue;
  ADest.bstrVal := @obj.FValue[1];

  FObjects.AddObject('', obj);
end;

constructor TCache.Create;
begin
  FObjects := TStringList.Create;
end;

destructor TCache.Destroy;
var
  i: Integer;
begin
  for i := 0 to FObjects.Count - 1 do
    FObjects.Objects[i].Free;

  FreeAndNil(FObjects);

  inherited;
end;

{ TBindEvent }

constructor TBindEvent.Create(AOwner: TComponent; APropertyName: String; ACallBack: IDispatch);
type
  TEvent = procedure of object;
var
  pi: PPropInfo;
  me: TMethod;
  ev: TEvent;
begin
  inherited Create(AOwner);

  FCallBack := ACallBack;

  FMethodData := nil;
  pi := GetPropInfo(AOwner, APropertyName);
  if pi^.PropType^.Kind = tkMethod then
  begin
    ev := Execute;
    me := TMethod(ev);
    SetMethodProp(AOwner, pi, me);
    FMethodData := GetTypeData(pi^.PropType^);
  end;
end;

procedure TBindEvent.DoExecute(AContext: PContext);
var
  par: DispParams;
  vResult: Variant;
  parArray: array of TVariantArg;
  iRtti, iJs, iCount: Integer;
  pParamList: Pointer;
  strParamType: String;
  cache: TCache;
begin
  cache := TCache.Create;
  try
    FillChar(par, SizeOf(par), 0);

    iCount := FMethodData^.ParamCount;
    par.cArgs := iCount + 1; //FMethodData^.ParamCount������Self����˼�1

    SetLength(parArray, par.cArgs);
    par.rgvarg := @parArray[0];

    //����Ĳ���˳����js�е��෴
    iJs := iCount;
    parArray[iJs].vt := VT_DISPATCH;
    cache.Assign(parArray[iJs], Owner);

    pParamList := @FMethodData^.ParamList;
    for iRtti := 1 to iCount do
    begin
      Dec(iJs);

      pParamList := Pointer(DWORD(pParamList) + SizeOf(TParamFlags));
      pParamList := Pointer(DWORD(pParamList) + PByte(pParamList)^ + 1);
      strParamType := PShortString(pParamList)^;
      pParamList := Pointer(DWORD(pParamList) + PByte(pParamList)^ + 1);

      //Delphi����¼������ṩ��������Ϣ�൱����(����ǿ�����METHODINFO���뿪�صĺ�����������μ�����Э����Ϣ��Ƚ�����)��
      //�޷���ȷ�ж�������ͼ�����Э�飬���ﰴ__pascal����Э�鴦��(��ΰ������ҵ�˳����ջ�����������ͷŶ�ջ)
      if 0 = CompareText(strParamType, 'Integer') then
      begin
        parArray[iJs].vt := VT_INT;
        parArray[iJs].intVal := GetParam(AContext, iRtti, iCount);
      end
      else if 0 = CompareText(strParamType, 'String') then
      begin
        parArray[iJs].vt := VT_BSTR;
        cache.Assign(parArray[iJs], String(GetParam(AContext, iRtti, iCount)));
      end
      else if 0 = CompareText(strParamType, 'WideString') then
      begin
        parArray[iJs].vt := VT_BSTR;
        parArray[iJs].bstrVal := PWideChar(GetParam(AContext, iRtti, iCount));
      end
      else
      begin
        WriteView('(TBindEvent)�޷�ʶ��Ĳ���(%d)����(%s)', [iRtti, strParamType]);
      end;
    end;

    FCallBack.Invoke(0, GUID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
      par, @vResult, nil, nil);

    //����EAX������γ��ȣ���Execute�����ڵ�����ջƽ��
    if iCount >= 3 then
    begin
      AContext.EAX := (iCount - 2) * 4;
      PDWORD(DWORD(AContext) + SizeOf(TContext) + AContext.EAX)^
        := PDWORD(DWORD(AContext) + SizeOf(TContext))^;
    end
    else
      AContext.EAX := 0;
  finally
    FreeAndNil(cache);
  end;
end;

procedure TBindEvent.Execute;
asm
  pushad
  mov edx, esp
  call DoExecute
  popad
  add esp, eax
  ret
end;

function TBindEvent.GetParam(AContext: PContext; AIndex, AMax: DWORD): DWORD;
begin
  case AIndex of
    //0: Result := AContext.EAX;
    1: Result := AContext.EDX;
    2: Result := AContext.ECX;
  else
    Result := PDWORD(AContext.ESP + 4 + 4 * (AMax - AIndex))^;
  end;
end;

function TBindEvent.GetValid: Boolean;
begin
  Result := Assigned(FMethodData);
end;

//Bind
//����1��������
//����0��js�лص������ĵ�ַ
//����������ΪAObject�����ָ������(�ص��¼�)���󶨻ص�����(�ص�������js��ʵ��)
//ע�⣺��������˳����js�е��෴
function Bind(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray): OleVariant;
begin
  Result := False;
  if not (AObject is TComponent) then
    Exit;
  if (AParamCount <> 2) or (VarType(AParams[0]) <> varDispatch) then
    Exit;

  //������TBindEvent������AObject����ʱ�ͷ�
  with TBindEvent.Create(TComponent(AObject), VarToStr(AParams[1]), IUnknown(AParams[0]) as IDispatch) do
  begin
    if not Valid then
    begin
      Free;
      Exit;
    end;
  end;

  Result := True;
end;

initialization
  TDispatchWrapper.RegCustomDispatch(TComponent, 'Bind', Bind);

end.
