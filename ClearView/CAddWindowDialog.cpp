#include "stdafx.h"
#include "CAddWindowDialog.h"
#include "ClearView.h"
#include "Defines.h"
#include <strsafe.h>

CAddWindowDialog::CAddWindowDialog(HINSTANCE hInstance, HWND hParent) : CModalDialog(hInstance, hParent)
{
}

BOOL CAddWindowDialog::SetupDialog()
{
	this->dialogResource = MAKEINTRESOURCE(IDD_ADD_WINDOW);
	return TRUE;
}

BOOL CAddWindowDialog::InitInstance(t_string programName)
{
	this->programName = programName;
	return CDialog::InitInstance();
}

INT_PTR CALLBACK CAddWindowDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	this->hWnd = hDlg;
	switch (message)
	{
	case WM_INITDIALOG:
		this->windowsListView = std::make_unique<ListView>(hDlg, IDC_LIST_ADD_APPS);
		LoadAppWindows();
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			StoreSelectedWindow();
			EndDialog(hDlg, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 2);
			return TRUE;

		}
	}
	return FALSE;
}

void CAddWindowDialog::LoadAppWindows()
{
	foundWindowClasses.clear();
	processNameOfWindowsToFind = this->programName;
	EnumWindows(EnumWindowsForProcess, NULL);
	for (TCHAR* windowClass : foundWindowClasses) {
		this->windowsListView->AddItem(windowClass);
	}
}

void CAddWindowDialog::StoreSelectedWindow()
{
	int index = this->windowsListView->GetSelectedIndex();
	TCHAR textBuffer[MAX_WINDOW_CLASS_NAME];
	LPWSTR text = this->windowsListView->GetTextByIndex(index, textBuffer);
	this->selectedWindowClass = std::unique_ptr<TCHAR[]>(new TCHAR[MAX_PATH]);
	StringCchCopy(this->selectedWindowClass.get(), MAX_PATH, textBuffer);
}

LPWSTR CAddWindowDialog::GetSelectedWindowClass()
{
	return this->selectedWindowClass.get();
}
