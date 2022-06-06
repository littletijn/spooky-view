#pragma once
#include "CModalDialog.h"
#include "Tlhelp32.h"
#include "ListView.h"
#include <memory>

class CAddAppDialog : public CModalDialog
{
public:
	CAddAppDialog(HINSTANCE hInstance, HWND hParent);
	~CAddAppDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LPWSTR GetSelectedProcess();
protected:
	std::unique_ptr<TCHAR[]> selectedProcess;
	std::unique_ptr<ListView> appsListView;
	void LoadModules();
	void AddProcessToList(TCHAR* exeName);
	void BrowseFile();
	void StoreSelectedProcess();
	void GetProcessProgramName(PROCESSENTRY32 sProcess);
};

