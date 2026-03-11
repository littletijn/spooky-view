#include "stdafx.h"
#include "CModalDialog.h"
#include "CDialog.h"
#include "SpookyView.h"

CModalDialog::CModalDialog(HINSTANCE hInstance, HWND hParent) : CDialog(hInstance)
{
	this->hParent = hParent;
}

INT_PTR CModalDialog::StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CModalDialog* pThis = NULL;

	switch (message) {
		case WM_ACTIVATE:
			if (wParam == 0)
			{
				hWndDialogCurrent = NULL;
			}
			else
			{
				hWndDialogCurrent = hDlg;
			}
			break;
		case WM_INITDIALOG:
			pThis = (CModalDialog*)lParam;
			SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
			pThis->hWnd = hDlg;
			break;
		default:
			pThis = (CModalDialog*)GetWindowLongPtr(hDlg, DWLP_USER);
			break;
	}
	if (pThis)
	{
		return pThis->DlgProc(hDlg, message, wParam, lParam);
	}
	return FALSE;
}

BOOL CModalDialog::Create()
{
	result = DialogBoxParam(hInstance, dialogResource, hParent, StaticDialogProc, (LPARAM)this);
	return TRUE;
}

INT_PTR CModalDialog::GetResult()
{
	return this->result;
}
