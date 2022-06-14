#pragma once

#include "stdafx.h"
#include "CDialog.h"

class CModelessDialog : public CDialog
{
public:
	CModelessDialog(HINSTANCE hInstance);
	~CModelessDialog();
	virtual INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;
protected:
	virtual BOOL SetupDialog() = 0;
	virtual BOOL Create();
};

