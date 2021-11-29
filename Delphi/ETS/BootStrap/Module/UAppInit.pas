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
unit UAppInit;

{$i UConfigure.inc}

interface

uses
  Windows, Classes, SysUtils, Graphics, UInterface;

type
  TCreateForm = procedure;
  TDoIdle = procedure(ASender: TObject);

  TAppInit = class
  private
    procedure CreateForm;
    //procedure DoException(ASender: TObject; AException: Exception);
    procedure DoMessage(var AMsg: TMsg; var AHandled: Boolean);
    procedure DoIdle(ASender: TObject; var ADone: Boolean);
  private
    class var FControlAtomList: TStringList;
    class var FAppInit: TAppInit;
    class var FCreateForm: TCreateForm;
    class var FDoIdle: TDoIdle;
    class procedure ExeInit;
    class procedure DllInit;
  public
    class procedure AddControlAtom(AValue: String);
    class procedure Init(AIsLibrary: Boolean);
    class procedure UnInit;
    class procedure RegCreateForm(ACreateForm: TCreateForm; ADoIdle: TDoIdle);
  end;

implementation

uses
  Forms, Controls, Messages, DSIntf, JclDebug, JclHookExcept, UETSRegComponents,
  UTool;

{ TAppInit }

class procedure TAppInit.AddControlAtom(AValue: String);
begin
  FControlAtomList.Add(AValue);
end;

procedure TAppInit.CreateForm;
begin
  if Assigned(FCreateForm) then
  begin
    //����MainFormOnTaskbar���������л������չ��ʽWS_EX_APPWINDOW
    //�����ʽ��Ч����������������С��ʱ������WM_Size��Ϣ(Ӧ�ú��������Ĳ����й�)
    Application.MainFormOnTaskbar := True;

    UETSRegComponents.RegComponents;

    FCreateForm();
  end;
end;

{$IFDEF LAZARUS}
function UpdateLayeredWindow(AHandle: THandle; ADestDC: HDC; ADestPosition: PPoint;
  ASize: PSize; ASrcDC: HDC; ASrcPosition: PPoint;
  AColor: COLORREF; ABlend: PBLENDFUNCTION; AFlags: DWORD): Boolean; stdcall;
  external 'user32.dll' name 'UpdateLayeredWindow';
{$ENDIF}

