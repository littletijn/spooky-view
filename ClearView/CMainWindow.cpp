#include "stdafx.h"
#include "CMainWindow.h"
#include "ClearView.h"
#include "Resource.h"

//Constructor
CMainWindow::CMainWindow(HINSTANCE hInstance) : CWnd(hInstance) 
{
};

BOOL CMainWindow::InitInstance()
{
	//Try to create window
	BOOL canInit = CWnd::InitInstance();

	if (canInit)
	{
		//Window created, try to create Notification Icon
		TCHAR appName[128];
		LoadString(hInstance, IDS_APP_TITLE, appName, ARRAYSIZE(appName));
		HICON notifyIcon = LoadIcon(this->hInstance, MAKEINTRESOURCE(IDI_CLEARVIEW));
		cNotifyIcon = new CNotifyIcon(this->hWnd, notifyIcon, appName);
		canInit = cNotifyIcon->Init();
		notifyIconContextMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDC_CLEARVIEW));
	}
	return canInit;
}

void CMainWindow::CloseWindow()
{
	DestroyMenu(notifyIconContextMenu);
	delete cNotifyIcon;
	PostQuitMessage(0);
}

//Message handler
LRESULT CALLBACK CMainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
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
			HMENU iconMenu = GetSubMenu(notifyIconContextMenu, 0);
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
			case IDM_EXIT:
				DestroyWindow(this->hWnd);
				return 0;
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