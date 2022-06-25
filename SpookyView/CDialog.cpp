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
