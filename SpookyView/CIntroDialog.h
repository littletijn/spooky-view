#ifndef CINTRODIALOG_H
#define CINTRODIALOG_H

#include "CModelessDialog.h"

class CIntroDialog :public CModelessDialog
{
public:
	CIntroDialog(HINSTANCE hInstance, HWND hParent);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	BOOL SetupDialog();
	LOGFONT introFont;
	void SetStyle(HWND hDlg);
	void ApplyChoice();
};

#endif