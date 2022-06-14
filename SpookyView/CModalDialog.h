#pragma once
#include "CDialog.h"


class CModalDialog : public CDialog
{
public:
	CModalDialog(HINSTANCE hInstance, HWND hParent);
	~CModalDialog();
	virtual INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) = 0;
	INT_PTR GetResult();
protected:
	HWND hParent;
	INT_PTR result;
	virtual BOOL SetupDialog() = 0;
	virtual BOOL Create();
};