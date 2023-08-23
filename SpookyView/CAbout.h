#ifndef CABOUT_H
#define CABOUT_H

#include "CModelessDialog.h"

class CAbout :public CModelessDialog
{
public:
	CAbout(HINSTANCE hInstance, HWND hParent);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	BOOL SetupDialog();
};

#endif