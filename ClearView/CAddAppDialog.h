#pragma once
#include "CModalDialog.h"
#include "Tlhelp32.h"

class CAddAppDialog : public CModalDialog
{
public:
	CAddAppDialog(HINSTANCE hInstance, HWND hParent);
	~CAddAppDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	void LoadModules();
	void AddProcessToList(MODULEENTRY32 *module);
	void BrowseFile();
};

