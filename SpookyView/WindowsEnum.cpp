#include "stdafx.h"
#include "WindowsEnum.h"
#include "strsafe.h"
#include "ISettingsManager.h"
#include <psapi.h>
#include "SpookyView.h"
#include <list>

//http://msdn.microsoft.com/en-us/library/windows/desktop/dd318055(v=vs.85).aspx
const TCHAR DIALOGBOXCLASSNAME[] = _T("#32770");

const TCHAR UWP_APPLICATION_FRAME_WINDOW[] = _T("ApplicationFrameWindow");

const TCHAR UWP_WINDOW_UI_CLASSNAME[] = _T("Windows.UI.Core.CoreWindow");

//Static variables
t_string WindowsEnum::processNameOfWindowsToFind;
std::map<tstring, tstring> WindowsEnum::foundWindowClasses;
TCHAR WindowsEnum::windowClassName[MAX_WINDOW_CLASS_NAME];
TCHAR* WindowsEnum::fileName;
TCHAR WindowsEnum::filePathName[MAX_PATH];
DWORD WindowsEnum::processIdToCheckForUsableWindows;
BOOL WindowsEnum::processHasUsableWindow;
DWORD WindowsEnum::processId;
BOOL WindowsEnum::isUWPProcess;
BOOL WindowsEnum::UWPProcessFound;
std::list<HWND> WindowsEnum::applicationFrameHostWindows;
BOOL WindowsEnum::isMinimizedCoreWindow;

BOOL WindowsEnum::IsPaused()
{
	return isPause;
}

void WindowsEnum::TogglePause()
{
	isPause = !isPause;
	if (isPause) {
		Unhook();
	}
	else {
		CreateHook();
		SetWindowsModifications();
	}
}

BOOL WindowsEnum::HasProcessUsableWindows(DWORD processId)
{
	processIdToCheckForUsableWindows = processId;
	processHasUsableWindow = FALSE;
	EnumDesktopWindows(NULL, EnumProcessHasUsableWindows, 0);
	return processHasUsableWindow;
}

BOOL WindowsEnum::HasProcessUWPCoreWindow(DWORD processId)
{
	BOOL processHasUWPCoreWindow = FALSE;
	isMinimizedCoreWindow = FALSE;
	applicationFrameHostWindows.clear();
	processIdToCheckForUsableWindows = processId;
	EnumDesktopWindows(NULL, EnumGetProcessApplicationFrameHost, 0);
	if (isMinimizedCoreWindow)
	{
		//TODO: This will also find all suspended UWP apps as well.
		return TRUE;
	}
	if (applicationFrameHostWindows.size() > 0)
	{
		for (auto applicationFrameHostWindow : applicationFrameHostWindows)
		{
			HWND coreWindowHwnd = FindWindowEx(applicationFrameHostWindow, NULL, _T("Windows.UI.Core.CoreWindow"), NULL);
			if (coreWindowHwnd != NULL)
			{
				GetWindowThreadProcessId(coreWindowHwnd, &processId);
				if (processId == processIdToCheckForUsableWindows)
				{
					processHasUWPCoreWindow = TRUE;
					break;
				}
			}
		}
	}
	return processHasUWPCoreWindow;
}

std::map<tstring, tstring> WindowsEnum::GetWindowsForProcess(t_string processName)
{
	processNameOfWindowsToFind = processName;
	foundWindowClasses.clear();
	EnumDesktopWindows(NULL, EnumWindowsForProcess, 0);
	return foundWindowClasses;
}

void WindowsEnum::SetWindowsTransparency()
{
	EnumDesktopWindows(NULL, EnumWindowsProc, 0);
}

void WindowsEnum::SetWindowsModifications()
{
	EnumDesktopWindows(NULL, EnumWindowsProc, 1);
}

void WindowsEnum::ResetWindowsModifications()
{
	EnumDesktopWindows(NULL, EnumWindowsReset, 0);
}

BOOL WindowsEnum::IsWindowUsable(HWND hwnd, BOOL includeHidden)
{
	if (GetClassName(hwnd, windowClassName, ARRAYSIZE(windowClassName)))
	{
		LONG_PTR styles = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (GetAncestor(hwnd, GA_PARENT) == GetDesktopWindow() && (includeHidden || IsWindowVisible(hwnd)) && ((styles & (WS_POPUP | WS_CHILD)) == 0 || (styles & WS_DLGFRAME) || _tcscmp(windowClassName, DIALOGBOXCLASSNAME) == 0 || _tcscmp(windowClassName, UWP_APPLICATION_FRAME_WINDOW) == 0))
		{
			//This is a top-level window that is not hidden and not a pop-up window or a pop-up windows that is a dialog
			return TRUE;
		}
	}
	return FALSE;
}

