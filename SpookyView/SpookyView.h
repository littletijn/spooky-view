#include "CSettings.h"
#include <list>
#include <unordered_set>

//Globals variables
extern t_string processNameOfWindowsToFind;
extern std::unordered_set<TCHAR*> foundWindowClasses;

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
void SetWindowsTransparency();
void ResetWindowsTransparency();
void TogglePause();
void SendAlreadyRunningNotify();
void GetIsWindows8();