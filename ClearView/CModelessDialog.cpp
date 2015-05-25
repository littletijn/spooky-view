#include "stdafx.h"
#include "CModelessDialog.h"
#include "WndProc.h"
#include "CPtrMap.h"

extern CPtrMap<CDialog> g_ptrmapDlg;

CModelessDialog::CModelessDialog(HINSTANCE hInstance) : CDialog(hInstance)
{
}


CModelessDialog::~CModelessDialog()
{
}

BOOL CModelessDialog::Create()
{
	this->hWnd = CreateDialog(this->hInstance, this->dialogResource, NULL, DlgProcMain);
	if (this->hWnd != NULL)
	{
		ShowWindow(this->hWnd, SW_NORMAL);
		return TRUE;
	}
	return FALSE;
}
