#include "CSettings.h"
#include <list>

//Globals variables
extern t_string processNameOfWindowsToFind;
extern std::list<TCHAR*> foundWindowClasses;

//Callbacks
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK EnumWindowsReset(HWND hwnd, LPARAM lParam);
BOOL CALLBACK EnumWindowsForProcess(HWND hwnd, LPARAM lParam);

//Functions
void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType);
BOOL GetWindowProcessAndClass(HWND hwnd);
void LoadFunctionAdresses();
void CreateHook();
void Unhook();
BOOL IsPaused();
void TogglePause();
void ExitProgram();
void SendAlreadyRunningNotify();