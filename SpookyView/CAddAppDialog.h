#pragma once
#include "CModalDialog.h"
#include "tlhelp32.h"
#include "ListView.h"
#include "Textbox.h"
#include <memory>

class CAddAppDialog : public CModalDialog
{
public:
	CAddAppDialog(HINSTANCE hInstance, HWND hParent);
	~CAddAppDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	LPTSTR GetSelectedProcess();
protected:
	std::unique_ptr<TCHAR[]> selectedProcess;
	std::unique_ptr<ListView> appsListView;
	std::unique_ptr<Textbox> programTextbox;
	void LoadModules();
	void BrowseFile();
	void SetSelectedProgram();
	void StoreSelectedProcess();
	void GetProcessProgramName(PROCESSENTRY32 sProcess, t_string* programName);
};

