#include "stdafx.h"
#include "CDialog.h"
#include "WndProc.h"
#include "CPtrMap.h"

extern CPtrMap<CDialog> g_ptrmapDlg;

CDialog::CDialog(HINSTANCE hInstance) : CWnd(hInstance)
{
}


CDialog::~CDialog()
{
}

BOOL CDialog::InitInstance()
{
	SetupDialog();
	g_ptrmapDlg.Add(this);

	this->hWnd = CreateDialog(this->hInstance, this->dialogResource, NULL, DlgProcMain);
	if (this->hWnd != NULL)
	{
		ShowWindow(this->hWnd, SW_NORMAL);
		return TRUE;
	}
	return FALSE;
}
