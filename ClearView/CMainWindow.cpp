#include "stdafx.h"
#include "CMainWindow.h"
#include "ClearView.h"
#include "Resource.h"
#include "CLimitSingleInstance.h"
#include "CSettingsDialog.h"
#include "Defines.h"
#include "UpdateResponse.h"

extern UpdateResponse updateResponse;
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
	HICON notifyIcon = LoadIcon(this->hInstance, MAKEINTRESOURCE(IDI_CLEARVIEW));
	cNotifyIcon = std::make_unique<CNotifyIcon>(this->hWnd, notifyIcon, appName);
	BOOL canInit = cNotifyIcon->Init();
	notifyIconContextMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_CLEARVIEW));
	return canInit && notifyIconContextMenu != NULL;
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
		case WM_UPDATE_AVAILABLE:
		{
			cUpdateAvailableDialog = std::make_unique<CUpdateAvailableDialog>(this->hInstance);
#ifdef UNICODE
			auto convertedMessage = string_to_wchar_t(updateResponse.message);
			cUpdateAvailableDialog->SetMessage(convertedMessage);
			delete[] convertedMessage;
			auto convertedDownloadUrl = string_to_wchar_t(updateResponse.download_url);
			cUpdateAvailableDialog->SetDownloadUrl(convertedDownloadUrl);
			delete[] convertedDownloadUrl;
#else
			cUpdateAvailableDialog->setMessage(updateResponse.message);
			cUpdateAvailableDialog->SetDownloadUrl(updateResponse.download_url);
#endif // UNICODE
			cUpdateAvailableDialog->InitInstance();
			break;
		}

		case WM_COPYDATA:
		{
			PCOPYDATASTRUCT dataCopy = (PCOPYDATASTRUCT)lParam;
			if (dataCopy->dwData == ALREADY_RUNNING_NOTIFY)
			{
				ShowAlreadyRunningBalloon();
			}
		}
		break;

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
			break;
		case WM_LBUTTONDBLCLK:
			//Left double click. Open default option of Context menu
			break;
		case WM_LBUTTONUP:
			//Left single click. Open Flyout window
			break;
		}
		break;
	case WM_COMMAND:
		//Handles the Context Menu of the Notification Area Icon
		switch (LOWORD(wParam))
		{
		case IDM_SETTINGS:
			cSettingsDialog = std::make_unique<CSettingsDialog>(this->hInstance);
			cSettingsDialog->InitInstance();
			break;

			case IDM_OPEN:
			{
				cSetupDialog = std::make_unique<CSetupDialog>(this->hInstance);
				cSetupDialog->InitInstance();
			}
			break;

			case IDM_ABOUT:
			{
				cAboutDialog = std::make_unique<CAbout>(this->hInstance);
				cAboutDialog->InitInstance();
			}
			break;

			case IDM_EXIT:
				DestroyWindow(this->hWnd);
				return 0;
			break;

			case IDM_PAUSE:
				UINT checkState = IsPaused() ? MF_UNCHECKED : MF_CHECKED;
				if (CheckMenuItem(GetContextMenu(), IDM_PAUSE, checkState) != -1){
					DrawMenuBar(hWnd);
					TogglePause();
				}
			break;
		}
		break;
	case WM_DESTROY:
		this->CloseWindow();
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void CMainWindow::ShowAlreadyRunningBalloon()
{
	cNotifyIcon->ShowBalloon(_T("ClearView is already running"), _T("Right click on this icon to configure this application."));
}