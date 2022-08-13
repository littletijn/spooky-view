#include "stdafx.h"
#include "CModalDialog.h"

CDialog::CDialog(HINSTANCE hInstance) : CWnd(hInstance)
{
}

BOOL CDialog::InitInstance()
{
	SetupDialog();
	return Create();
}

void CDialog::SetForeground()
{
	if (IsIconic(hWnd))
	{
		ShowWindow(hWnd, SW_RESTORE);
	}
	SetForegroundWindow(hWnd);
}
