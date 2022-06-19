#include "stdafx.h"
#include "CModelessDialog.h"
#include "WndProc.h"
#include "CPtrMap.h"

extern CPtrMap<CDialog> g_ptrmapDlg;
extern HICON spookyIcon;

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
		if (spookyIcon)
		{
			SendMessage(hWnd, WM_SETICON, ICON_SMALL, (LPARAM)spookyIcon);
			SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)spookyIcon);
		}
		ShowWindow(this->hWnd, SW_NORMAL);
		return TRUE;
	}
	return FALSE;
}
