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
unit UDispatchWrapper;

{$i UConfigure.inc}

interface

uses
  Windows, Classes, SysUtils, ObjComAuto, SyncObjs, TypInfo, ObjAuto, ActiveX, UInterface;

type
  PVariantArray = ^TVariantArray;
  TVariantArray = array[0..65535] of Variant;
  TDispCallback = function (AObject: TObject; AName: String; AFlag: Word;
    AParamCount: Integer; const AParams: PVariantArray): OleVariant;
  //TDispatchWrapper从ObjComAuto.TObjectDispatch移植而来
  TDispatchWrapper = class(TInterfacedBase, IDispatch)
  private
    class var
      FObjectList: TStringList;
      FLock: TCriticalSection;
      FCustomDispatch: TStringList;
    class procedure Init;
    class procedure UnInit;
    class procedure Add(ASelf: TDispatchWrapper);
    class procedure Delete(ASelf: TDispatchWrapper);
  public
    class procedure RegCustomDispatch(AClass: TClass; AName: String; ADispCallback: TDispCallback);
  private
    FDispatchInfoCount: Integer;
    FDispatchInfos: TDispatchInfos;
    FInstance: TObject;
    FOwned: Boolean;
    function GetMethodInfo(const AName: ShortString; var AInstance: TObject): PMethodInfoHeader;
    function GetPropInfo(const AName: string; var AInstance: TObject; var CompIndex: Integer): PPropInfo;
  protected
    function GetObjectDispatchEx(AObj: TObject): TDispatchWrapper;
  public
    constructor Create(AInstance: TObject; AOwned: Boolean = True; AParent: IInterfaceNoRefCount = nil); reintroduce;
    destructor Destroy; override;
  public
    {IDispatch实现}
    function GetIDsOfNames(const IID: TGUID; Names: Pointer;
      NameCount: Integer; LocaleID: Integer; DispIDs: Pointer): HRESULT; stdcall;
    function Invoke(ADispID: Integer; const AIID: TGUID; ALocaleID: Integer;
      AFlags: Word; var AParams; AVarResult: Pointer; AExcepInfo: Pointer;
      AArgErr: Pointer): HRESULT; stdcall;
    function GetTypeInfo(AIndex: Integer; ALocaleID: Integer; out ATypeInfo): HRESULT; stdcall;
    function GetTypeInfoCount(out ACount: Integer): HRESULT; stdcall;
  end;

implementation

uses
  UTool;

const
  ofDispIDOffset = 100;
  ofCustomDispIDOffset = $40000000;

type
  TDispCallbackImpl = class
  strict private
    FClass: TClass;
    FName: String;
    FDispCallback: TDispCallback;
  public
    constructor Create(AClass: TClass; AName: String; ADispCallback: TDispCallback); reintroduce;
    function Call(AObject: TObject; AFlag: Word; const AParams: TDispParams): OleVariant;
    function IsMatch(AObject: TObject; AName: String): Boolean;
    function Replace(AClass: TClass; AName: String; ADispCallback: TDispCallback): Boolean;
  end;

{ TDispCallbackImpl }

constructor TDispCallbackImpl.Create(AClass: TClass; AName: String; ADispCallback: TDispCallback);
begin
  FClass := AClass;
  FName := AName;
  FDispCallback := ADispCallback;
end;

function TDispCallbackImpl.IsMatch(AObject: TObject; AName: String): Boolean;
begin
  Result := AObject.InheritsFrom(FClass) and (CompareText(FName, AName) = 0);
end;

function TDispCallbackImpl.Replace(AClass: TClass; AName: String; ADispCallback: TDispCallback): Boolean;
begin
  if (FClass = AClass) and (CompareText(FName, AName) = 0) then
  begin
    FDispCallback := ADispCallback;
    Result := True;
  end
  else
    Result := False;
end;

function TDispCallbackImpl.Call(AObject: TObject; AFlag: Word; const AParams: TDispParams): OleVariant;
begin
  Result := FDispCallback(AObject, FName, AFlag, AParams.cArgs, PVariantArray(AParams.rgvarg));
end;

{ TDispatchWrapper }

constructor TDispatchWrapper.Create(AInstance: TObject; AOwned: Boolean; AParent: IInterfaceNoRefCount);
begin
  inherited Create(AParent);

  FInstance := AInstance;
  FOwned := AOwned;

  Add(Self);
end;

