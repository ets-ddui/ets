{
  Copyright (c) 2021-2031 Steven Shi

  ETS(Extended Tool Set)，可扩展工具集。

  本工具软件是开源自由软件，您可以遵照 MIT 协议，修改和发布此程序。
  发布此库的目的是希望其有用，但不做任何保证。
  如果将本库用于商业项目，由于本库中的Bug，而引起的任何风险及损失，本作者不承担任何责任。

  开源地址: https://github.com/ets-ddui/ets
            https://gitee.com/ets-ddui/ets
  开源协议: The MIT License (MIT)
  作者邮箱: xinghun87@163.com
  官方博客：https://blog.csdn.net/xinghun61
}
unit UScript;

interface

uses
  Windows, Classes, SysUtils, ActiveX, Variants, UDUICore, UFrameBase, UInterface;

type
  TOnETSAfterNotify = procedure(ANotifyType: String) of object;
  TFrmScriptFrame = class(TFrameBase)
    procedure FrameBaseInit(AParent: IParent; AIndex: Integer);
  private
    FScript: IScript;
    FOnETSAfterNotify: TOnETSAfterNotify;
    procedure DoAfterNotify(ANotifyType: TNotifyType);
    procedure SetOnETSAfterNotify(const AValue: TOnETSAfterNotify);
  published
    property AcceptDropFiles;
    property OnETSAfterNotify: TOnETSAfterNotify read FOnETSAfterNotify write SetOnETSAfterNotify;
  end;

implementation

uses
  Controls, TypInfo, qjson, UModuleBase, UDispatchWrapper, UDUIUtils, UDUIForm, UTool;

{$R *.dfm}

procedure TFrmScriptFrame.DoAfterNotify(ANotifyType: TNotifyType);
const
  CName: array[TNotifyType] of String = ('ntActive', 'ntDeActive', 'ntResize', 'ntIdle', 'ntToggle');
begin
  if Assigned(FOnETSAfterNotify) then
    FOnETSAfterNotify(CName[ANotifyType]);
end;

procedure TFrmScriptFrame.FrameBaseInit(AParent: IParent; AIndex: Integer);
  function getScript(AScriptName: String): IScript;
  var
    objModule, objScript: IDispatch;
  begin
    Result := nil;

    objModule := GetRawManager.Plugins['Script'];
    if not Assigned(objModule) then
      Exit;

    case TScriptLanguage(GetEnumValue(TypeInfo(TScriptLanguage), AScriptName)) of
      slJScript: objScript := Variant(objModule).GetJScript;
      slPython: objScript := Variant(objModule).GetPython;
    else
      Exit;
    end;

    if not Assigned(objScript) then
      Exit;

    if not Supports(objScript, IScript, Result) then
      Result := nil;
  end;
var
  js, jsConfig: TQJson;
begin
  jsConfig := TQJson(AParent.Param[AIndex].FConfig);
  if not Assigned(jsConfig) then
    Exit;

  FScript := getScript(jsConfig.ValueByName('Language', 'slJScript'));
  if not Assigned(FScript) then
    Exit;

  FScript.RegContainer(nil);
  FScript.RegFrame(WrapperObject(Self, False));

  if jsConfig.HasChild('File', js) then
    //FScript.RunModule(js.Value, 'Init();')
    FScript.RunCode(Format('var __main__ = Require("%s"); __main__.Init();', [js.Value]))
  else if jsConfig.HasChild('Source', js) then
    FScript.RunCode(js.Value);
end;

procedure TFrmScriptFrame.SetOnETSAfterNotify(const AValue: TOnETSAfterNotify);
begin
  OnAfterNotify := DoAfterNotify;
  FOnETSAfterNotify := AValue;
end;

