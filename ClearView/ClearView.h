#include "resource.h"
#include "CSettings.h"

//Callbacks
void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
BOOL CALLBACK EnumWindowsReset(HWND hwnd, LPARAM lParam);

//Functions
void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType);
void LoadFunctionAdresses();
void CreateHook();
void Unhook();
BOOL IsPaused();
void TogglePause();
void ExitProgram();