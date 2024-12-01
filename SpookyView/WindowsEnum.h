#ifndef WINDOWSENUM_H
#define WINDOWSENUM_H

#include "MultiPlatformString.h"
#include <map>
#include "Unicode.h"
#include "CSettings.h"
#include "Defines.h"
#include <list>
class WindowsEnum
{
public:
	//Other static variables
	BOOL isPause = false;

	//Callbacks
	static void CALLBACK WinEventProcMinimizeChange(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static void CALLBACK WinEventProcForegroundChange(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static void CALLBACK WinEventProcShow(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);
	static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumWindowsReset(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumWindowsForProcess(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumGetProcessApplicationFrameHost(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumUWPChildWindows(HWND hwnd, LPARAM lParam);
	static BOOL CALLBACK EnumProcessHasUsableWindows(HWND hwnd, LPARAM lParam);
	static BOOL IsMaximized(HWND hwnd);

	//Static functions
	static void CheckAndSetWindowAlwaysOnTop(HWND hwnd);
	static void SetWindowsTransparency();
	static void SetWindowsModifications();
	static std::map<tstring, tstring> GetWindowsForProcess(t_string processName);
	static BOOL HasProcessUsableWindows(DWORD processId);
	static BOOL HasProcessUWPCoreWindow(DWORD processId);

	//Functions
	void CreateHook();
	void Unhook();
	BOOL IsPaused();
	void ResetWindowsModifications();
	void TogglePause();
protected:
	HWINEVENTHOOK hWinEventHook[3];

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
	static BOOL IsWindowUsable(HWND hwnd, BOOL includeHidden = FALSE);
	static void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType);
	static void CheckAndSetUWPProcessAndClass(HWND hwnd);
	static CModificationSettings* GetWindowModificationSettings(HWND hwnd);
};

#endif