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
		return FALSE;
	}
	else
	{
		CLimitSingleInstance singleInstanceObj(_T("SpookyView"));
		if (singleInstanceObj.IsAnotherInstanceRunning())
		{
			SendAlreadyRunningNotify();
			return FALSE;
		}

		if (!mainWindow->InitNotifyIcon())
		{
			return FALSE;
		}
		windowsEnum.GetIsWindows8();
		windowsEnum.CreateHook();
		settingsManager = std::make_unique<CRegistrySettingsManager>();
		settingsManager->Init();
		settingsManager->LoadSettings();
		windowsEnum.SetWindowsTransparency();
		mainWindow->CheckIsFirstRun();
#ifdef UNICODE
		if (!settingsManager->GetDisableUpdateCheck())
		{
			CreateUpdateCheckerThread();
		}
#endif
	}

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
