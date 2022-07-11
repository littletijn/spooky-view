#include "stdafx.h"
#include "AppMain.h"
#include <uxtheme.h>
#include "CLimitSingleInstance.h"
#include "WindowsEnum.h"
#include "ISettingsManager.h"
#include "CRegistrySettingsManager.h"
#include "CMainWindow.h"
#ifdef UNICODE
#include "UpdateChecker.h"
#endif

extern HICON spookyIcon;
extern HINSTANCE hInst;
extern std::unique_ptr<ISettingsManager> settingsManager;
extern std::unique_ptr<CMainWindow> mainWindow;
extern WindowsEnum windowsEnum;
extern PGNSI isImmersive;
extern bool isWindows8;
extern bool isWindows10orNewer;

int AppMain::Run()
{
	MSG msg;
	HACCEL hAccelTable;

	INITCOMMONCONTROLSEX init;
	init.dwSize = sizeof(INITCOMMONCONTROLSEX);
	init.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&init);

	spookyIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SPOOKYVIEW));

	LoadFunctionAdresses();

	mainWindow = std::make_unique<CMainWindow>(hInst);
	mainWindow->RegisterWindowClass();

	// Perform application initialization:
	if (!mainWindow->InitInstance())
	{
		MessageBox(NULL, _T("Cannot create main window."), _T("Error"), MB_OK);
		return FALSE;
	}

	CLimitSingleInstance singleInstanceObj(_T("SpookyView"));
	if (singleInstanceObj.IsAnotherInstanceRunning())
	{
		SendAlreadyRunningNotify();
		return FALSE;
	}

	if (!mainWindow->InitNotifyIcon())
	{
		MessageBox(NULL, _T("Cannot create notification area icon."), _T("Error"), MB_OK);
		return FALSE;
	}
	GetIsWindows8();
	GetIsWindows10orNewer();
	windowsEnum.CreateHook();
	settingsManager = std::make_unique<CRegistrySettingsManager>();
	if (!settingsManager->Init())
	{
		MessageBox(NULL, _T("Cannot create Registry key to store configuration. Settings will not be stored."), _T("Error"), MB_OK);
	}
	else
	{
		settingsManager->LoadSettings();
	}
	windowsEnum.SetWindowsTransparency();
	mainWindow->CheckIsFirstRun();
#ifdef UNICODE
	if (!settingsManager->GetDisableUpdateCheck())
	{
		CreateUpdateCheckerThread();
	}
#endif

	hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(IDC_SPOOKYVIEW));

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
	windowsEnum.Unhook();
	//Reset windows
	windowsEnum.ResetWindowsTransparency();

	return (int)msg.wParam;
}

/*
Load addresses of function that do not exist on certain version of Windows
*/
void AppMain::LoadFunctionAdresses()
{
	isImmersive = (PGNSI)GetProcAddress(GetModuleHandle(_T("user32.dll")), "IsImmersiveProcess");
}

void AppMain::SendAlreadyRunningNotify()
{
	TCHAR windowTitle[MAX_LOADSTRING], windowClosingTitle[MAX_LOADSTRING];
	CHAR message[] = "Spooky View - already running";

	LoadString(hInst, IDS_APP_CLOSING_TITLE, windowClosingTitle, sizeof(windowClosingTitle) / sizeof(TCHAR));
	LoadString(hInst, IDS_APP_TITLE, windowTitle, MAX_LOADSTRING);

	SetWindowText(mainWindow->GetHwnd(), windowClosingTitle);

	HWND otherHwnd = FindWindow(_T("SpookyViewMainClass"), windowTitle);
	if (otherHwnd != NULL)
	{
		COPYDATASTRUCT dataCopy;
		dataCopy.dwData = ALREADY_RUNNING_NOTIFY;
		dataCopy.cbData = sizeof(message);
		dataCopy.lpData = message;
		if (!SendMessage(otherHwnd, WM_COPYDATA, (WPARAM)mainWindow->GetHwnd(), (LPARAM)&dataCopy))
		{
			DWORD error = GetLastError();
		}
	}
}

void AppMain::GetIsWindows8()
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
#ifdef _DEBUG
	OutputDebugString(isWindows8 ? _T("Running on Windows 8 or 8.1\r\n") : _T("Not running on Windows 8 or 8.1\r\n"));
#endif
}

void AppMain::GetIsWindows10orNewer()
{
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	// Initialize the OSVERSIONINFOEX structure.

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 10;
	osvi.dwMinorVersion = 0;
	osvi.wServicePackMajor = 0;
	osvi.wServicePackMinor = 0;

	// Initialize the condition mask.

	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_MINORVERSION, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMAJOR, VER_GREATER_EQUAL);
	VER_SET_CONDITION(dwlConditionMask, VER_SERVICEPACKMINOR, VER_GREATER_EQUAL);

	// Perform the test.

	isWindows10orNewer = VerifyVersionInfo(
		&osvi,
		VER_MAJORVERSION | VER_MINORVERSION |
		VER_SERVICEPACKMAJOR | VER_SERVICEPACKMINOR,
		dwlConditionMask
	);
#ifdef _DEBUG
	OutputDebugString(isWindows10orNewer ? _T("Running on Windows 10 or newer\r\n") : _T("Not running on Windows 10 or newer\r\n"));
#endif
}
