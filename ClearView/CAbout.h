#pragma once
#include "CDialog.h"

class CAbout :public CDialog
{
public:
	CAbout(HINSTANCE hInstance);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	void SetupDialog();
};

