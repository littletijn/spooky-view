#include "String.h"
#include <map>
#include "Unicode.h"
#include "CSettings.h"
#include "Defines.h"
#include <list>
#pragma once

class WindowsEnum
{
public:
	//Other static variables
	static BOOL isWindows8;
	BOOL isPause = false;

	//Callbacks
	static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static void CALLBACK WinEventProcForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumWindowsReset(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumWindowsForProcess(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumGetProcessApplicationFrameHost(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumUWPChildWindows(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumProcessHasUsableWindows(HWND hwnd, LPARAM lParam);

	//Static functions
	static void SetWindowsTransparency();
	static std::map<tstring, tstring> GetWindowsForProcess(t_string processName);
	static BOOL HasProcessUsableWindows(DWORD processId);
	static BOOL HasProcessUWPCoreWindow(DWORD processId);

	//Functions
	void GetIsWindows8();
	void CreateHook();
	void Unhook();
	BOOL IsPaused();
	void ResetWindowsTransparency();
	void TogglePause();
protected:
	HWINEVENTHOOK hWinEventHook[4];

	//Static variables for EnumWindowsForProcess
	static t_string processNameOfWindowsToFind;
	static std::map<tstring, tstring> foundWindowClasses;

	//Static variables for GetWindowProcessAndClass
	static TCHAR windowClassName[MAX_WINDOW_CLASS_NAME];
	static TCHAR filePathName[MAX_PATH]; //Buffer for complete path of a file
	static TCHAR* fileName; //Buffer for filename only
	static DWORD processId;

	//Static variables for EnumProcessHasUsableWindows
	static DWORD processIdToCheckForUsableWindows;
	static BOOL processHasUsableWindow;

	//Static variables for EnumUWPChildWindows
	static BOOL isUWPProcess;
	static BOOL UWPProcessFound;

	//Static variables for EnumGetProcessApplicationFrameHost
	static std::list<HWND> applicationFrameHostWindows;
	static BOOL isMinimizedCoreWindow;

	//Static functions
	static BOOL GetWindowProcessAndClass(HWND hwnd);
	static BOOL IsWindowUsable(HWND hwnd);
	static void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType);
	static void CheckAndSetUWPProcessAndClass(HWND hwnd);
};