BOOL WindowsEnum::IsMaximized(HWND hwnd)
{
	WINDOWPLACEMENT placement;
	placement.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hwnd, &placement);
	return placement.showCmd == SW_SHOWMAXIMIZED;
}

void CALLBACK WindowsEnum::WinEventProcMinimizeChange(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (idObject == OBJID_WINDOW && IsWindowUsable(hwnd))
	{
		//This is a top-level window, enumerate all windows
		SetWindowsTransparency();
	}
}

void CALLBACK WindowsEnum::WinEventProcForegroundChange(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (idObject == OBJID_WINDOW)
	{
		//When the foreground window has changed, always reset transparency values
		SetWindowsTransparency();
	}
}

void CALLBACK WindowsEnum::WinEventProcShow(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	//When a window is set to show, check if it is a usable window.
	// If so, check and set always on top value
	if (idObject == OBJID_WINDOW && IsWindowUsable(hwnd))
	{
		CheckAndSetWindowAlwaysOnTop(hwnd);
		SetWindowsTransparency();
	}
}

void WindowsEnum::CheckAndSetWindowAlwaysOnTop(HWND hwnd)
{
	auto modificationSettings = GetWindowModificationSettings(hwnd);
	if (modificationSettings && modificationSettings->alwaysOnTop)
	{
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	}
}

BOOL CALLBACK WindowsEnum::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd)){
		if (!IsIconic(hwnd))
		{
			if (GetForegroundWindow() == hwnd)
			{
				SetWindowAlpha(hwnd, CSettings::WindowTypes::Foregound);
			}
			else
			{
				SetWindowAlpha(hwnd, CSettings::WindowTypes::Background);
			}
		}
		if (lParam == 1)
		{
			CheckAndSetWindowAlwaysOnTop(hwnd);
		}
	}
	return TRUE;
}

BOOL CALLBACK WindowsEnum::EnumWindowsReset(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd, TRUE))
	{
		auto modificationSettings = GetWindowModificationSettings(hwnd);
		//Only reset windows changed by our app
		if (modificationSettings)
		{
			//Check if window is transparent
			if ((GetWindowLongPtr(hwnd, GWL_STYLE) & WS_EX_LAYERED))
			{
				if (modificationSettings->enabled)
				{
					//Window is now transparent. Remove transparency
					//https://docs.microsoft.com/en-us/windows/win32/winmsg/using-windows#using-layered-windows
					SetWindowLongPtr(hwnd, GWL_EXSTYLE, (GetWindowLongPtr(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED));
					SetLayeredWindowAttributes(hwnd, 0, 255, LWA_ALPHA);
					RedrawWindow(hwnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_FRAME | RDW_ALLCHILDREN);
				}
			}
			//Check if window is always-on-top
			if ((GetWindowLongPtr(hwnd, GWL_EXSTYLE) & WS_EX_TOPMOST))
			{
				if (modificationSettings->alwaysOnTop)
				{
					//Window is always-on-top (most likely by our app), set as non top window
					SetWindowPos(hwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				}
			}
		}
	}
	return TRUE;
}

BOOL CALLBACK WindowsEnum::EnumWindowsForProcess(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd))
	{
		GetWindowProcessAndClass(hwnd);
		if (_tcsicmp(fileName, processNameOfWindowsToFind.c_str()) == 0)
		{
			auto windowClassNameCopy = std::make_unique<TCHAR[]>(MAX_WINDOW_CLASS_NAME);
			StringCchCopy(windowClassNameCopy.get(), MAX_WINDOW_CLASS_NAME, windowClassName);

			auto textLength = GetWindowTextLength(hwnd);
			auto textBuffer = std::make_unique<TCHAR[]>(textLength + 1);
			GetWindowText(hwnd, textBuffer.get(), textLength + 1);
			foundWindowClasses.insert(std::pair<tstring, tstring>(tstring(windowClassNameCopy.get()), tstring(textBuffer.get())));
		}
	}
	return TRUE;
}

BOOL CALLBACK WindowsEnum::EnumGetProcessApplicationFrameHost(HWND hwnd, LPARAM lParam)
{
	TCHAR currentWindowClassName[MAX_WINDOW_CLASS_NAME];
	if (GetClassName(hwnd, currentWindowClassName, ARRAYSIZE(currentWindowClassName)))
	{
		if (_tcsicmp(currentWindowClassName, UWP_WINDOW_UI_CLASSNAME) == 0)
		{
			//We have found a minimized CoreWindow. When minimized, a CoreWindows will be a top window of the desktop with the class "Windows.UI.Core.CoreWindow".
			//TODO: This is also for suspended UWP apps, even without a current window.
			isMinimizedCoreWindow = TRUE;
			return TRUE;
		}
		if (_tcsicmp(currentWindowClassName, UWP_APPLICATION_FRAME_WINDOW) == 0)
		{
			applicationFrameHostWindows.push_back(hwnd);
		}
	}
	return TRUE;
}