procedure TAppInit.DoMessage(var AMsg: TMsg; var AHandled: Boolean);
  procedure DoKeyMessage;
  var
    ctl: TWinControl;
    i: Integer;
    atomExe, atomDll: TAtom;
  begin
    //�����������DLL��ʵ�ֵģ��������������߳��д����Ļ���
    //TApplication.ProcessMessage������EXE�д���ģ�
    //FindControl����ȡ��ControlAtomString����EXE�д��ֵ������DLL�д��ֵ��
    //����FindControl���ش����ֵ�����Ӱ����̰�����Ϣ�Ĵ���
    //���´����޷���Ӧ���ơ����Ƶļ��̲�����
    //����ͨ����DLL�д�ŵ�����ֵ������һ�ݵ�EXE��Ӧ��Atom�����У��ﵽ�޸��������Ŀ��
    atomExe := GlobalFindAtom(PChar(Format('ControlOfs%.8X%.8X', [HInstance, GetCurrentThreadID])));
    if atomExe = 0 then
      Exit;

    ctl := TWinControl(Pointer(GetProp(AMsg.hwnd, MakeIntAtom(atomExe))));
    if not Assigned(ctl) then
    begin
      for i := 0 to FControlAtomList.Count - 1 do
      begin
        atomDll := GlobalFindAtom(PChar(FControlAtomList.Strings[i]));
        if atomDll = 0 then
          Continue;

        ctl := TWinControl(Pointer(GetProp(AMsg.hwnd, MakeIntAtom(atomDll))));
        if not Assigned(ctl) then
          Continue;

        SetProp(AMsg.hwnd, MakeIntAtom(atomExe), THandle(ctl));
        Exit;
      end;
    end;
  end;
  procedure DoPaintMessage;
  var
    ctl: TWinControl;
    iExStyle: Integer;
    bUpdateLayeredWindow, bUpdateSucces: Boolean;
    hDC, hMemDC: Cardinal;
    hMemBitmap, hOldBitmap: HBITMAP;
    ps: TPaintStruct;
    bf: TBlendFunction;
    ptMem, ptScreen: TPoint;
    szScreen: TSize;
  begin
    ctl := FindControl(AMsg.hwnd);
    if not Assigned(ctl) then
      Exit;

    bUpdateLayeredWindow := True;
    iExStyle := GetWindowLong(ctl.Handle, GWL_EXSTYLE);
    if (iExStyle and WS_EX_LAYERED) = 0 then
    begin
      if 0 = SetWindowLong(ctl.Handle, GWL_EXSTYLE, iExStyle or WS_EX_LAYERED) then
        bUpdateLayeredWindow := False;
    end;

    with bf do
    begin
      BlendOp := AC_SRC_OVER;
      BlendFlags := 0;
      SourceConstantAlpha := 230;
      AlphaFormat :=AC_SRC_ALPHA;
    end;
    ptScreen := Point(ctl.Left, ctl.Top);
    ptMem := Point(0, 0);
    szScreen.cx := ctl.Width;
    szScreen.cy := ctl.Height;

    hDC := BeginPaint(AMsg.hwnd, ps);
    hMemBitmap := CreateCompatibleBitmap(hDC, ps.rcPaint.Right - ps.rcPaint.Left,
      ps.rcPaint.Bottom - ps.rcPaint.Top);
    try
      hMemDC := CreateCompatibleDC(hDC);
      hOldBitmap := SelectObject(hMemDC, hMemBitmap);
      try
        SetWindowOrgEx(hMemDC, ps.rcPaint.Left, ps.rcPaint.Top, nil);
        ctl.Perform(WM_ERASEBKGND, hMemDC, hMemDC);
        ctl.Perform(WM_PAINT, hMemDC, 0);
        if bUpdateLayeredWindow then
          bUpdateSucces := UpdateLayeredWindow(AMsg.hwnd, 0, @ptScreen, @szScreen,
            hMemDC, @ptMem, 0, @bf, ULW_ALPHA)
        else
          bUpdateSucces := BitBlt(hDC, ps.rcPaint.Left, ps.rcPaint.Top,
            ps.rcPaint.Right - ps.rcPaint.Left, ps.rcPaint.Bottom - ps.rcPaint.Top,
            hMemDC, ps.rcPaint.Left, ps.rcPaint.Top,
            SRCCOPY);

        if not bUpdateSucces then
          WriteView('Update Fail %s %d %s',
            [ctl.Name, GetLastError, SysErrorMessage(GetLastError)]);
