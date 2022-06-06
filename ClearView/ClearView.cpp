// ClearView.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "ClearView.h"
#include "CMainWindow.h"
#include "ISettingsManager.h"
#include "CRegistrySettingsManager.h"
#include <Psapi.h>
#include <Uxtheme.h>
#include <string>
#include "CLimitSingleInstance.h"
#include <memory>
#include <list>
#include <strsafe.h>
#include "Defines.h"
#include "UpdateChecker.h"
#include "UpdateResponse.h"

typedef BOOL (WINAPI *PGNSI)(HANDLE);
typedef BOOL (WINAPI *PGNSI2)(HWND, MARGINS*);

//http://msdn.microsoft.com/en-us/library/windows/desktop/dd318055(v=vs.85).aspx
const TCHAR DIALOGBOXCLASSNAME[7] = _T("#32770");

// Global Variables:
HINSTANCE hInst;								// current instance
std::unique_ptr<CMainWindow> mainWindow;
HWINEVENTHOOK hWinEventHook[3];
PGNSI isImmersive;
std::unique_ptr<ISettingsManager> settingsManager;
BOOL isPause = false;
CLimitSingleInstance singleInstanceObj(_T("ClearView"));
UpdateResponse updateResponse;
BOOL isWindows8;

TCHAR windowClassName[MAX_WINDOW_CLASS_NAME];
TCHAR* fileName;
//Buffer for complete path of a file
TCHAR filePathName[MAX_PATH];

//Global variables for EnumWindowsForProcess
t_string processNameOfWindowsToFind;
std::list<TCHAR*> foundWindowClasses;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;
	HACCEL hAccelTable;

	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof(INITCOMMONCONTROLSEX);
	init.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&init);

	hInst = hInstance;
	mainWindow = std::make_unique<CMainWindow>(hInstance);
	mainWindow->RegisterWindowClass();
	
	// Perform application initialization:
	if (!mainWindow->InitInstance(/*nCmdShow*/))
	{
		return FALSE;
	}
	else
	{
		if (singleInstanceObj.IsAnotherInstanceRunning())
		{
			SendAlreadyRunningNotify();
			return FALSE;
		}

		if (!mainWindow->InitNotifyIcon())
		{
			return FALSE;
		}
		GetIsWindows8();
		LoadFunctionAdresses();
		CreateHook();
		settingsManager = std::make_unique<CRegistrySettingsManager>();
		settingsManager->LoadSettings();
		SetWindowsTransparency();
		if (!settingsManager->GetDisableUpdateCheck())
		{
			CreateUpdateCheckerThread();
		}
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CLEARVIEW));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	//Remove event hooks
	Unhook();
	//Reset windows
	ResetWindowsTransparency();

	return (int) msg.wParam;
}

BOOL IsPaused()
{
	return isPause;
}

void TogglePause()
{
	isPause = !isPause;
	if (isPause){
		Unhook();
	}
	else{
		CreateHook();
		SetWindowsTransparency();
	}
}

void SetWindowsTransparency()
{
	EnumWindows(EnumWindowsProc, NULL);
}

void ResetWindowsTransparency()
{
	EnumWindows(EnumWindowsReset, NULL);
}

BOOL IsWindowUsable(HWND hwnd)
{
	if (GetClassName(hwnd, windowClassName, ARRAYSIZE(windowClassName)))
	{
		LONG_PTR styles = GetWindowLongPtr(hwnd, GWL_STYLE);
		if (GetAncestor(hwnd, GA_PARENT) != GetDesktopWindow() || ((styles & WS_POPUP) && _tcscmp(windowClassName, DIALOGBOXCLASSNAME) != 0))
		{
			//This is not a top-level window or a pop-up window that is not a dialog or it is hidden at the moment, skip it.
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	if (IsWindowUsable(hwnd) && idObject == OBJID_WINDOW)
	{
		//This is a top-level window, enumerate all windows
		OutputDebugString(_T("Enuming...\r\n"));
		SetWindowsTransparency();
		OutputDebugString(_T("-------------------------------------\r\n"));
	}
}

void CALLBACK WinEventProcForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	//When the foreground window has changed, always reset transparency values
	SetWindowsTransparency();
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
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

BOOL CALLBACK EnumWindowsReset(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd))
	{
		SetWindowLongPtr(hwnd, GWL_EXSTYLE, (GetWindowLongPtr(hwnd, GWL_EXSTYLE) ^ WS_EX_LAYERED));
	}
	return TRUE;
}