BOOL CALLBACK WindowsEnum::EnumUWPChildWindows(HWND hwnd, LPARAM lParam)
{
	//Store the process and class belonging to given hwnd in static variables.
	if (GetWindowProcessAndClass(hwnd))
	{
		if (_tcsicmp(windowClassName, UWP_WINDOW_UI_CLASSNAME) == 0)
		{
			//We have the application belonging to the UWP window.
			UWPProcessFound = TRUE;
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CALLBACK WindowsEnum::EnumProcessHasUsableWindows(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd))
	{
		DWORD windowProcessId = 0;
		GetWindowThreadProcessId(hwnd, &windowProcessId);
		if (windowProcessId != 0 && windowProcessId == processIdToCheckForUsableWindows)
		{
			processHasUsableWindow = TRUE;
			return FALSE; //Stop enumeration. We have found a usable window in the process
		}
	}
	return TRUE; //Continue enumeration
}

void WindowsEnum::CheckAndSetUWPProcessAndClass(HWND hwnd)
{
	if (_tcsicmp(windowClassName, UWP_APPLICATION_FRAME_WINDOW) == 0)
	{
		isUWPProcess = TRUE;
		UWPProcessFound = FALSE;
		//We have a UWP app. Check child window for process name. Store the processId belonging to the window
		EnumChildWindows(hwnd, EnumUWPChildWindows, 0);
	}
	else
	{
		isUWPProcess = FALSE;
	}
}

CModificationSettings* WindowsEnum::GetWindowModificationSettings(HWND hwnd)
{
	if (GetWindowProcessAndClass(hwnd)) {
		CheckAndSetUWPProcessAndClass(hwnd);
		if (!isUWPProcess || UWPProcessFound) {
			return settingsManager->GetSettings()->GetModificationSetting(fileName, windowClassName);
		}
	}
	return NULL;
}

void WindowsEnum::SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType)
{
	auto modificationSettings = GetWindowModificationSettings(hwnd);

	if (modificationSettings && modificationSettings->enabled)
	{
		BYTE alpha;
		switch (windowType)
		{
		case CSettings::WindowTypes::Foregound:
			alpha = modificationSettings->foreground;
			break;

		case CSettings::WindowTypes::Background:
			alpha = modificationSettings->separateBackgroundValue ? modificationSettings->background : modificationSettings->foreground;
			break;
		}
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, (GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED));
		SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
	}
}

BOOL WindowsEnum::GetWindowProcessAndClass(HWND hwnd)
{
	BOOL result = FALSE;
	GetWindowThreadProcessId(hwnd, &processId);
	if (processId == GetCurrentProcessId())
	{
		return FALSE;
	}
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
	if (hProcess != NULL)
	{
		//Check if current process is not a Windows Store App on Windows 8 or Windows 8.1.
		//They should not be transparent because they are always running full-screen
		if (!isWindows8 || isImmersive == NULL || !isImmersive(hProcess))
		{
			//Get process image file name
			GetProcessImageFileName(hProcess, filePathName, ARRAYSIZE(filePathName));
			fileName = _tcsrchr(filePathName, '\\');
			fileName++;

			//Output debug data
#ifdef _DEBUG
			OutputDebugString(fileName);
			OutputDebugString(_T("  --  "));
			OutputDebugString(windowClassName);
			OutputDebugString(_T("\r\n"));
#endif

			if (GetClassName(hwnd, windowClassName, ARRAYSIZE(windowClassName)))
			{
				result = TRUE;
			}
		}
		CloseHandle(hProcess);
	}
	return result;
}

/*
Create the hook for capturing the events
*/
void WindowsEnum::CreateHook()
{
	//EVENT_OBJECT_SHOW is needed for UWP apps and to check if a window has been created
	hWinEventHook[0] = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, NULL, WinEventProcShow, 0, 0, WINEVENT_OUTOFCONTEXT);
	hWinEventHook[1] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProcForegroundChange, 0, 0, WINEVENT_OUTOFCONTEXT);
	hWinEventHook[2] = SetWinEventHook(EVENT_SYSTEM_MINIMIZESTART, EVENT_SYSTEM_MINIMIZEEND, NULL, WinEventProcMinimizeChange, 0, 0, WINEVENT_OUTOFCONTEXT);
}

void WindowsEnum::Unhook()
{
	for (HWINEVENTHOOK hook : hWinEventHook)
	{
		UnhookWinEvent(hook);
	}
}