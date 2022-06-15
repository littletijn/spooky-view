#pragma once
#include "CModalDialog.h"
#include "ListView.h"
#include <memory>

class CAddWindowDialog : public CModalDialog
{
public:
	CAddWindowDialog(HINSTANCE hInstance, HWND hParent);
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL InitInstance(t_string programName);
	LPTSTR GetSelectedWindowClass();
protected:
	t_string programName;
	std::unique_ptr<TCHAR[]> selectedWindowClass;
	std::unique_ptr<ListView> windowsListView;
	void LoadAppWindows();
	void SetSelectedWindow();
	void StoreSelectedWindow();
};

