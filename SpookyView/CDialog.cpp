#include "stdafx.h"
#include "CModalDialog.h"

CDialog::CDialog(HINSTANCE hInstance) : CWnd(hInstance)
{
}

BOOL CDialog::hasInitInstance()
{
	return hasInit;
}

BOOL CDialog::InitInstance()
{
	if (!hasInit)
	{
		SetupDialog();
		auto created = Create();
		if (created)
		{
			hasInit = TRUE;
		}
	}
	return hasInit;
}

void CDialog::SetForeground()
{
	if (IsIconic(hWnd))
	{
		ShowWindow(hWnd, SW_RESTORE);
	}
	SetForegroundWindow(hWnd);
}
