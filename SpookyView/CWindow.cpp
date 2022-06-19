#include "stdafx.h"
#include "CWindow.h"
#include "CPtrMap.h"
#include "WndProc.h"

extern CPtrMap<CWindow> g_ptrmap;

// Saves instance handle and register window class
CWindow::CWindow(HINSTANCE hInstance, HWND hParent) : CWnd(hInstance)
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
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	this->hParent = HWND_MESSAGE; // Make this window a Message-only window
	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SPOOKYVIEW));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPOOKYVIEW);
	wcex.hIconSm = NULL;
	wcex.lpfnWndProc = WndProcMain;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = _T("SpookyViewMainClass");

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
BOOL CWindow::InitInstance(int nCmdShow)
{
	hWnd = CreateWindow(_T("SpookyViewMainClass"), szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, this->hParent, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
