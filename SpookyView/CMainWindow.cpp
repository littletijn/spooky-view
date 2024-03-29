#include "stdafx.h"
#include "CMainWindow.h"
#include "SpookyView.h"
#include "Resource.h"
#include "CLimitSingleInstance.h"
#include "CSettingsDialog.h"
#include "Defines.h"
#include "UpdateResponse.h"
#include "ISettingsManager.h"
#include "WindowsEnum.h"
#ifdef UNICODE
#include "UnicodeConversion.h"
#endif //UNICODE
#include "SpookyView.h"

const int SINGLE_CLICK_TIMER = 1;

//Constructor
CMainWindow::CMainWindow(HINSTANCE hInstance) : CWindow(hInstance) 
{
	LoadString(hInstance, IDS_PAUSED, pausedString, ARRAYSIZE(pausedString));
};

BOOL CMainWindow::InitInstance()
{
	//Try to create window
	BOOL canInit = CWindow::InitInstance(0);
	return canInit;
}

BOOL CMainWindow::InitNotifyIcon()
{
	//try to create Notification Icon
	TCHAR appName[MAX_LOADSTRING];
	LoadString(hInstance, IDS_APP_TITLE, appName, ARRAYSIZE(appName));
	HICON notifyIcon = LoadIcon(this->hInstance, MAKEINTRESOURCE(IDI_SPOOKYVIEW));
	cNotifyIcon = std::make_unique<CNotifyIcon>(this->hWnd, notifyIcon, appName);
	BOOL canInit = cNotifyIcon->Init();
	notifyIconContextMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_SPOOKYVIEW));
	return canInit == TRUE && notifyIconContextMenu != NULL;
}

void CMainWindow::CloseWindow()
{
	DestroyMenu(notifyIconContextMenu);
	PostQuitMessage(0);
}

HMENU CMainWindow::GetContextMenu()
{
	return GetSubMenu(notifyIconContextMenu, 0);
}

HWND CMainWindow::GetHwnd()
{
	return this->hWnd;
}

//Message handler
LRESULT CALLBACK CMainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_MODELESS_DIALOG_DESTROYED:
			switch (wParam)
			{
			case IDD_SETTINGS:
				cSettingsDialog.reset();
				break;
			case IDD_SETUP:
				cSetupDialog.reset();
				break;
			case IDD_ABOUTBOX:
				cAboutDialog.reset();
				break;
			}
			return FALSE;
#ifdef UNICODE
		case WM_UPDATE_AVAILABLE:
		{
			auto versionNumber = string_to_wchar_t(updateResponse.version);
			if (!settingsManager->ShouldSkipVersion(versionNumber.get()))
			{
				auto message = string_to_wchar_t(updateResponse.message);
				auto downloadUrl = string_to_wchar_t(updateResponse.download_url);
				cUpdateAvailableDialog = std::make_unique<CUpdateAvailableDialog>(this->hInstance, this->hWnd);
				cUpdateAvailableDialog->SetMessage(message.get());
				cUpdateAvailableDialog->SetDownloadUrl(downloadUrl.get());
				cUpdateAvailableDialog->SetVersionNumber(versionNumber.get());
				cUpdateAvailableDialog->InitInstance();
			}
			return FALSE;
		}
