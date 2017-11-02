// mainwin.cxx
#include "mainwin.h"
#include "resource.h"

// GDIリソース //////////////////////////////////////////////////////////////////////////////
//// font
HFONT  hfont=NULL;
//// pen
HPEN   hpen=NULL;
//// brush
HBRUSH hbrush=NULL;

// static variables /////////////////////////////////////////////////////////////////////////

// WindowRegist関連 /////////////////////////////////////////////////////////////////////////
void mainWin::RegisterClass()
{
  WNDCLASSEX wc_main;

  memset(&wc_main, 0, sizeof(WNDCLASSEX));
  wc_main.cbSize=sizeof(WNDCLASSEX);
  wc_main.style=CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
  wc_main.lpfnWndProc=(WNDPROC)(mainWin::wndProcMain);
  wc_main.cbClsExtra=wc_main.cbWndExtra=0;
  wc_main.hInstance=hAppInstance;
  wc_main.hIcon=LoadIcon(hAppInstance, IDI_APPLICATION);
  wc_main.hIconSm=LoadIcon(hAppInstance, IDI_APPLICATION);
  wc_main.hCursor=LoadCursor(NULL, IDC_ARROW);

  //  wc_main.hbrBackground=(HBRUSH)(NULL_BRUSH);
  //  wc_main.hbrBackground=(HBRUSH)(BLACK_BRUSH);
  wc_main.hbrBackground=(HBRUSH)(WHITE_BRUSH);

  wc_main.lpszMenuName=NULL;
  //  wc_main.lpszMenuName=MAKEINTRESOURCE(IDR_MENU);
  
  wc_main.lpszClassName=TEXT(MAINWIN_CLASS_NAME);
  if(!::RegisterClassEx(&wc_main)) PostQuitMessage(0);
}  

void mainWin::UnregisterClass()
{
  ::UnregisterClass(TEXT(MAINWIN_CLASS_NAME), hAppInstance);
}

// MesasgeLoop ////////////////////////////////////////////////////////////////////////////////
void mainWin::RunMessageLoop()
{
  MSG msg;
  bool done=false;

  while(!done){
    if(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)){
      if(msg.message==WM_QUIT){
        done=true;
      }else{
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
}

void mainWin::Show()
{
  ShowWindow(m_hdlg, SW_SHOW);
  ShowWindow(m_hwnd, SW_SHOWNORMAL);
  UpdateWindow(m_hwnd);
}

void mainWin::Initialize()
{
  RegisterClass();

  m_hwnd=CreateWindow(TEXT(MAINWIN_CLASS_NAME), TEXT(MAINWIN_WINDOW_NAME),
                      WS_OVERLAPPED|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
                      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
                      NULL, NULL, hAppInstance, (LPVOID)this);
  m_hdlg=CreateDialogParam(hAppInstance, MAKEINTRESOURCE(IDD_CONTROL), m_hwnd,
                           (DLGPROC)dlgProcMain, (LONG_PTR)this);

  // dlg adjust size
  RECT drc;
  GetClientRect(m_hdlg, &drc);
  MoveWindow(m_hdlg, 0, 0, drc.right-drc.left, drc.bottom-drc.top, TRUE);

  RECT wrc;
  GetWindowRect(m_hwnd, &wrc);
  int xp=wrc.left, yp=wrc.top;
  SetRect(&wrc, 0, 0, drc.right-drc.left, drc.bottom-drc.top);
  DWORD style=(DWORD)GetWindowLongPtr(m_hwnd, GWL_STYLE);
  AdjustWindowRect(&wrc, style, FALSE); // AdjustWindowRect(&wrc, style, GetMenu(m_hwnd));
  SetWindowPos(m_hwnd, HWND_NOTOPMOST, xp, yp, wrc.right-wrc.left, wrc.bottom-wrc.top,
               SWP_NOZORDER| SWP_NOACTIVATE);
}

void mainWin::Uninitialize()
{
  UnregisterClass();
}

#define DLGCOLOR_R 250
#define DLGCOLOR_G 250
#define DLGCOLOR_B 250

mainWin::mainWin()
{
  hbrush=CreateSolidBrush(RGB(DLGCOLOR_R, DLGCOLOR_G, DLGCOLOR_B)); //GetSysColor(COLOR_BTNFACE));

  m_hwnd=NULL;
  m_hdlg=NULL;
  m_hdlgWpf=NULL;

#ifdef USE_COM
  CoInitialize(NULL);
#endif
#ifdef USE_WPF
  // regist wfp.dll >> c:\Windows\Microsoft.NET\Framework\v4.0.30319\RegAsm.exe wfp.dll
  HRESULT hresult;
  CLSID clsid;
  hresult=CLSIDFromProgID(L"WPFPage.InteropTest", &clsid);
  hresult=CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void **)&pUnkWpf);
  hresult=pUnkWpf->QueryInterface(IID_IDispatch, (void **)&pDisWpf);
#endif
}