//GetFrame
//获取某个控件下面的子控件，支持多级查找，支持使用控件名称或索引
//参数0：
//要获取组件的路径名，支持按索引和组件名两种方式查找，多级路径用“.”连接
//如果输入空，则返回根组件
//调用样例(js代码)：
//objFrame.GetFrame('Parent1.Parent2.<其他父控件>.Control')
function GetFrame(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray): OleVariant;
  function getControlByName(AParent: TDUIBase; AName: String): TDUIBase;
  var
    i: Integer;
  begin
    for i := 0 to AParent.ControlCount - 1 do
    begin
      Result := AParent.Controls[i];
      if CompareText(Result.Name, AName) = 0 then
        Exit;
    end;

    raise Exception.Create('控件不存在');
  end;
  function getControlByIndex(AParent: TDUIBase; AIndex: Integer): TDUIBase;
  begin
    if (AIndex < 0) or (AIndex >= AParent.ControlCount) then
      raise Exception.Create('控件不存在');

    Result := AParent.Controls[AIndex];
  end;
var
  ctl: TDUIBase;
  pcBegin, pcEnd: PChar;
  strName: String;
begin
  Result := Null;
  if not (AObject is TDUIBase) then
    Exit;
  if AParamCount <> 1 then
    Exit;

  ctl := TDUIBase(AObject);
  strName := VarToStr(AParams[0]);
  pcBegin := @strName[1];
  if pcBegin^ = '[' then
    pcBegin := pcBegin + 1;
  pcEnd := pcBegin + 1;
  while pcBegin^ <> #0 do
  begin
    case pcEnd^ of
      '[', '.':
      begin
        if pcBegin < pcEnd then
          ctl := getControlByName(ctl, Copy(pcBegin, 0, pcEnd - pcBegin));
        pcBegin := pcEnd + 1;
        pcEnd := pcBegin;
      end;
      ']':
      begin
        ctl := getControlByIndex(ctl, StrToIntDef(Copy(pcBegin, 0, pcEnd - pcBegin), -1));
        pcBegin := pcEnd + 1;
        pcEnd := pcBegin;
      end;
      #0:
      begin
        ctl := getControlByName(ctl, Copy(pcBegin, 0, pcEnd - pcBegin));
        Break;
      end;
    else
      Inc(pcEnd);
    end;
  end;

  if Assigned(ctl) then
    Result := WrapperObject(ctl, False);
end;

//CreateFrame
//传入json格式的配置，创建子控件，控件支持级联(类似dfm，但改用json作为配置)
//调用样例(js代码)：
//objFrame.CreateFrame('{"__class__": "TDUIBase", "__property__": {...}, "__child__": [...]}')
function CreateFrame(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray): OleVariant;
var
  js: TQJson;
  ss: TStringStream;
  ctl: TObject;
begin
  Result := Null;
  if not (AObject is TDUIBase) then
    Exit;
  if AParamCount <> 1 then
    Exit;

  js := nil;
  ss := nil;
  ctl := nil;
  try
    js := TQJson.Create;
    ss := TStringStream.Create(VarToStr(AParams[0]));
    js.LoadFromStream(ss);
    JsonToComponent(ctl, js, AObject);
  finally
    FreeAndNil(js);
    FreeAndNil(ss);
  end;

  if Assigned(ctl) then
    Result := WrapperObject(ctl, False);
end;

//CreateChild
//创建AObject的子控件(将控件创建放到脚本中处理，用于取代CreateFrame的功能)
//调用样例(js代码)：
//objFrame.CreateChild('TDUIBase')
function CreateChild(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray): OleVariant;
var
  cc: TComponentClass;
  obj: TComponent;
begin
  Result := Null;
  if AParamCount <> 1 then
    Exit;

  try
    cc := TComponentClass(FindClass(VarToStr(AParams[0])));
  except
    on e: EClassNotFound do
    begin
      WriteView('无法找到类信息(%s)', [VarToStr(AParams[0])]);
      Exit;
    end;
  end;

  obj := cc.Create(TComponent(AObject));
  if obj is TDUIBase then
  begin
    if AObject is TDUIForm then
      TDUIBase(obj).Parent := TWinControl(AObject)
    else if AObject is TDUIBase then
      TDUIBase(obj).DUIParent := TDUIBase(AObject)
    else
    begin
      obj.Free;
      Exit;
    end;
  end
  else if (obj is TControl) and (AObject is TWinControl) then
    TControl(obj).Parent := TWinControl(AObject);

  Result := WrapperObject(obj, False);
end;

