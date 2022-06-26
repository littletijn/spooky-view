#include "String.h"
#include <map>
#include "Unicode.h"
#include "CSettings.h"
#include "Defines.h"
#pragma once

class WindowsEnum
{
public:
	HWINEVENTHOOK hWinEventHook[3];

	//Static variables for EnumWindowsForProcess
	static t_string processNameOfWindowsToFind;
	static std::map<tstring, tstring> foundWindowClasses;
	static TCHAR windowClassName[MAX_WINDOW_CLASS_NAME];
	static TCHAR* fileName;
	//Buffer for complete path of a file
	static TCHAR filePathName[MAX_PATH];

	//Other static variables
	static BOOL isWindows8;
	BOOL isPause = false;

	//Callbacks
	static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumWindowsReset(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumWindowsForProcess(HWND hwnd, LPARAM lParam);
	static void CALLBACK WinEventProcForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

	//Static functions
	static BOOL IsWindowUsable(HWND hwnd);
	static void SetWindowsTransparency();
	static void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType);
	static BOOL GetWindowProcessAndClass(HWND hwnd);

	//Functions
	void GetIsWindows8();
	void CreateHook();
	void Unhook();
	BOOL IsPaused();
	void ResetWindowsTransparency();
	void TogglePause();

protected:
};

