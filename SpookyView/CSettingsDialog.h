#pragma once
#include "CModelessDialog.h"

class CSettingsDialog :public CModelessDialog
{
public:
	CSettingsDialog(HINSTANCE hInstance, HWND hParent);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	BOOL SetupDialog();
	void ApplySettings();
	void SetFormValues(HWND hDlg);
	BOOL HasAutoRunValue();
	void AddAutoRun();
	void RemoveAutoRun();
};