destructor TDispatchWrapper.Destroy;
begin
  Delete(Self);
  if FOwned then
    FreeAndNil(FInstance);

  inherited;
end;

function TDispatchWrapper.GetObjectDispatchEx(AObj: TObject): TDispatchWrapper;
begin
  Result := TDispatchWrapper.Create(AObj, False);
end;

function TDispatchWrapper.GetIDsOfNames(const IID: TGUID; Names: Pointer;
  NameCount, LocaleID: Integer; DispIDs: Pointer): HRESULT; stdcall;
  function AllocDispID(AKind: TDispatchKind; Value: Pointer; AInstance: TObject): TDispID;
  var
    I: Integer;
  begin
    for I := FDispatchInfoCount - 1 downto 0 do
      with FDispatchInfos[I] do
        if (Kind = AKind) and (MethodInfo = Value) then
        begin
          // Already have a dispid for this methodinfo
          Result := ofDispIDOffset + I;
          Exit;
        end;
    if FDispatchInfoCount = Length(FDispatchInfos) then
      SetLength(FDispatchInfos, Length(FDispatchInfos) + 10);
    Result := ofDispIDOffset + FDispatchInfoCount;
    with FDispatchInfos[FDispatchInfoCount] do
    begin
      Instance := AInstance;
      Kind := AKind;
      MethodInfo := Value;
    end;
    Inc(FDispatchInfoCount);
  end;
  procedure WideCharToShortString(P: PWideChar; var S: ShortString);
  var
    I: Integer;
    W: WideChar;
  begin
    I := 0;
    repeat
      W := P[I];
      if W = #0 then Break;
      if W >= #256 then W := #0;
      Inc(I);
      S[I] := Char(W);
    until I = 255;
    S[0] := Char(I);
  end;
type
  PNames = ^TNames;
  TNames = array[0..100] of POleStr;
  PDispIDs = ^TDispIDs;
  TDispIDs = array[0..100] of Cardinal;
var
  Name: ShortString;
  Info: PMethodInfoHeader;
  PropInfo: PPropInfo;
  InfoEnd: Pointer;
  Params, Param: PParamInfo;
  I: Integer;
  ID: Cardinal;
  CompIndex: Integer;
  objInstance: TObject;
begin
  Result := S_OK;

  WideCharToShortString(PNames(Names)^[0], Name);
  Info := GetMethodInfo(Name, objInstance);
  FillChar(DispIDs^, SizeOf(PDispIds(DispIDs^)[0]) * NameCount, $FF);
  if Info = nil then
  begin
    // Not a  method, try a property.
    PropInfo := GetPropInfo(Name, objInstance, CompIndex);
    if PropInfo <> nil then
      PDispIds(DispIds)^[0] := AllocDispID(dkProperty, PropInfo, objInstance)
    else if CompIndex > -1 then
      PDispIds(DispIds)^[0] := AllocDispID(dkSubComponent, Pointer(CompIndex), objInstance)
    else
    begin
      Result := DISP_E_UNKNOWNNAME;

      FLock.Enter;
      try
        for I := 0 to FCustomDispatch.Count - 1 do
          with TDispCallbackImpl(FCustomDispatch.Objects[I]) do
          begin
            if IsMatch(FInstance, Name) then
            begin
              PDispIds(DispIDs)^[0] := ofCustomDispIDOffset + I;
              Result := S_OK;
              Exit;
            end;
          end;
      finally
        FLock.Leave;
      end;
    end;
  end
  else
  begin
    // Ensure the method information has enough type information
    if Info.Len <= SizeOf(Info^) - SizeOf(ShortString) + 1 + Length(Info.Name) then
      Result := DISP_E_UNKNOWNNAME
    else
    begin
      PDispIds(DispIds)^[0] := AllocDispID(dkMethod, Info, objInstance);
      Result := S_OK;
      if NameCount > 1 then
      begin
        // Now find the parameters. The DISPID is assumed to be the parameter
        // index.
        InfoEnd := Pointer(DWORD(Info) + Info^.Len);
        Params := PParamInfo(DWORD(Info) + SizeOf(Info^) - SizeOf(ShortString) + 1
          + SizeOf(TReturnInfo) + Length(Info^.Name));
        for I := 1 to NameCount - 1 do
        begin
          WideCharToShortString(PNames(Names)^[I], Name);
          Param := Params;
          ID := 0;
          while DWORD(Param) < DWORD(InfoEnd) do
          begin
            // ignore Self
            if (Param^.ParamType^.Kind <> tkClass) or not SameText(Param^.Name, 'SELF') then
              if SameText(Param.Name, Name) then
              begin
                PDispIDs(DispIDs)^[I] := ID;
                Break;
              end;
            Inc(ID);
            Param := PParamInfo(DWORD(Param) + SizeOf(Param^) -
              SizeOf(ShortString) + 1 + Length(Param^.Name));
          end;
          if DWORD(Param) >= DWORD(InfoEnd) then
            Result := DISP_E_UNKNOWNNAME
        end;
      end;
    end;
  end;
