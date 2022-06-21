#include "stdafx.h"
#include "CMainWindow.h"
#include "SpookyView.h"
#include "Resource.h"
#include "CLimitSingleInstance.h"
#include "CSettingsDialog.h"
#include "Defines.h"
#include "UpdateResponse.h"
#include "ISettingsManager.h"

extern UpdateResponse updateResponse;
extern std::unique_ptr<ISettingsManager> settingsManager;
#ifdef UNICODE
extern wchar_t* string_to_wchar_t(std::string string);
#endif // UNICODE

//Constructor
CMainWindow::CMainWindow(HINSTANCE hInstance) : CWindow(hInstance) 
{
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

		case WM_UPDATE_AVAILABLE:
		{
#ifdef UNICODE
			auto versionNumber = string_to_wchar_t(updateResponse.version);
#else
			auto versionNumber = updateResponse.version;
#endif // UNICODE
			if (!settingsManager->ShouldSkipVersion(versionNumber))
			{
#ifdef UNICODE
				auto message = string_to_wchar_t(updateResponse.message);
				auto downloadUrl = string_to_wchar_t(updateResponse.download_url);
#else
				auto message = updateResponse.message;
				auto downloadUrl = updateResponse.download_url;
#endif // UNICODE
				cUpdateAvailableDialog = std::make_unique<CUpdateAvailableDialog>(this->hInstance, this->hWnd);
				cUpdateAvailableDialog->SetMessage(message);
				cUpdateAvailableDialog->SetDownloadUrl(downloadUrl);
				cUpdateAvailableDialog->SetVersionNumber(versionNumber);
				cUpdateAvailableDialog->InitInstance();
#ifdef UNICODE
				// Only the unicode string_to_wchar_t returns pointers. The other aren't.
				delete[] message;
				delete[] downloadUrl;
				delete[] versionNumber;
#endif
			}
			return FALSE;
		}

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
					ShowAlreadyRunningBalloon();
				}
			}
		}
		return FALSE;

	case WM_NOTIFYICON:
		//Handles the Notification Area Icon
		switch (lParam)
		{
		case WM_CONTEXTMENU:
		case WM_RBUTTONUP:
		case NIN_KEYSELECT:
		case NIN_SELECT:
			//Right click. Open Context menu
		{
			POINT cursorPos;
			GetCursorPos(&cursorPos);
			//We must set the owning window of this Context Menu on the foreground, otherwise the menu cannot be closed by clicking outside of the menu.
			SetForegroundWindow(hWnd);
			HMENU iconMenu = GetContextMenu();
			TrackPopupMenu(iconMenu, TPM_CENTERALIGN | TPM_BOTTOMALIGN, cursorPos.x, cursorPos.y, 0, hWnd, NULL);
		}
			return FALSE;
		case WM_LBUTTONDBLCLK:
			//Left double click. Open default option of Context menu
			return FALSE;
		case WM_LBUTTONUP:
			//Left single click. Open Flyout window
			return FALSE;
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
			return FALSE;

			case IDM_OPEN:
			{
				if (!cSetupDialog)
				{
					cSetupDialog = std::make_unique<CSetupDialog>(this->hInstance, this->hWnd);
					cSetupDialog->InitInstance();
				}
			}
			return FALSE;

			case IDM_ABOUT:
			{
				if (!cAboutDialog)
				{
					cAboutDialog = std::make_unique<CAbout>(this->hInstance, this->hWnd);
					cAboutDialog->InitInstance();
				}
			}
			return FALSE;

			case IDM_EXIT:
				DestroyWindow(this->hWnd);
				return FALSE;

			case IDM_PAUSE:
				UINT checkState = IsPaused() ? MF_UNCHECKED : MF_CHECKED;
				if (CheckMenuItem(GetContextMenu(), IDM_PAUSE, checkState) != -1){
					DrawMenuBar(hWnd);
					TogglePause();
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

void CMainWindow::ShowAlreadyRunningBalloon()
{
	cNotifyIcon->ShowBalloon(_T("Spooky View is already running"), _T("Right click on this icon to configure this application."));
}

void CMainWindow::CheckIsFirstRun()
{
	if (!settingsManager->GetSkipWelcome())
	{
		settingsManager->SetSkipWelcome();
		cNotifyIcon->ShowBalloon(_T("Welcome to Spooky View"), _T("Right click on this icon to configure this application."));
	}
}