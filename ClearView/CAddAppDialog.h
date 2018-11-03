#pragma once
#include "CModalDialog.h"
#include "Tlhelp32.h"
#include "ListView.h"

class CAddAppDialog : public CModalDialog
{
public:
	CAddAppDialog(HINSTANCE hInstance, HWND hParent);
	~CAddAppDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	ListView * appsListView;
	void LoadModules();
	void AddProcessToList(MODULEENTRY32 *module, TCHAR *programName);
	void BrowseFile();
};

