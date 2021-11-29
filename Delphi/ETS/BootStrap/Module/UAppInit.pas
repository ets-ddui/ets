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
    //设置MainFormOnTaskbar后，主窗口中会包含扩展样式WS_EX_APPWINDOW
    //这个样式的效果是让主窗口在最小化时，触发WM_Size消息(应该和任务栏的操作有关)
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
    //如果窗体是在DLL中实现的，但窗体是在主线程中创建的话，
    //TApplication.ProcessMessage是在主EXE中处理的，
    //FindControl函数取的ControlAtomString是主EXE中存的值，不是DLL中存的值，
    //导致FindControl返回错误的值，这会影响键盘按键消息的处理，
    //导致窗口无法响应左移、右移的键盘操作；
    //这里通过将DLL中存放的属性值，复制一份到EXE对应的Atom属性中，达到修复此问题的目的
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
    WM_KEYFIRST..WM_KEYLAST: DoKeyMessage; //在关闭包模式编译的情况下要开启这行代码
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
    str := Format('错误信息：'#$D#$A'%s'#$D#$A'调用栈：'#$D#$A'%s', [AException.Message, slst.Text]);
  finally
    FreeAndNil(sil);
    FreeAndNil(slst);
  end;

  WriteView(str);
  Application.MessageBox(PChar(str), '程序出错，调用栈信息', MB_OK + MB_ICONSTOP);
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
      str := Format('错误信息：'#$D#$A'(%s)%s'#$D#$A'调用栈：'#$D#$A'%s',
        [AExceptObj.ClassName, Exception(AExceptObj).Message, slst.Text])
    else
      str := Format('错误信息：'#$D#$A'%s'#$D#$A'调用栈：'#$D#$A'%s',
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

  //1.0 midas.dll注册
  if FileExists('Dll/Common/midas.dll') then
  begin
    hMidas := LoadLibrary('Dll/Common/midas.dll');
    if hMidas <> 0 then
    begin
      hEntry := GetProcAddress(hMidas, 'DllGetClassObject'); //这里的句柄在程序退出时不释放，防止TClientDataSet出现问题
      DSIntf.RegisterMidasLib(hEntry);
    end;
  end;

  //2.0 静态编译兼容处理
  FControlAtomList := TStringList.Create;
  FControlAtomList.Sorted := True;

  FAppInit := TAppInit.Create;
  {$IFNDEF LAZARUS}
  Application.OnMessage := FAppInit.DoMessage;
  {$ENDIF}
  Application.OnIdle := FAppInit.DoIdle;

  //Application.OnException是在TWinControl.MainWndProc的外层except部分调用的，
  //这时堆栈已经被UnWind了，无法正确的查看到异常触发时的调用栈情况，
  //改用JclDebug中提供的JclStartExceptionTracking机制(实际上是替换了System.ExceptObjProc的指针)，
  //异常回调处理是在UnWind前处理的，可保证堆栈保留了异常发生时的情况
  //Application.OnException := FAppInit.DoException;
  JclStartExceptionTracking;
  JclAddExceptNotifier(DoJclException);

  //3.0 创建主窗口
  FAppInit.CreateForm;
end;

class procedure TAppInit.DllInit;
begin
  {$IFDEF LAZARUS}
  //Lazarus在WidgetSet.AppInit中统一创建窗口类，然后，在实际创建窗口时，
  //直接使用这类窗口类，不再重复创建，而窗口类是与注册时的执行模块绑定在一起的，
  //会导致在DLL中创建的窗口因为未注册窗口类而失败，
  //因此，在DLL初始化时调用Application的初始化函数
  Application.Initialize;
  {$ENDIF}
end;

class procedure TAppInit.Init(AIsLibrary: Boolean);
begin
  {$IFDEF PACKAGE_COMPILE_MODE}
  //Delphi在开启多线程的情况下会设置此全局变量，其会影响内存分配调用的函数，
  //设置不正确会导致出现异常，由于在未开启包模式编译开关时，
  //不同DLL中的变量没有同步更新，因此，在此模式下默认按多线程方式运行，
  //以牺牲效率为代价保证程序执行的正确性
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