end;

function TDispatchWrapper.Invoke(ADispID: Integer; const AIID: TGUID;
  ALocaleID: Integer; AFlags: Word; var AParams; AVarResult, AExcepInfo,
  AArgErr: Pointer): HRESULT; stdcall;
type
  PIntegerArray = ^TIntegerArray;
  TIntegerArray = array[0..65535] of Integer;
var
  Parms: PDispParams;
  TempRet: Variant;
  DispatchInfo: TDispatchInfo;
  ReturnInfo: PReturnInfo;
begin
  Result := S_OK;

  Parms := @AParams;
  try
    if AVarResult = nil then
      AVarResult := @TempRet;
    if (ADispID - ofDispIDOffset >= 0) and (ADispID - ofDispIDOffset < FDispatchInfoCount) then
    begin
      DispatchInfo := FDispatchInfos[ADispID - ofDispIDOffset];
      case DispatchInfo.Kind of
        dkProperty:
          begin
            if AFlags and (DISPATCH_PROPERTYPUTREF or DISPATCH_PROPERTYPUT) <> 0 then
              if (Parms.cNamedArgs <> 1) or
                (PIntegerArray(Parms.rgdispidNamedArgs)^[0] <> DISPID_PROPERTYPUT) then
                Result := DISP_E_MEMBERNOTFOUND
              else
                SetPropValue(DispatchInfo.Instance, DispatchInfo.PropInfo,
                  PVariantArray(Parms.rgvarg)^[0])
            else
              if Parms.cArgs <> 0 then
                Result := DISP_E_BADPARAMCOUNT
              else if DispatchInfo.PropInfo^.PropType^.Kind = tkClass then
                POleVariant(AVarResult)^ := GetObjectDispatchEx(
                  TObject(Pointer(GetOrdProp(DispatchInfo.Instance, DispatchInfo.PropInfo)))) as IDispatch
              else
                POleVariant(AVarResult)^ := GetPropValue(DispatchInfo.Instance,
                  DispatchInfo.PropInfo, False);
          end;
        dkMethod:
          begin
            ReturnInfo := PReturnInfo(DWORD(DispatchInfo.MethodInfo) + SizeOf(TMethodInfoHeader) - SizeOf(ShortString) + 1 +
              Length(DispatchInfo.MethodInfo.Name));
            if (ReturnInfo.ReturnType <> nil) and (ReturnInfo.ReturnType^.Kind = tkClass) then
              POleVariant(AVarResult)^ := GetObjectDispatchEx(TObject(Integer(ObjectInvoke(DispatchInfo.Instance,
                DispatchInfo.MethodInfo,
                Slice(PIntegerArray(Parms.rgdispidNamedArgs)^, Parms.cNamedArgs),
                Slice(PVariantArray(Parms.rgvarg)^, Parms.cArgs))))) as IDispatch
            else
              POleVariant(AVarResult)^ := ObjectInvoke(DispatchInfo.Instance,
                DispatchInfo.MethodInfo,
                Slice(PIntegerArray(Parms.rgdispidNamedArgs)^, Parms.cNamedArgs),
                Slice(PVariantArray(Parms.rgvarg)^, Parms.cArgs));
          end;
        dkSubComponent:
          POleVariant(AVarResult)^ := GetObjectDispatchEx(TComponent(DispatchInfo.Instance).Components[DispatchInfo.Index]) as IDispatch;
      end;
    end else
    begin
      Result := DISP_E_MEMBERNOTFOUND;

      if (ADispID >= ofCustomDispIDOffset) and (ADispID < ofCustomDispIDOffset + FCustomDispatch.Count) then
      begin
        with TDispCallbackImpl(FCustomDispatch.Objects[ADispID - ofCustomDispIDOffset]) do
        begin
          POleVariant(AVarResult)^ := Call(FInstance, AFlags, Parms^);
          Result := S_OK;
        end;
      end;
    end;
  except
    if AExcepInfo <> nil then
    begin
      FillChar(AExcepInfo^, SizeOf(TExcepInfo), 0);
      {$IFDEF LAZARUS}
      with TExcepInfo(AExcepInfo^) do
      begin
        Source := StringToOleStr(ClassName);
        if ExceptObject is Exception then
          Description := StringToOleStr(Exception(ExceptObject).Message);
        scode := E_FAIL;
      end;
      {$ELSE}
      with TExcepInfo(AExcepInfo^) do
      begin
        bstrSource := StringToOleStr(ClassName);
        if ExceptObject is Exception then
          bstrDescription := StringToOleStr(Exception(ExceptObject).Message);
        scode := E_FAIL;
      end;
      {$ENDIF}
    end;
    Result := DISP_E_EXCEPTION;
  end;
