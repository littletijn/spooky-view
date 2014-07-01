#pragma once

#include "stdafx.h"
#include "CWnd.h"

class CDialog : protected CWnd
{
public:
	CDialog(HINSTANCE hInstance);
	~CDialog();
	BOOL InitInstance();
	virtual INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;
protected:
	LPCTSTR dialogResource;
	virtual void SetupDialog() = 0;
};

