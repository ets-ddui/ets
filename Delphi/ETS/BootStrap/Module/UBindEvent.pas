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
    par.cArgs := iCount + 1; //FMethodData^.ParamCount不包含Self，因此加1

    SetLength(parArray, par.cArgs);
    par.rgvarg := @parArray[0];

    //这里的参数顺序与js中的相反
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

      //Delphi针对事件属性提供的类型信息相当有限(如果是开启了METHODINFO编译开关的函数，函数入参及调用协议信息会比较完整)，
      //无法正确判断入参类型及调用协议，这里按__pascal调用协议处理(入参按从左到右的顺序入栈，被调函数释放堆栈)
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
      else
      begin
        WriteView('(TBindEvent)无法识别的参数(%d)类型(%s)', [iRtti, strParamType]);
      end;
    end;

    FCallBack.Invoke(0, GUID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
      par, @vResult, nil, nil);

    //借用EAX保存入参长度，在Execute中用于调整堆栈平衡
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
//参数1：属性名
//参数0：js中回调函数的地址
//函数作用是为AObject对象的指定属性(回调事件)，绑定回调函数(回调函数在js中实现)
//注意：这里的入参顺序与js中的相反
function Bind(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray): OleVariant;
begin
  Result := False;
  if not (AObject is TComponent) then
    Exit;
  if (AParamCount <> 2) or (VarType(AParams[0]) <> varDispatch) then
    Exit;

  //创建的TBindEvent对象，在AObject析构时释放
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
