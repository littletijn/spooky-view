#ifndef CMAINWINDOW_H
#define CMAINWINDOW_H

#include "CWindow.h"
#include "CNotifyIcon.h"
#include "CAbout.h"
#include "CIntroDialog.h"
#include "CSetupDialog.h"
#include "CSettingsDialog.h"
#include <memory>
#include "CUpdateAvailableDialog.h"
class CMainWindow: public CWindow
{
public:
	CMainWindow(HINSTANCE hInstance);
	BOOL InitInstance();
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND GetHwnd();
	BOOL InitNotifyIcon();
	void CheckIsFirstRun();
private:
	void OpenContextMenu();
	void OpenSetupDialog();
	void CloseWindow();
	HMENU GetContextMenu();
	//Members
	TCHAR pausedString[80];
	POINT cursorPos;
	BOOL hasDoubleClicked;
	std::unique_ptr<CNotifyIcon> cNotifyIcon;
	HMENU notifyIconContextMenu;
	std::unique_ptr<CSetupDialog> cSetupDialog;
	std::unique_ptr<CAbout> cAboutDialog;
	std::unique_ptr<CUpdateAvailableDialog> cUpdateAvailableDialog;
	std::unique_ptr<CIntroDialog> cIntroDialog;
};

#endif