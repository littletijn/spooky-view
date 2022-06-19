#include "CWindow.h"
#include "CNotifyIcon.h"
#include "CAbout.h"
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
	void CloseWindow();
	void ShowAlreadyRunningBalloon();
	HMENU GetContextMenu();
	//Members
	std::unique_ptr<CNotifyIcon> cNotifyIcon;
	HMENU notifyIconContextMenu;
	std::unique_ptr<CSetupDialog> cSetupDialog;
	std::unique_ptr<CSettingsDialog> cSettingsDialog;
	std::unique_ptr<CAbout> cAboutDialog;
	std::unique_ptr<CUpdateAvailableDialog> cUpdateAvailableDialog;
};
