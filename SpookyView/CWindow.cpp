#include "stdafx.h"
#include "CWindow.h"

extern HICON spookyIcon;

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
	wcex.hIcon = spookyIcon;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCE(IDC_SPOOKYVIEW);
	wcex.hIconSm = NULL;
	wcex.lpfnWndProc = CWindow::StaticWindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.lpszClassName = _T("SpookyViewMainClass");

	return RegisterClassEx(&wcex);
}

LRESULT CALLBACK CWindow::StaticWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CWindow* pThis = NULL;

	if (uMsg == WM_NCCREATE)
	{
		CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
		pThis = (CWindow*)pCreate->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);

		pThis->hWnd = hWnd;
	}
	else
	{
		pThis = (CWindow*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
	}
	if (pThis)
	{
		return pThis->WndProc(hWnd, uMsg, wParam, lParam);
	}
	else
	{
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
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
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, 0, this->hParent, NULL, hInstance, this);

	if (!hWnd)
	{
		return FALSE;
	}
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}
