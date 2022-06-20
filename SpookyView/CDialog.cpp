#include "stdafx.h"
#include "CModalDialog.h"

CDialog::CDialog(HINSTANCE hInstance) : CWnd(hInstance)
{
}

CDialog::~CDialog()
{
}

BOOL CDialog::InitInstance()
{
	SetupDialog();
	return Create();
}