BOOL CALLBACK EnumWindowsForProcess(HWND hwnd, LPARAM lParam)
{
	if (IsWindowUsable(hwnd))
	{
		GetWindowProcessAndClass(hwnd);
		if (_wcsicmp(fileName, processNameOfWindowsToFind.c_str()) == 0)
		{
			//TODO: Fix memory leak
			auto windowClassNameCopy = new TCHAR[MAX_WINDOW_CLASS_NAME];
			StringCchCopy(windowClassNameCopy, MAX_WINDOW_CLASS_NAME, windowClassName);
			foundWindowClasses.push_back(windowClassNameCopy);
		}
	}
	return TRUE;
}

void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType)
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

BOOL GetWindowProcessAndClass(HWND hwnd)
{
	BOOL result = FALSE;
	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);
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
void CreateHook()
{
	/*EVENT_SYSTEM_FOREGROUND*/
	/*EVENT_OBJECT_SHOW*/
	/*EVENT_SYSTEM_MINIMIZEEND*/
	hWinEventHook[0] = SetWinEventHook(EVENT_OBJECT_SHOW, EVENT_OBJECT_SHOW, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	hWinEventHook[1] = SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProcForeground, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
	hWinEventHook[2] = SetWinEventHook(EVENT_SYSTEM_MINIMIZEEND, EVENT_SYSTEM_MINIMIZEEND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);
}

void Unhook()
{
	for (HWINEVENTHOOK hook : hWinEventHook)
	{
		UnhookWinEvent(hook);
	}
}

/*
Load addresses of function that do not exist on certain version of Windows
*/
void LoadFunctionAdresses()
{
	isImmersive = (PGNSI)GetProcAddress(GetModuleHandle(_T("user32.dll")), "IsImmersiveProcess");
}

void GetIsWindows8()
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

	isWindows8 = VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		dwlConditionMask
	);
	OutputDebugString(isWindows8 ? _T("Running on Windows 8 or 8.1\r\n") : _T("Not running on Windows 8 or 8.1\r\n"));
}

void SendAlreadyRunningNotify()
{
	TCHAR windowClass[MAX_LOADSTRING], windowTitle[MAX_LOADSTRING], windowClosingTitle[MAX_LOADSTRING];
	TCHAR message[] = _T("Already running");

	LoadString(hInst, IDC_CLEARVIEW, windowClass, MAX_LOADSTRING);
	LoadString(hInst, IDS_APP_TITLE, windowTitle, MAX_LOADSTRING);
	LoadString(hInst, IDS_APP_CLOSING_TITLE, windowClosingTitle, MAX_LOADSTRING);

	SetWindowText(mainWindow->GetHwnd(), windowClosingTitle);

	HWND otherHwnd = FindWindow(NULL, windowTitle);
	if (otherHwnd != NULL)
	{
		COPYDATASTRUCT dataCopy;
		dataCopy.dwData = ALREADY_RUNNING_NOTIFY;
		dataCopy.cbData = sizeof(message);
		dataCopy.lpData = message;
		SetLastError(ERROR_SUCCESS);
		SendMessage(otherHwnd, WM_COPYDATA, (WPARAM)(HWND)mainWindow->GetHwnd(), (LPARAM)(LPVOID)&dataCopy);
		DWORD error = GetLastError();
	}
}

#ifdef UNICODE
// Convert std::string to a wchar_t* string.
wchar_t* string_to_wchar_t(std::string string)
{
	size_t newsize = strlen(string.c_str()) + 1;
	wchar_t* wcstring = new wchar_t[newsize];
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring, newsize, string.c_str(), _TRUNCATE);
	return wcstring;
}
#endif // UNICODE
