#include "stdafx.h"
#include "CAbout.h"

CAbout::CAbout(HINSTANCE hInstance) : CDialog(hInstance)
{
};

void CAbout::SetupDialog()
{
	this->dialogResource = MAKEINTRESOURCE(IDD_ABOUTBOX);
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