#include "CWindow.h"
#include "CNotifyIcon.h"

class CMainWindow: public CWnd
{
public:
	CMainWindow(HINSTANCE hInstance);
	~CMainWindow();
	BOOL InitInstance();
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	void CloseWindow();
	//Members
	CNotifyIcon *cNotifyIcon;
	HMENU notifyIconContextMenu;
};
