// winmain.cxx
#include "mainwin.h"

// global-variables //////////////////////////////////////////////////////////////////////
HINSTANCE hAppInstance;
char homeFolderName[MAX_PATH];
CRITICAL_SECTION c_section;

//////////////////////////////////////////////////////////////////////////////////////////
LONG WINAPI ExceptionFilter(EXCEPTION_POINTERS *ExceptionInfo)
{
  SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);

  char mes[256];
  sprintf(mes, "Unhandled error occurred.(address:0x%08x)",
          PtrToUlong(ExceptionInfo->ExceptionRecord->ExceptionAddress));
  MessageBox(NULL, mes, "ExceptionError", MB_OK|MB_TOPMOST|MB_APPLMODAL);

  return EXCEPTION_EXECUTE_HANDLER;
}

//////////////////////////////////////////////////////////////////////////////////////////
#ifdef USE_WPF
[System::STAThreadAttribute]
#endif
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
  // 2重起動防止処理
  HANDLE hmutex;
  {
    SECURITY_DESCRIPTOR sd;
    InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(&sd, TRUE, 0, FALSE);

    SECURITY_ATTRIBUTES secAttribute;
    secAttribute.nLength=sizeof(SECURITY_ATTRIBUTES);
    secAttribute.lpSecurityDescriptor=&sd;
    secAttribute.bInheritHandle=TRUE;
    hmutex=CreateMutex(&secAttribute, FALSE, MAINWIN_CLASS_NAME);

    if((hmutex==NULL)||(GetLastError()==ERROR_ALREADY_EXISTS)){
      HWND hwnd_fs=FindWindow(MAINWIN_CLASS_NAME, NULL);
      if(hwnd_fs!=NULL){
        ShowWindow(hwnd_fs, SW_SHOWNORMAL);
        SetForegroundWindow(hwnd_fs);
        SetActiveWindow(hwnd_fs);
      }else{
        hwnd_fs=FindWindow(NULL, MAINWIN_WINDOW_NAME);
        if(hwnd_fs!=NULL){
          SetForegroundWindow(hwnd_fs);
          SetActiveWindow(hwnd_fs);
          SetWindowPos(hwnd_fs, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
        }
      }
      return 0;
    }
  }

  // グローバル変数設定
  hAppInstance=hInstance;
  GetModuleFileName(NULL, homeFolderName, MAX_PATH); PathRemoveFileSpec(homeFolderName);
  InitializeCriticalSection(&c_section);

  // 例外ハンドラの登録
  HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);
  SetUnhandledExceptionFilter(ExceptionFilter);

  // コモンライブラリ初期化
  INITCOMMONCONTROLSEX icex;
  icex.dwSize=sizeof(INITCOMMONCONTROLSEX); icex.dwICC=ICC_COOL_CLASSES|ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icex);
  HMODULE hre_dll=LoadLibrary("RichEd32.dll");

  // 各種初期化処理
  timeBeginPeriod(1);
  
  mainWin *mainwin=new mainWin();
  mainwin->Initialize();
  mainwin->Show();
  mainwin->RunMessageLoop();
  mainwin->Uninitialize();
  delete mainwin;

  // 各種終了処理
  timeEndPeriod(1);
  SystemParametersInfo(SPI_SETCURSORS, 0, NULL, 0);

  if(hre_dll!=NULL) FreeLibrary(hre_dll);

  DeleteCriticalSection(&c_section);
  CloseHandle(hmutex);

  return 0;
}
