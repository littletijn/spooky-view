#include "stdafx.h"
#include "CModalDialog.h"
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
	return Create();
}
