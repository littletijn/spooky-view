#include "stdafx.h"
#include "CWindow.h"
#include "WndProc.h"
#include "CPtrMap.h"

extern CPtrMap g_ptrmap;

// Saves instance handle and register window class
CWindow::CWindow(HINSTANCE hInstance, HWND hParent = NULL) : CWnd(hInstance)
{
	this->hParent = hParent;
}

//
//  FUNCTION: RegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM CWindow::RegisterWindowClass()
{
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CLEARVIEW));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_CLEARVIEW);
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return CWnd::RegisterWindowClass();
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
BOOL CWindow::InitInstance(int nCmdShow)
{
	CWnd::InitInstance();

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