#endif

		case WM_COPYDATA:
		{
			if (InSendMessage())
			{
				ReplyMessage(TRUE);
			}
			PCOPYDATASTRUCT dataCopy = (PCOPYDATASTRUCT)lParam;
			if (dataCopy->dwData == ALREADY_RUNNING_NOTIFY)
			{
				CHAR *message = (CHAR*)dataCopy->lpData;
				if (strrchr(message, *"Spooky View - already running"))
				{
					OpenSetupDialog();
				}
			}
		}
		return FALSE;

	case WM_TIMER:
		if (wParam == SINGLE_CLICK_TIMER)
		{
			KillTimer(hWnd, SINGLE_CLICK_TIMER);
			if (!hasDoubleClicked)
			{
				OpenContextMenu();
			}
			hasDoubleClicked = FALSE;
		}
		return FALSE;

	case WM_NOTIFYICON:
		//Handles the Notification Area Icon
		switch (lParam)
		{
		case WM_CONTEXTMENU:
		case WM_RBUTTONUP:
		case NIN_KEYSELECT:
			//Right click. Open Context menu
			GetCursorPos(&cursorPos);
			OpenContextMenu();
			return FALSE;
		case WM_LBUTTONDBLCLK:
			//Left double click. Open default option of Context menu
			GetCursorPos(&cursorPos);
			hasDoubleClicked = TRUE;
			KillTimer(hWnd, SINGLE_CLICK_TIMER);
			OpenSetupDialog();
			return FALSE;
		case WM_LBUTTONUP:
			//Left single click. Start timer to check if not a double click is made
			GetCursorPos(&cursorPos);
			SetTimer(hWnd, SINGLE_CLICK_TIMER, GetDoubleClickTime(), NULL);
			break;
			//return FALSE;
		}
		break;
	case WM_COMMAND:
		//Handles the Context Menu of the Notification Area Icon
		switch (LOWORD(wParam))
		{
			case IDM_SETTINGS:
				if (!cSettingsDialog)
				{
					cSettingsDialog = std::make_unique<CSettingsDialog>(this->hInstance, this->hWnd);
					cSettingsDialog->InitInstance();
				}
				else
				{
					cSettingsDialog->SetForeground();
				}
			return FALSE;

			case IDM_OPEN:
				OpenSetupDialog();
				return FALSE;

			case IDM_ABOUT:
			{
				if (!cAboutDialog)
				{
					cAboutDialog = std::make_unique<CAbout>(this->hInstance, this->hWnd);
					cAboutDialog->InitInstance();
				}
				else
				{
					cAboutDialog->SetForeground();
				}
			}
			return FALSE;

			case IDM_EXIT:
				DestroyWindow(this->hWnd);
				return FALSE;

			case IDM_PAUSE:
				UINT checkState = windowsEnum.IsPaused() ? MF_UNCHECKED : MF_CHECKED;
				if (CheckMenuItem(GetContextMenu(), IDM_PAUSE, checkState) != -1){
					DrawMenuBar(hWnd);
					windowsEnum.TogglePause();
					if (windowsEnum.IsPaused())
					{
						cNotifyIcon->SetTooltipText(pausedString);
					}
					else
					{
						cNotifyIcon->SetTooltipText(NULL);
					}

				}
				return FALSE;
		}
		break;
	case WM_DESTROY:
		this->CloseWindow();
		return FALSE;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return TRUE;
}

void CMainWindow::OpenContextMenu()
{
	//We must set the owning window of this Context Menu on the foreground, otherwise the menu cannot be closed by clicking outside of the menu.
	SetForegroundWindow(hWnd);
	HMENU iconMenu = GetContextMenu();
	TrackPopupMenu(iconMenu, TPM_CENTERALIGN | TPM_BOTTOMALIGN, cursorPos.x, cursorPos.y, 0, hWnd, NULL);
}

void CMainWindow::OpenSetupDialog()
{
	if (!cSetupDialog)
	{
		cSetupDialog = std::make_unique<CSetupDialog>(this->hInstance, this->hWnd);
		cSetupDialog->InitInstance();
	}
	else
	{
		cSetupDialog->SetForeground();
	}
}

void CMainWindow::ShowAlreadyRunningBalloon()
{
	TCHAR titleString[100];
	TCHAR messageString[100];
	LoadString(hInst, IDS_ALREADY_RUNNING, titleString, sizeof(titleString) / sizeof(TCHAR));
	if (isWindows10orNewer)
	{
		LoadString(hInst, DS_ALREADY_RUNNING_HINT_WIN10, messageString, sizeof(messageString) / sizeof(TCHAR));
	}
	else
	{
		LoadString(hInst, DS_ALREADY_RUNNING_HINT, messageString, sizeof(messageString) / sizeof(TCHAR));
	}
	cNotifyIcon->ShowBalloon(titleString, messageString);
}

void CMainWindow::CheckIsFirstRun()
{
	if (!settingsManager->GetSkipWelcome())
	{
		settingsManager->SetSkipWelcome();
		TCHAR titleString[100];
		TCHAR messageString[100];
		LoadString(hInst, IDS_WELCOME_TITLE, titleString, sizeof(titleString) / sizeof(TCHAR));
		if (isWindows10orNewer)
		{
			LoadString(hInst, IDS_WELCOME_MESSAGE_WIN10, messageString, sizeof(messageString) / sizeof(TCHAR));
		}
		else
		{
			LoadString(hInst, IDS_WELCOME_MESSAGE, messageString, sizeof(messageString) / sizeof(TCHAR));
		}
		cNotifyIcon->ShowBalloon(titleString, messageString);
	}
}