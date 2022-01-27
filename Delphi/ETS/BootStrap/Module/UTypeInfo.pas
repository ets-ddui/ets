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
unit UTypeInfo;

interface

uses
  Windows, Classes, SysUtils, ActiveX, UInterface;

type
  //Windows和System中都有定义PWord类型，此文件如果没有引用Windows，
  //则GetDllEntry的入参AOrdinal，定义的就是System的类型，而ActiveX却使用的是Windows，
  //所以，会导致TTypeInfo定义与声明不一致的问题
  TTypeInfo = class(TInterfacedBase, ITypeInfo)
  private
    //ITypeInfo接口实现
    function GetTypeAttr(out ATypeAttr: PTypeAttr): HResult; stdcall;
    function GetTypeComp(out ATypeComp: ITypeComp): HResult; stdcall;
    function GetFuncDesc(AIndex: Integer; out AFuncDesc: PFuncDesc): HResult; stdcall;
    function GetVarDesc(AIndex: Integer; out AVarDesc: PVarDesc): HResult; stdcall;
    function GetNames(AMemberID: TMemberID; ANames: PBStrList;
      AMaxNameCount: Integer; out ANameCount: Integer): HResult; stdcall;
    function GetRefTypeOfImplType(AIndex: Integer; out ARefType: HRefType): HResult; stdcall;
    function GetImplTypeFlags(AIndex: Integer; out AImplTypeFlags: Integer): HResult; stdcall;
    function GetIDsOfNames(ANames: POleStrList; ANameCount: Integer;
      AMemberID: PMemberIDList): HResult; stdcall;
    function Invoke(AInstance: Pointer; AMemberID: TMemberID; AFlags: Word;
      var ADispParams: TDispParams; AResult: PVariant;
      AExcepInfo: PExcepInfo; AError: PInteger): HResult; stdcall;
    function GetDocumentation(AMemberID: TMemberID; AName: PWideString;
      ADocString: PWideString; AHelpContext: PLongint; AHelpFile: PWideString): HResult; stdcall;
    function GetDllEntry(AMemberID: TMemberID; AInvokeKind: TInvokeKind;
      ADllName, AName: PWideString; AOrdinal: Windows.PWord): HResult; stdcall;
    function GetRefTypeInfo(ARefType: HRefType; out ATypeInfo: ITypeInfo): HResult; stdcall;
    function AddressOfMember(AMemberID: TMemberID; AInvokeKind: TInvokeKind;
      out AResult: Pointer): HResult; stdcall;
    function CreateInstance(const AOuter: IUnknown; const AIID: TIID; out AResult): HResult; stdcall;
    function GetMops(AMemberID: TMemberID; out AMops: WideString): HResult; stdcall;
    function GetContainingTypeLib(out ATypeLib: ITypeLib; out AIndex: Integer): HResult; stdcall;
    procedure ReleaseTypeAttr(ATypeAttr: PTypeAttr); stdcall;
    procedure ReleaseFuncDesc(AFuncDesc: PFuncDesc); stdcall;
    procedure ReleaseVarDesc(AVarDesc: PVarDesc); stdcall;
  private
    FObject: TObject;
    FFuncName: TStringList;
    FFuncDesc: TList;
    FTypeAttr: TTypeAttr;
  public
    constructor Create(AObject: TObject); reintroduce;
    destructor Destroy; override;
  end;

implementation

uses
  TypInfo, ObjAuto, URtti;

{ TTypeInfo }

