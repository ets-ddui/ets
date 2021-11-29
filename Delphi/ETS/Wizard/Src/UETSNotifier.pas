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
unit UETSNotifier;

interface

uses
  Classes, SysUtils, ToolsAPI, UTool;

type
  TOnETSIDENotifier = procedure (ANotifyCode: TOTAFileNotification;
    const AFileName: String) of object;

  TETSNotifierManager = class
  public
    function AddIDENotifier(ANotifier: TOnETSIDENotifier): Integer;
    procedure RemoveIDENotifier(AIndex: Integer);
  private
    class var FManager: TETSNotifierManager;
  public
    class function GetManager: TETSNotifierManager;
    class procedure Init(const ABorlandIDEServices: IBorlandIDEServices;
      ARegisterProc: TWizardRegisterProc);
    class procedure UnInit;
  end;

implementation

type
  TETSNotifierMethod = record
    FUsed: Boolean;
    FNotifier: TMethod;
  end;
  PETSNotifierMethod = ^TETSNotifierMethod;
  TETSNotifier = class(TObject, IOTANotifier)
  private
    //IInterfaceʵ��
    function QueryInterface(const AIID: TGUID; out AObj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  private
    //IOTANotifierʵ��
    procedure AfterSave;
    procedure BeforeSave;
    procedure Destroyed;
    procedure Modified;
  private
    FList: TList;
    FBlankCount: Integer; //FList��δʹ�õļ�¼��
  protected
    procedure DoEnumNotifier(ANotifier: TMethod; AParameter: TObject); virtual; abstract;
  public
    constructor Create; virtual;
    destructor Destroy; override;
    function AddNotifier(ANotifier: TMethod): Integer;
    procedure RemoveNotifier(AIndex: Integer);
    procedure EnumNotifier(AParameter: TObject);
  end;

  TETSIDENotifier = class(TETSNotifier, IOTAIDENotifier)
  private
    //IOTAIDENotifierʵ��
    procedure FileNotification(ANotifyCode: TOTAFileNotification;
      const AFileName: String; var ACancel: Boolean);
    procedure BeforeCompile(const AProject: IOTAProject; var ACancel: Boolean);
    procedure AfterCompile(ASucceeded: Boolean);
  private
    FIndex: Integer;
  protected
    type
      TParameter = class
        FNotifyCode: TOTAFileNotification;
        FFileName: String;
      end;
    procedure DoEnumNotifier(ANotifier: TMethod; AParameter: TObject); override;
  public
    constructor Create; override;
    destructor Destroy; override;
    function AddNotifier(ANotifier: TOnETSIDENotifier): Integer;
  end;

{ TETSNotifier }

function TETSNotifier.AddNotifier(ANotifier: TMethod): Integer;
var
  i: Integer;
  p: PETSNotifierMethod;
begin
  Result := -1;

  p := nil;
  if 0 = FBlankCount then
  begin
    New(p);
    Result := FList.Add(p);
  end
  else
  begin
    Dec(FBlankCount);

    for i := 0 to FList.Count - 1 do
      if not PETSNotifierMethod(FList[i]).FUsed then
      begin
        p := FList[i];
        Result := i;
        Break;
      end;

    Assert(Assigned(p), 'TETSNotifier.AddNotifier FBlankCount״̬����ȷ');
  end;

  p.FUsed := True;
  p.FNotifier := ANotifier;
end;

procedure TETSNotifier.RemoveNotifier(AIndex: Integer);
begin
  Assert((AIndex >= 0) and (AIndex < FList.Count));
  Assert(PETSNotifierMethod(FList[AIndex]).FUsed, '�ظ��ͷ�');

  PETSNotifierMethod(FList[AIndex]).FUsed := False;
  Inc(FBlankCount);

  {TODO: �����������ڴ��ͷŵ��߼��������������ʱ��ɾ����β����δʹ�õ���}
end;

procedure TETSNotifier.EnumNotifier(AParameter: TObject);
var
  i: Integer;
begin
  for i := 0 to FList.Count - 1 do
    if PETSNotifierMethod(FList[i]).FUsed then
      DoEnumNotifier(PETSNotifierMethod(FList[i]).FNotifier, AParameter);
end;

constructor TETSNotifier.Create;
begin
  FList := TList.Create;
  FBlankCount := 0;
end;

destructor TETSNotifier.Destroy;
var
  i: Integer;
begin
  for i := 0 to FList.Count - 1 do
    Dispose(FList[i]);
  FreeAndNil(FList);

  inherited;
end;

procedure TETSNotifier.AfterSave;
begin

end;

procedure TETSNotifier.BeforeSave;
begin

end;

procedure TETSNotifier.Destroyed;
begin

end;

procedure TETSNotifier.Modified;
begin

end;

function TETSNotifier.QueryInterface(const AIID: TGUID; out AObj): HResult;
begin
  if GetInterface(AIID, AObj) then
    Result := S_OK
  else
    Result := E_NOINTERFACE;
end;

function TETSNotifier._AddRef: Integer;
begin
  Result := -1;
end;

function TETSNotifier._Release: Integer;
begin
  Result := -1;
end;

{ TETSNotifier }

constructor TETSIDENotifier.Create;
begin
  inherited;

  FIndex := (BorlandIDEServices as IOTAServices).AddNotifier(Self);
end;

destructor TETSIDENotifier.Destroy;
begin
  if FIndex >= 0 then
  begin
    (BorlandIDEServices as IOTAServices).RemoveNotifier(FIndex);
    FIndex := -1;
  end;

  inherited;
end;

procedure TETSIDENotifier.DoEnumNotifier(ANotifier: TMethod; AParameter: TObject);
begin
  TOnETSIDENotifier(ANotifier)(TParameter(AParameter).FNotifyCode,
    TParameter(AParameter).FFileName);
end;

function TETSIDENotifier.AddNotifier(ANotifier: TOnETSIDENotifier): Integer;
begin
  Result := inherited AddNotifier(TMethod(ANotifier));
end;

procedure TETSIDENotifier.AfterCompile(ASucceeded: Boolean);
begin

end;

procedure TETSIDENotifier.BeforeCompile(const AProject: IOTAProject; var ACancel: Boolean);
begin

end;

procedure TETSIDENotifier.FileNotification(ANotifyCode: TOTAFileNotification;
  const AFileName: String; var ACancel: Boolean);
var
  pam: TParameter;
begin
  pam := TParameter.Create;
  try
    pam.FNotifyCode := ANotifyCode;
    pam.FFileName := AFileName;

    EnumNotifier(pam);
  finally
    FreeAndNil(pam);
  end;
end;

{ TETSNotifierManager }

var
  GETSIDENotifier: TETSIDENotifier;

function TETSNotifierManager.AddIDENotifier(ANotifier: TOnETSIDENotifier): Integer;
begin
  if not Assigned(GETSIDENotifier) then
    GETSIDENotifier := TETSIDENotifier.Create;
  
  Result := GETSIDENotifier.AddNotifier(ANotifier);
end;

procedure TETSNotifierManager.RemoveIDENotifier(AIndex: Integer);
begin
  if Assigned(GETSIDENotifier) then
    GETSIDENotifier.RemoveNotifier(AIndex);
end;

class function TETSNotifierManager.GetManager: TETSNotifierManager;
begin
  Result := FManager;
end;

class procedure TETSNotifierManager.Init(const ABorlandIDEServices: IBorlandIDEServices;
  ARegisterProc: TWizardRegisterProc);
begin
  if not Assigned(FManager) then
    FManager := TETSNotifierManager.Create;
end;

class procedure TETSNotifierManager.UnInit;
begin
  FreeAndNil(FManager);
  FreeAndNil(GETSIDENotifier);
end;

initialization
  TETSNotifierManager.FManager := nil;
  GETSIDENotifier := nil;

finalization
  TETSNotifierManager.UnInit;

end.
