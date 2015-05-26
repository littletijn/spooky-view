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

using namespace std;

typedef BOOL (WINAPI *PGNSI)(HANDLE);
typedef BOOL (WINAPI *PGNSI2)(HWND, MARGINS*);

//http://msdn.microsoft.com/en-us/library/windows/desktop/dd318055(v=vs.85).aspx
const TCHAR DIALOGBOXCLASSNAME[7] = _T("#32770");

// Global Variables:
HINSTANCE hInst;								// current instance
CMainWindow* mainWindow;
HWINEVENTHOOK hWinEventHook[3];
PGNSI isImmersive;
CSettings* settings;
ISettingsManager* settingsManager;
BOOL isPause = false;
CLimitSingleInstance singleInstanceObj(_T("ClearView"));

//http://msdn.microsoft.com/en-us/library/windows/desktop/ms633577(v=vs.85).aspx
//Max length of className is 256 characters
TCHAR windowClassName[256];

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
	mainWindow = new CMainWindow(hInstance);
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
		LoadFunctionAdresses();
		CreateHook();
		settingsManager = new CRegistrySettingsManager();
		settings = settingsManager->LoadSettings();
		EnumWindows(EnumWindowsProc, NULL);
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
	EnumWindows(EnumWindowsReset, NULL);

	//Delete pointers
	delete mainWindow;
	delete settings;
	delete settingsManager;

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
		EnumWindows(EnumWindowsProc, NULL);
	}
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
		EnumWindows(EnumWindowsProc, NULL);
		OutputDebugString(_T("-------------------------------------\r\n"));
	}
}

void CALLBACK WinEventProcForeground(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime)
{
	//When the foreground window has changed, always reset transparency values
	EnumWindows(EnumWindowsProc, NULL);
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

void SetWindowAlpha(HWND hwnd, CSettings::WindowTypes windowType)
{
	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
	if (hProcess != NULL)
	{
		//Check if current process is not a Windows Store App or Explorer
		//if (isImmersive == NULL || !isImmersive(hProcess))
		{
			//Create buffers
			TCHAR filePathName[MAX_PATH];

			//Get process image file name
			GetProcessImageFileName(hProcess, filePathName, ARRAYSIZE(filePathName));
			TCHAR *fileName = _tcsrchr(filePathName, '\\');
			fileName++;

			//Output debug data
			OutputDebugString(fileName);
			OutputDebugString(_T("  --  "));
			OutputDebugString(windowClassName);
			OutputDebugString(_T("\r\n"));

			if (GetClassName(hwnd, windowClassName, ARRAYSIZE(windowClassName)))
			{
				BYTE alpha;
				if (settings->GetAlphaSetting(fileName, windowClassName, windowType, alpha))
				{
					SetWindowLongPtr(hwnd, GWL_EXSTYLE, (GetWindowLongPtr(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED));
					SetLayeredWindowAttributes(hwnd, 0, alpha, LWA_ALPHA);
				}
			}
		}
		CloseHandle(hProcess);
	}
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
	for each (HWINEVENTHOOK hook in hWinEventHook)
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