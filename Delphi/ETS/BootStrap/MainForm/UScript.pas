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
unit UScript;

interface

uses
  Windows, Classes, SysUtils, ActiveX, Variants, UDUICore, UFrameBase, UInterface;

type
  TFrmScriptFrame = class(TFrameBase)
    procedure FrameBaseInit(AParent: IParent; AIndex: Integer);
  private
    FScript: IScript;
  published
    property AcceptDropFiles;
  end;

implementation

uses
  Controls, TypInfo, qjson, UModuleBase, UDispatchWrapper, UDUIUtils, UDUIForm, UTool;

{$R *.dfm}

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

//GetFrame
//��ȡĳ���ؼ�������ӿؼ���֧�ֶ༶���ң�֧��ʹ�ÿؼ����ƻ�����
//����0��
//Ҫ��ȡ�����·������֧�ְ���������������ַ�ʽ���ң��༶·���á�.������
//�������գ��򷵻ظ����
//��������(js����)��
//objFrame.GetFrame('Parent1.Parent2.<�������ؼ�>.Control')
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

    raise Exception.Create('�ؼ�������');
  end;
  function getControlByIndex(AParent: TDUIBase; AIndex: Integer): TDUIBase;
  begin
    if (AIndex < 0) or (AIndex >= AParent.ControlCount) then
      raise Exception.Create('�ؼ�������');

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
//����json��ʽ�����ã������ӿؼ����ؼ�֧�ּ���(����dfm��������json��Ϊ����)
//��������(js����)��
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
//����AObject���ӿؼ�(���ؼ������ŵ��ű��д�������ȡ��CreateFrame�Ĺ���)
//��������(js����)��
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
      WriteView('�޷��ҵ�����Ϣ(%s)', [VarToStr(AParams[0])]);
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
    TControl(obj).Parent := TWinControl(AObject)
  else
  begin
    obj.Free;
    Exit;
  end;

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
  //�ؼ������Ĺ���ͳһ�������߳��д�����ֹ������Ŀؼ�����Ϣѭ����������
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

end.