mainWin::~mainWin()
{
#ifdef USE_WPF
  if(pDisWpf!=NULL) pDisWpf->Release();
  if(pUnkWpf!=NULL) pUnkWpf->Release();
#endif
#ifdef USE_COM
  CoUninitialize();
#endif
  if(hpen!=NULL)   DeleteObject(hpen);
  if(hbrush!=NULL) DeleteObject(hbrush);
  if(hfont!=NULL)  DeleteObject(hfont);
}

#ifdef USE_WPF

bool mainWin::WpfUncheck()
{
  HRESULT hresult;

  LPOLESTR funcname=L"UnCheck";
  DISPID dispid=0;
  hresult=pDisWpf->GetIDsOfNames(IID_NULL, &funcname, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
  if(FAILED(hresult)) return false;
  
  DISPPARAMS params;
  memset(&params, 0, sizeof(DISPPARAMS));
  params.cArgs=0;
  params.cNamedArgs=0;
  params.rgdispidNamedArgs=NULL;
  params.rgvarg = NULL;
  
  VARIANT vRet;
  VariantInit(&vRet);
  
  hresult=pDisWpf->Invoke(dispid, IID_NULL, LOCALE_SYSTEM_DEFAULT, DISPATCH_METHOD, &params, &vRet, NULL, NULL);

  if(FAILED(hresult)) return false;
  return true;
}

#endif

#ifdef USE_WPF
namespace ManagedCode
{
  using namespace System;
  using namespace System::Windows;
  using namespace System::Windows::Interop;

  HWND p_hwnd;
  
  IntPtr ChildHwndSourceHook(IntPtr, int msg, IntPtr wparam, IntPtr lparam, bool% handled)
  {
    if(msg==WM_COMMAND){
      ATOM a=(ATOM)((int)lparam);
      wchar_t wbuf[256];
      GetAtomNameW(a, wbuf, 256);
      DeleteAtom(a);

      char buf[256];
      WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wbuf, -1, buf, 256, NULL, NULL);

      SetDlgItemText(p_hwnd, IDC_STATIC_MESSAGE, buf);
    }
    return IntPtr(0);
  }

  HWND GetHwnd(HWND parent, int x, int y, int width, int height)
  {
    p_hwnd=parent;
    HwndSourceParameters params;
    params.PositionX=x;
    params.PositionY=y;
    params.Width=width;
    params.Height=height;
    params.ParentWindow=IntPtr(parent);
    params.WindowStyle=WS_VISIBLE| WS_CHILD;
    
    HwndSource^ source=gcnew HwndSource(params);
    source->AddHook(gcnew HwndSourceHook(ChildHwndSourceHook));

    UIElement^ page=gcnew WPFPage::TestPanel(true);
    source->RootVisual=page;

    return (HWND)source->Handle.ToPointer();
  }
}
#endif


LRESULT CALLBACK mainWin::wndProcMain(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static mainWin *_this=NULL;
  switch(msg){
  case WM_CREATE:
    _this=(mainWin *)(((CREATESTRUCT *)lparam)->lpCreateParams);
    break;
  case WM_CLOSE:
    DestroyWindow(hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  default:
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }
  return 0;
}

LRESULT CALLBACK mainWin::dlgProcMain(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static mainWin *_this=NULL;
  switch(msg){
  case WM_CTLCOLORSTATIC:
  case WM_CTLCOLORDLG:
    {
      HDC  hdc_ctrl=(HDC)wparam;
      HWND hwnd_ctrl=(HWND)lparam;
      return (LRESULT)hbrush;
    }
    break;

  case WM_COMMAND:
    {
      switch(LOWORD(wparam)){
      case IDC_BUTTON_UNCHECK:
        _this->WpfUncheck();
        return TRUE;
        break;
      case IDC_BUTTON_QUIT:
        DestroyWindow(_this->m_hwnd);
        return TRUE;
        break;
      }
    }
    break;

  case WM_INITDIALOG:
    {
      _this=(mainWin *)lparam;

      SetDlgItemText(hdlg, IDC_STATIC_MESSAGE, "");
      
#ifdef USE_WPF
      RECT rc_wpf;
      GetWindowRect(GetDlgItem(hdlg, IDC_STATIC_WPF), &rc_wpf);

      std::vector<POINT> p;
      p.resize(1);
      p[0].x=rc_wpf.left;
      p[0].y=rc_wpf.top;
      MapWindowPoints(NULL, hdlg, &p[0], p.size());

      int width, height;
      width=rc_wpf.right-rc_wpf.left;
      height=rc_wpf.bottom-rc_wpf.top;
      _this->m_hdlgWpf=ManagedCode::GetHwnd(hdlg, p[0].x, p[0].y, width, height);
#endif
      return TRUE;
    }
    break;
  }
  return FALSE;
}