constructor TTypeInfo.Create(AObject: TObject);
  procedure enumAll(AVmt: PVmt);
    procedure addFuncDesc(AName: String; AInvokeKind: TInvokeKind);
    var
      fd: PFuncDesc;
    begin
      New(fd);
      fd.memid := FFuncDesc.Count;
      fd.funckind := FUNC_PUREVIRTUAL;
      fd.invkind := AInvokeKind;
      fd.callconv := CC_PASCAL;

      FFuncName.Add(AName);
      FFuncDesc.Add(fd);
    end;

    procedure enumProperty(ATypeInfo: PTypeInfo);
    var
      i: Integer;
      ptd: PTypeData;
      ppd: PPropData;
      ppi: PPropInfo;
    begin
      if ATypeInfo^.Kind <> tkClass then
        Exit;

      ptd := GetTypeData(ATypeInfo);
      ppd := Pointer(DWORD(@ptd^.UnitName) + Length(ptd^.UnitName) + 1);
      ppi := PPropInfo(@ppd^.PropList);
      for i := 1 to ppd^.PropCount do
      begin
        if Assigned(ppi^.GetProc) then
          addFuncDesc(ppi^.Name, INVOKE_PROPERTYGET);

        if Assigned(ppi^.SetProc) then
          addFuncDesc(ppi^.Name, INVOKE_PROPERTYPUT);

        ppi := Pointer(DWORD(@ppi^.Name) + Length(ppi^.Name) + 1);
      end;
    end;

    procedure enumMethod(AMethodInfo: PMethodInfo);
    var
      i: Integer;
      pmih: PMethodInfoHeader;
    begin
      pmih := Pointer(@AMethodInfo^.MethodList);
      for i := 1 to AMethodInfo^.MethodCount do
      begin
        addFuncDesc(pmih^.Name, INVOKE_FUNC);

        pmih := Pointer(DWORD(pmih) + pmih^.Len);
      end;
    end;
  begin
    if Assigned(AVmt^.vParent) then
      enumAll(TRtti.GetVmt((TObject(AVmt^.vParent))));

    if Assigned(AVmt^.vTypeInfo) then
      enumProperty(PTypeInfo(AVmt^.vTypeInfo));

    if Assigned(AVmt^.vMethodTable) then
      enumMethod(PMethodInfo(AVmt^.vMethodTable));
  end;

  procedure initFuncDesc;
  begin
    FFuncName := TStringList.Create;
    FFuncDesc := TList.Create;

    if Assigned(FObject) then
      enumAll(TRtti.GetVmt(FObject));
  end;

  procedure initTypeAttr;
  begin
    FTypeAttr.memidConstructor := MEMBERID_NIL;
    FTypeAttr.memidDestructor := MEMBERID_NIL;
    if Assigned(FObject) then
      FTypeAttr.cbSizeInstance := FObject.InstanceSize;
    FTypeAttr.typekind := TKIND_DISPATCH;
    FTypeAttr.cFuncs := FFuncDesc.Count;
    FTypeAttr.cbSizeVft := 12; //虚拟函数表暂时无法精确计算，先按IUnknown的大小处理
    FTypeAttr.cbAlignment := 1;
  end;

begin
  inherited Create(nil);

  FObject := AObject;
  initFuncDesc;
  initTypeAttr;
end;

destructor TTypeInfo.Destroy;
var
  i: Integer;
begin
  for i := FFuncDesc.Count - 1 downto 0 do
    Dispose(FFuncDesc[i]);
  FreeAndNil(FFuncDesc);

  FreeAndNil(FFuncName);

  inherited;
end;

function TTypeInfo.AddressOfMember(AMemberID: TMemberID; AInvokeKind: TInvokeKind;
  out AResult: Pointer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.CreateInstance(const AOuter: IInterface; const AIID: TIID;
  out AResult): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetContainingTypeLib(out ATypeLib: ITypeLib; out AIndex: Integer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetDllEntry(AMemberID: TMemberID; AInvokeKind: TInvokeKind;
  ADllName, AName: PWideString; AOrdinal: Windows.PWord): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetDocumentation(AMemberID: TMemberID;
  AName, ADocString: PWideString; AHelpContext: PLongint; AHelpFile: PWideString): HResult;
begin
  if AName = nil then
  begin
    Result := E_POINTER;
    Exit;
  end;

  if (AMemberID < 0) or (AMemberID >= FFuncName.Count) then
  begin
    Result := E_INVALIDARG;
    Exit;
  end;

  AName^ := FFuncName.Strings[AMemberID];
  Result := S_OK;
end;

function TTypeInfo.GetFuncDesc(AIndex: Integer; out AFuncDesc: PFuncDesc): HResult;
begin
  if (AIndex < 0) or (AIndex >= FFuncDesc.Count) then
  begin
    Result := E_INVALIDARG;
    Exit;
  end;

  AFuncDesc := PFuncDesc(FFuncDesc[AIndex]);
  Result := S_OK;
end;

function TTypeInfo.GetIDsOfNames(ANames: POleStrList; ANameCount: Integer;
  AMemberID: PMemberIDList): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetImplTypeFlags(AIndex: Integer; out AImplTypeFlags: Integer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetMops(AMemberID: TMemberID; out AMops: WideString): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetNames(AMemberID: TMemberID; ANames: PBStrList;
  AMaxNameCount: Integer; out ANameCount: Integer): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetRefTypeInfo(ARefType: HRefType; out ATypeInfo: ITypeInfo): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetRefTypeOfImplType(AIndex: Integer; out ARefType: HRefType): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetTypeAttr(out ATypeAttr: PTypeAttr): HResult;
begin
  ATypeAttr := @FTypeAttr;
  Result := S_OK;
end;

function TTypeInfo.GetTypeComp(out ATypeComp: ITypeComp): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.GetVarDesc(AIndex: Integer; out AVarDesc: PVarDesc): HResult;
begin
  Result := E_NOTIMPL;
end;

function TTypeInfo.Invoke(AInstance: Pointer; AMemberID: TMemberID;
  AFlags: Word; var ADispParams: TDispParams; AResult: PVariant;
  AExcepInfo: PExcepInfo; AError: PInteger): HResult;
begin
  Result := E_NOTIMPL;
end;

procedure TTypeInfo.ReleaseFuncDesc(AFuncDesc: PFuncDesc);
begin

end;

procedure TTypeInfo.ReleaseTypeAttr(ATypeAttr: PTypeAttr);
begin

end;

procedure TTypeInfo.ReleaseVarDesc(AVarDesc: PVarDesc);
begin

end;

end.