end;

function TDispatchWrapper.GetTypeInfo(AIndex, ALocaleID: Integer;
  out ATypeInfo): HRESULT;
begin
  Result := E_NOTIMPL;
end;

function TDispatchWrapper.GetTypeInfoCount(out ACount: Integer): HRESULT;
begin
  Result := E_NOTIMPL;
end;

function TDispatchWrapper.GetMethodInfo(const AName: ShortString;
  var AInstance: TObject): PMethodInfoHeader;
begin
  Result := ObjAuto.GetMethodInfo(FInstance, AName);
  if Result <> nil then
    AInstance := FInstance;
end;

function TDispatchWrapper.GetPropInfo(const AName: string;
  var AInstance: TObject; var CompIndex: Integer): PPropInfo;
var
  Component: TComponent;
begin
  CompIndex := -1;
  Result := TypInfo.GetPropInfo(FInstance, AName);
  if (Result = nil) and (FInstance is TComponent) then
  begin
    // Not a property, try a sub component
    Component := TComponent(FInstance).FindComponent(AName);
    if Component <> nil then
    begin
      AInstance := FInstance;
      CompIndex := Component.ComponentIndex;
    end;
  end else if Result <> nil then
    AInstance := FInstance
  else
    AInstance := nil;
end;

class procedure TDispatchWrapper.Init;
begin
  FLock := TCriticalSection.Create;
  FObjectList := TStringList.Create;
  FCustomDispatch := TStringList.Create;
end;

class procedure TDispatchWrapper.UnInit;
var
  i: Integer;
begin
  if Assigned(FObjectList) and (FObjectList.Count > 0) then
  begin
    WriteView(Format('TObjectDispatch个数: %d', [FObjectList.Count]));
    for i := 0 to FObjectList.Count - 1 do
    begin
      WriteView(Format('    对象类名: %s', [FObjectList.ValueFromIndex[i]]));
    end;
  end;

  for i := 0 to FCustomDispatch.Count - 1 do
    FCustomDispatch.Objects[i].Free;
  FreeAndNil(FCustomDispatch);

  FreeAndNil(FObjectList);
  FreeAndNil(FLock);
end;

class procedure TDispatchWrapper.Add(ASelf: TDispatchWrapper);
begin
  FLock.Enter;
  try
    FObjectList.AddObject(Format('%d=%s', [Integer(ASelf), ASelf.FInstance.ClassName]), ASelf);
  finally
    FLock.Leave;
  end;
end;

class procedure TDispatchWrapper.Delete(ASelf: TDispatchWrapper);
begin
  if Assigned(FObjectList) then
  begin
    FLock.Enter;
    try
      FObjectList.Delete(FObjectList.IndexOfName(IntToStr(Integer(ASelf))));
    finally
      FLock.Leave;
    end;
  end;
end;

class procedure TDispatchWrapper.RegCustomDispatch(AClass: TClass; AName: String; ADispCallback: TDispCallback);
var
  i: Integer;
begin
  FLock.Enter;
  try
    for i := 0 to FCustomDispatch.Count - 1 do
      if TDispCallbackImpl(FCustomDispatch.Objects[i]).Replace(AClass, AName, ADispCallback) then
        Exit;

    FCustomDispatch.AddObject('', TDispCallbackImpl.Create(AClass, AName, ADispCallback));
  finally
    FLock.Leave;
  end;
end;

initialization
  TDispatchWrapper.Init;

finalization
  TDispatchWrapper.UnInit;

end.
