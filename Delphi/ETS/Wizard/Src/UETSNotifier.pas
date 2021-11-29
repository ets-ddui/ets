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
    //IInterface实现
    function QueryInterface(const AIID: TGUID; out AObj): HResult; stdcall;
    function _AddRef: Integer; stdcall;
    function _Release: Integer; stdcall;
  private
    //IOTANotifier实现
    procedure AfterSave;
    procedure BeforeSave;
    procedure Destroyed;
    procedure Modified;
  private
    FList: TList;
    FBlankCount: Integer; //FList中未使用的记录数
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
    //IOTAIDENotifier实现
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

    Assert(Assigned(p), 'TETSNotifier.AddNotifier FBlankCount状态不正确');
  end;

  p.FUsed := True;
  p.FNotifier := ANotifier;
end;

procedure TETSNotifier.RemoveNotifier(AIndex: Integer);
begin
  Assert((AIndex >= 0) and (AIndex < FList.Count));
  Assert(PETSNotifierMethod(FList[AIndex]).FUsed, '重复释放');

  PETSNotifierMethod(FList[AIndex]).FUsed := False;
  Inc(FBlankCount);

  {TODO: 这里可以添加内存释放的逻辑，当空闲想过多时，删除结尾部分未使用的项}
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
