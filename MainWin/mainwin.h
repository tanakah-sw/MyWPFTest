// mainwin.h
#include "config.h"
#include "common.h"

// optional headers /////////////////////////////////////////////////////////////////////////
// COM
#ifdef USE_COM
#include <comutil.h>
#include <objbase.h>
#pragma comment(lib, "comsupp.lib")
#pragma comment(lib, "comsuppw.lib")
#endif

// Direct2D
#ifdef USE_DIRECT2D
#include <d2d1.h>
#include <d2d1helper.h>
#pragma comment(lib, "d2d1.lib")
//#include <dwrite.h>
//#pragma comment(lib, "dwrite.lib")

//#include <wincodec.h>
//#pragma comment(lib, "windowscodecs.lib")
#endif

/*
// DirectInput
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
*/

// GDI+
#ifdef USE_GDIPLUS
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")
using namespace Gdiplus;
#endif

// global-variables
extern HINSTANCE hAppInstance;
extern CRITICAL_SECTION c_section;
extern char homeFolderName[MAX_PATH];

// constant-defines
#define MAINWIN_CLASS_NAME  "mymainwinclass"
#define MAINWIN_WINDOW_NAME "mymainwin"

class mainWin
{
public:
  mainWin();
  ~mainWin();
  void Initialize();
  void Show();
  void RunMessageLoop();
  void Uninitialize();

public:
  HWND m_hwnd;

private:
  // wnd/dlg proc
  HWND m_hdlg, m_hdlgWpf;

  static LRESULT CALLBACK wndProcMain(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
  static LRESULT CALLBACK dlgProcMain(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam);

  void RegisterClass();
  void UnregisterClass();
  
#ifdef USE_WPF
  bool WpfUncheck();
#endif

#ifdef USE_WPF
  IDispatch *pDisWpf;
  IUnknown *pUnkWpf;
#endif
};
