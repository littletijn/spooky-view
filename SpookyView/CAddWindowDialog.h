#pragma once
#include "CModalDialog.h"
#include "ListView.h"
#include <memory>
#include "Textbox.h"
#include "Unicode.h"

class CAddWindowDialog : public CModalDialog
{
public:
	CAddWindowDialog(HINSTANCE hInstance, HWND hParent);
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL InitInstance(t_string programName);
	tstring GetSelectedWindowClass();
protected:
	t_string programName;
	tstring selectedWindowClass;
	std::unique_ptr<ListView> windowsListView;
	std::unique_ptr<Textbox> classTextbox;
	void LoadAppWindows();
	void SetSelectedWindow();
	void StoreSelectedWindow();
};

