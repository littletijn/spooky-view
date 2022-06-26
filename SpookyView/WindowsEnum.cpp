#include "stdafx.h"
#include "WindowsEnum.h"
#include "strsafe.h"
#include "ISettingsManager.h"
#include <psapi.h>
#include "SpookyView.h"

extern std::unique_ptr<ISettingsManager> settingsManager;
extern PGNSI isImmersive;

//http://msdn.microsoft.com/en-us/library/windows/desktop/dd318055(v=vs.85).aspx
const TCHAR DIALOGBOXCLASSNAME[7] = _T("#32770");

//Static variables
BOOL WindowsEnum::isWindows8;
t_string WindowsEnum::processNameOfWindowsToFind;
std::map<tstring, tstring> WindowsEnum::foundWindowClasses;
TCHAR WindowsEnum::windowClassName[MAX_WINDOW_CLASS_NAME];
TCHAR* WindowsEnum::fileName;
TCHAR WindowsEnum::filePathName[MAX_PATH];

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
		SetWindowsTransparency();
	}
}

void WindowsEnum::SetWindowsTransparency()
{
	EnumWindows(EnumWindowsProc, NULL);
}

void WindowsEnum::ResetWindowsTransparency()
{
	EnumWindows(EnumWindowsReset, NULL);
}

BOOL WindowsEnum::IsWindowUsable(HWND hwnd)
{
	if (GetClassName(hwnd, windowClassName, ARRAYSIZE(windowClassName)))
	{
		LONG_PTR styles = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (GetAncestor(hwnd, GA_PARENT) == GetDesktopWindow() && (styles & WS_VISIBLE) && (!(styles & WS_POPUP) || _tcscmp(windowClassName, DIALOGBOXCLASSNAME) == 0))
		{
			//This is a top-level window that is not hidden and not a pop-up window or a pop-up windows that is a dialog
			return TRUE;
		}
	}
	return FALSE;
}

void CALLBACK WindowsEnum::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (IsWindowUsable(hwnd) && idObject == OBJID_WINDOW)
	{
		//This is a top-level window, enumerate all windows
		OutputDebugString(_T("Enuming...\r\n"));
		SetWindowsTransparency();
		OutputDebugString(_T("-------------------------------------\r\n"));
	}
}

void CALLBACK WindowsEnum::WinEventProcForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	//When the foreground window has changed, always reset transparency values
	SetWindowsTransparency();
}

BOOL CALLBACK WindowsEnum::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd) && IsWindowVisible(hwnd) && !IsIconic(hwnd))
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
	return TRUE;
}

BOOL CALLBACK WindowsEnum::EnumWindowsReset(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd))
	{
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, (GetWindowLongPtr(hwnd, GWL_EXSTYLE) ^ WS_EX_LAYERED));
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

void WindowsEnum::SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType)
{
	if (GetWindowProcessAndClass(hwnd)) {
		BYTE alpha;
		if (settingsManager->GetSettings()->GetAlphaSetting(fileName, windowClassName, windowType, alpha))
		{
			SetWindowLongPtr(hwnd, GWL_EXSTYLE, (GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED));
			SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
		}
	}
}

BOOL WindowsEnum::GetWindowProcessAndClass(HWND hwnd)
{
	BOOL result = FALSE;
	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
	if (hProcess != NULL)
	{
		//Check if current process is not a Windows Store App on Windows 8 or Windows 8.1.
		//They should not be transparent because they are always running full-screen
		if (!WindowsEnum::isWindows8 || isImmersive == NULL || !isImmersive(hProcess))
		{
			//Get process image file name
			GetProcessImageFileName(hProcess, filePathName, ARRAYSIZE(filePathName));
			fileName = _tcsrchr(filePathName, '\\');
			fileName++;

			//Output debug data
			OutputDebugString(fileName);
			OutputDebugString(_T("  --  "));
			OutputDebugString(windowClassName);
			OutputDebugString(_T("\r\n"));

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
	/*EVENT_SYSTEM_FOREGROUND*/
	/*EVENT_OBJECT_SHOW*/
	/*EVENT_SYSTEM_MINIMIZEEND*/
	hWinEventHook[0] = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	hWinEventHook[1] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProcForeground, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	hWinEventHook[2] = SetWinEventHook(EVENT_SYSTEM_MINIMIZEEND, EVENT_SYSTEM_MINIMIZEEND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}

void WindowsEnum::Unhook()
{
	for (HWINEVENTHOOK hook : hWinEventHook)
	{
		UnhookWinEvent(hook);
	}
}

void WindowsEnum::GetIsWindows8()
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	// Initialize the OSVERSIONINFOEX structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 6;
	osvi.dwMinorVersion = 2;
	osvi.wServicePackMajor = 0;
	osvi.wServicePackMinor = 0;

	// Initialize the condition mask.

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

	// Perform the test.

	WindowsEnum::isWindows8 = VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		dwlConditionMask
	);
	OutputDebugString(WindowsEnum::isWindows8 ? _T("Running on Windows 8 or 8.1\r\n") : _T("Not running on Windows 8 or 8.1\r\n"));
}