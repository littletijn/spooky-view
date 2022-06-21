#include "stdafx.h"
#include "CModelessDialog.h"
#include "Defines.h"

extern HICON spookyIcon;

CModelessDialog::CModelessDialog(HINSTANCE hInstance, HWND hParent) : CDialog(hInstance)
{
	this->hParent = hParent;
}

INT_PTR CModelessDialog::StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CModelessDialog* pThis = NULL;

	if (message == WM_INITDIALOG)
	{
		pThis = (CModelessDialog*)lParam;
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)pThis);
		pThis->hWnd = hDlg;
	}
	else
	{
		pThis = (CModelessDialog*)GetWindowLongPtr(hDlg, DWLP_USER);
	}
	if (pThis)
	{
		if (message == WM_DESTROY)
		{
			//Remove pointer to window to make sure that our DlgProc is not called anymore.
			//This pointer might be not valid anymore after calling OnDialogDestroyed()
			SetWindowLongPtr(hDlg, DWLP_USER, NULL);
			pThis->OnDialogDestroyed();
		}
		else
		{
			return pThis->DlgProc(hDlg, message, wParam, lParam);
		}
		
	}
	return FALSE;
}

BOOL CModelessDialog::Create()
{
	this->hWnd = CreateDialogParam(this->hInstance, this->dialogResource, NULL, StaticDialogProc, (LPARAM)this);
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

void CModelessDialog::OnDialogDestroyed()
{
	if (hParent != NULL && key > 0)
	{
		SendMessage(hParent, WM_MODELESS_DIALOG_DESTROYED, key, NULL);
	}
}
