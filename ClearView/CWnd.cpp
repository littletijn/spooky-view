#include "stdafx.h"
#include "WndProc.h"
#include "CPtrMap.h"
#include "CWnd.h"

extern CPtrMap g_ptrmap;

// Saves instance handle
CWnd::CWnd(HINSTANCE hInstance)
{
	LoadString(hInstance, IDC_CLEARVIEW, szWindowClass, MAX_LOADSTRING);
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	this->hInstance = hInstance;
	this->hParent = HWND_MESSAGE;
	wcex.cbSize = sizeof(WNDCLASSEX);
}

//
//  FUNCTION: RegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM CWnd::RegisterWindowClass()
{
	wcex.lpfnWndProc = WndProcMain;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = szWindowClass;

	g_ptrmap.Add(this);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(int)
//
//   PURPOSE: Creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL CWnd::InitInstance()
{
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, this->hParent, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	return TRUE;
}