//        TransparentBlt(hDC, ps.rcPaint.Left, ps.rcPaint.Top,
//          ps.rcPaint.Right - ps.rcPaint.Left, ps.rcPaint.Bottom - ps.rcPaint.Top,
//          hMemDC, ps.rcPaint.Left, ps.rcPaint.Top,
//          ps.rcPaint.Right - ps.rcPaint.Left, ps.rcPaint.Bottom - ps.rcPaint.Top,
//          ColorToRGB(clBtnFace));

        SaveDCToBitmap(ctl, hDC, '.\Pic\', -1);
      finally
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
      end;
    finally
      DeleteObject(hMemBitmap);
      ReleaseDC(AMsg.hwnd, hDC);
    end;

    AHandled := True;
  end;
  procedure DoEraseBackgroundMessage;
  begin
    AHandled := True;
  end;
begin
  case AMsg.message of
    {$IFDEF PACKAGE_COMPILE_MODE}
    WM_KEYFIRST..WM_KEYLAST: DoKeyMessage; //�ڹرհ�ģʽ����������Ҫ�������д���
    {$ENDIF}
//    WM_PAINT: DoPaintMessage;
    WM_ERASEBKGND: DoEraseBackgroundMessage;
  end;
end;

{
procedure TAppInit.DoException(ASender: TObject; AException: Exception);
var
  sil: TJclStackInfoList;
  slst: TStringList;
  str: String;
begin
  slst := nil;
  sil := JclCreateStackList(False, 0, ExceptAddr);
  if not Assigned(sil) then
    Exit;

  try
    slst := TStringList.Create;
    sil.AddToStrings(slst, True);
    str := Format('������Ϣ��'#$D#$A'%s'#$D#$A'����ջ��'#$D#$A'%s', [AException.Message, slst.Text]);
  finally
    FreeAndNil(sil);
    FreeAndNil(slst);
  end;

  WriteView(str);
  Application.MessageBox(PChar(str), '�����������ջ��Ϣ', MB_OK + MB_ICONSTOP);
end;
}

procedure DoJclException(AExceptObj: TObject; AExceptAddr: Pointer; AOSException: Boolean);
var
  slst: TStringList;
  str: String;
begin
  slst := TStringList.Create;
  try
    JclLastExceptStackListToStrings(slst, True);
    if AExceptObj is Exception then
      str := Format('������Ϣ��'#$D#$A'(%s)%s'#$D#$A'����ջ��'#$D#$A'%s',
        [AExceptObj.ClassName, Exception(AExceptObj).Message, slst.Text])
    else
      str := Format('������Ϣ��'#$D#$A'%s'#$D#$A'����ջ��'#$D#$A'%s',
        [AExceptObj.ClassName, slst.Text]);
  finally
    FreeAndNil(slst);
  end;

  WriteView(str);
end;

procedure TAppInit.DoIdle(ASender: TObject; var ADone: Boolean);
begin
  {$IFDEF PACKAGE_COMPILE_MODE}
  if Assigned(FDoIdle) then
    FDoIdle(Application.MainForm);
  {$ENDIF}
end;

function SetDllDirectory(APathName: PAnsiChar): Boolean; stdcall; external 'kernel32.dll' name 'SetDllDirectoryA';

class procedure TAppInit.ExeInit;
var
  hMidas: THandle;
  hEntry: Pointer;
begin
  if Assigned(FAppInit) then
    Exit;

  //1.0 midas.dllע��
  if FileExists('Dll/Common/midas.dll') then
  begin
    hMidas := LoadLibrary('Dll/Common/midas.dll');
    if hMidas <> 0 then
    begin
      hEntry := GetProcAddress(hMidas, 'DllGetClassObject'); //����ľ���ڳ����˳�ʱ���ͷţ���ֹTClientDataSet��������
      DSIntf.RegisterMidasLib(hEntry);
    end;
  end;

  //2.0 ��̬������ݴ���
  FControlAtomList := TStringList.Create;
  FControlAtomList.Sorted := True;

  FAppInit := TAppInit.Create;
  {$IFNDEF LAZARUS}
  Application.OnMessage := FAppInit.DoMessage;
  {$ENDIF}
  Application.OnIdle := FAppInit.DoIdle;

  //Application.OnException����TWinControl.MainWndProc�����except���ֵ��õģ�
  //��ʱ��ջ�Ѿ���UnWind�ˣ��޷���ȷ�Ĳ鿴���쳣����ʱ�ĵ���ջ�����
  //����JclDebug���ṩ��JclStartExceptionTracking����(ʵ�������滻��System.ExceptObjProc��ָ��)��
  //�쳣�ص���������UnWindǰ����ģ��ɱ�֤��ջ�������쳣����ʱ�����
  //Application.OnException := FAppInit.DoException;
  JclStartExceptionTracking;
  JclAddExceptNotifier(DoJclException);

  //3.0 ����������
  FAppInit.CreateForm;
end;

class procedure TAppInit.DllInit;
begin
  {$IFDEF LAZARUS}
  //Lazarus��WidgetSet.AppInit��ͳһ���������࣬Ȼ����ʵ�ʴ�������ʱ��
  //ֱ��ʹ�����ര���࣬�����ظ�������������������ע��ʱ��ִ��ģ�����һ��ģ�
  //�ᵼ����DLL�д����Ĵ�����Ϊδע�ᴰ�����ʧ�ܣ�
  //��ˣ���DLL��ʼ��ʱ����Application�ĳ�ʼ������
  Application.Initialize;
  {$ENDIF}
end;

class procedure TAppInit.Init(AIsLibrary: Boolean);
begin
  {$IFDEF PACKAGE_COMPILE_MODE}
  //Delphi�ڿ������̵߳�����»����ô�ȫ�ֱ��������Ӱ���ڴ������õĺ�����
  //���ò���ȷ�ᵼ�³����쳣��������δ������ģʽ���뿪��ʱ��
  //��ͬDLL�еı���û��ͬ�����£���ˣ��ڴ�ģʽ��Ĭ�ϰ����̷߳�ʽ���У�
  //������Ч��Ϊ���۱�֤����ִ�е���ȷ��
  IsMultiThread := True;
  {$ENDIF}

  if AIsLibrary then
    DllInit
  else
    ExeInit;
end;

class procedure TAppInit.UnInit;
begin
  {$IFNDEF LAZARUS}
  Application.OnMessage := nil;
  {$ENDIF}
  Application.OnIdle := nil;

  FreeAndNil(FControlAtomList);
  FreeAndNil(FAppInit);
end;

class procedure TAppInit.RegCreateForm(ACreateForm: TCreateForm; ADoIdle: TDoIdle);
begin
  FCreateForm := ACreateForm;
  FDoIdle := ADoIdle;
end;

end.
