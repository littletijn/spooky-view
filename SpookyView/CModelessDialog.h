#pragma once

#include "stdafx.h"
#include "CDialog.h"

class CModelessDialog : public CDialog
{
public:
	CModelessDialog(HINSTANCE hInstance, HWND hParent);
	static INT_PTR CALLBACK StaticDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	virtual INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;
protected:
	HWND parentHwnd;
	WORD key;
	virtual BOOL SetupDialog() = 0;
	virtual BOOL Create();
	void CModelessDialog::OnDialogDestroyed();
};

