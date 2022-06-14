#pragma once
#include "CModelessDialog.h"

class CAbout :public CModelessDialog
{
public:
	CAbout(HINSTANCE hInstance);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	BOOL SetupDialog();
};

