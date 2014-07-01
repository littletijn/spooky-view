#include "CWindow.h"
#include "CNotifyIcon.h"

class CMainWindow: public CWindow
{
public:
	CMainWindow(HINSTANCE hInstance);
	BOOL InitInstance();
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void CloseWindow();
	HMENU GetContextMenu();
	//Members
	CNotifyIcon *cNotifyIcon;
	HMENU notifyIconContextMenu;
};
