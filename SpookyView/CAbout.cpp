#include "stdafx.h"
#include "CAbout.h"

CAbout::CAbout(HINSTANCE hInstance, HWND hParent) : CModelessDialog(hInstance, hParent)
{
};

BOOL CAbout::SetupDialog()
{
	key = IDD_ABOUTBOX;
	this->dialogResource = MAKEINTRESOURCE(IDD_ABOUTBOX);
	return TRUE;
}

INT_PTR CALLBACK CAbout::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}