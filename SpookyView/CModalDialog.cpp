#include "stdafx.h"
#include "CModalDialog.h"
#include "CDialog.h"

CModalDialog::CModalDialog(HINSTANCE hInstance, HWND hParent) : CDialog(hInstance)
{
	this->hParent = hParent;
}

INT_PTR CModalDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CModalDialog* pThis = NULL;

	if (message == WM_INITDIALOG)
	{
		pThis = (CModalDialog*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
		pThis->hWnd = hDlg;
	}
	else
	{
		pThis = (CModalDialog*)GetWindowLongPtr(hDlg, DWLP_USER);
	}
	if (pThis)
	{
		return pThis->DlgProc(hDlg, message, wParam, lParam);
	}
	return FALSE;
}

BOOL CModalDialog::Create()
{
	result = DialogBoxParam(hInstance, dialogResource, hParent, DialogProc, (LPARAM)this);
	return TRUE;
}

INT_PTR CModalDialog::GetResult()
{
	return this->result;
}