//FindChild
//从所有子孙控件中，查找第一个名称为ControlName的控件
//调用样例(js代码)：
//objFrame.FindChild('ControlName')
function FindChild(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray): OleVariant;
  function doFind(AParent: TControl; AName: String): TControl;
  var
    i: Integer;
  begin
    if CompareText(AParent.Name, AName) = 0 then
    begin
      Result := AParent;
      Exit;
    end;

    if AParent is TDUIBase then
    begin
      for i := 0 to TDUIBase(AParent).ControlCount - 1 do
      begin
        Result := doFind(TDUIBase(AParent).Controls[i], AName);
        if Assigned(Result) then
          Exit;
      end;
    end
    else if AParent is TWinControl then
    begin
      for i := 0 to TWinControl(AParent).ControlCount - 1 do
      begin
        Result := doFind(TWinControl(AParent).Controls[i], AName);
        if Assigned(Result) then
          Exit;
      end;
    end;

    Result := nil;
  end;
var
  obj: TControl;
begin
  Result := Null;
  if AParamCount <> 1 then
    Exit;

  obj := doFind(TControl(AObject), VarToStr(AParams[0]));
  if not Assigned(obj) then
    Exit;

  Result := WrapperObject(obj, False);
end;

type
  TSyncCallback = class
  strict private
    FObject: TObject;
    FName: String;
    FFlag: Word;
    FParamCount: Integer;
    FParams: PVariantArray;
    FResult: POleVariant;
    class function DoCall(AObject: TObject; AName: String; AFlag: Word;
      AParamCount: Integer; const AParams: PVariantArray): OleVariant;
  public
    constructor Create(AObject: TObject; AName: String; AFlag: Word;
      AParamCount: Integer; const AParams: PVariantArray; AResult: POleVariant);
    procedure Call;
  public
    class function SyncCall(AObject: TObject; AName: String; AFlag: Word;
      AParamCount: Integer; const AParams: PVariantArray): OleVariant;
  end;

{ TSyncCallback }

constructor TSyncCallback.Create(AObject: TObject; AName: String; AFlag: Word;
  AParamCount: Integer; const AParams: PVariantArray; AResult: POleVariant);
begin
  inherited Create;

  FObject := AObject;
  FName := AName;
  FFlag := AFlag;
  FParamCount := AParamCount;
  FParams := AParams;
  FResult := AResult;
end;

procedure TSyncCallback.Call;
begin
  FResult^ := DoCall(FObject, FName, FFlag, FParamCount, FParams);
end;

class function TSyncCallback.DoCall(AObject: TObject; AName: String;
  AFlag: Word; AParamCount: Integer; const AParams: PVariantArray): OleVariant;
begin
  if CompareText(AName, 'CreateFrame') = 0 then
    Result := CreateFrame(AObject, AName, AFlag, AParamCount, AParams)
  else if CompareText(AName, 'CreateChild') = 0 then
    Result := CreateChild(AObject, AName, AFlag, AParamCount, AParams)
  else
    Result := Null;
end;

class function TSyncCallback.SyncCall(AObject: TObject; AName: String;
  AFlag: Word; AParamCount: Integer; const AParams: PVariantArray): OleVariant;
begin
  //控件创建的过程统一放在主线程中处理，防止待句柄的控件的消息循环出现问题
  if GetCurrentThreadID = MainThreadID then
    Result := DoCall(AObject, AName, AFlag, AParamCount, AParams)
  else
    with TSyncCallback.Create(AObject, AName, AFlag, AParamCount, AParams, @Result) do
      try
        TThread.Synchronize(nil, Call);
      finally
        Free;
      end;
end;

function SyncCall(AObject: TObject; AName: String;
  AFlag: Word; AParamCount: Integer; const AParams: PVariantArray): OleVariant;
begin
  Result := TSyncCallback.SyncCall(AObject, AName, AFlag, AParamCount, AParams);
end;

initialization
  TDispatchWrapper.RegCustomDispatch(TDUIBase, 'GetFrame', GetFrame);
  TDispatchWrapper.RegCustomDispatch(TDUIBase, 'CreateFrame', SyncCall);
  TDispatchWrapper.RegCustomDispatch(TControl, 'CreateChild', SyncCall);
  TDispatchWrapper.RegCustomDispatch(TControl, 'FindChild', FindChild);

end.
