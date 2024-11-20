#ifndef CINTRODIALOG_H
#define CINTRODIALOG_H

#include "CModelessDialog.h"

class CIntroDialog :public CModelessDialog
{
public:
	CIntroDialog(HINSTANCE hInstance, HWND hParent);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void setSkipWelcomeCheckbox(BOOL state);
protected:
	BOOL SetupDialog();
	LOGFONT introFont;
	void SetupWindow(HWND hDlg);
	void ApplyChoice();
};

#endif