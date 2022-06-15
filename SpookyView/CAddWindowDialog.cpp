#include "stdafx.h"
#include "CAddWindowDialog.h"
#include "SpookyView.h"
#include "Defines.h"
#include <strsafe.h>
#include <commctrl.h>
#include "Textbox.h"

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
		this->windowsListView = std::make_unique<ListView>(hDlg, IDC_LIST_ADD_WINDOWS);
		this->classTextbox = std::make_unique<Textbox>(hDlg, IDC_EDIT_CLASS_NAME);
		LoadAppWindows();
		return TRUE;
		break;
	case WM_NOTIFY:
	{
		LPNMHDR notifyMessage = (LPNMHDR)lParam;
		if (notifyMessage->code == LVN_ITEMCHANGED)
		{
			switch (notifyMessage->idFrom)
			{
			case IDC_LIST_ADD_WINDOWS:
				SetSelectedWindow();
				return TRUE;
			}
		}
	}
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

void CAddWindowDialog::SetSelectedWindow()
{
	int index = this->windowsListView->GetSelectedIndex();
	TCHAR textBuffer[MAX_WINDOW_CLASS_NAME];
	LPTSTR text = this->windowsListView->GetTextByIndex(index, textBuffer);
	this->classTextbox->SetText(text);
}

void CAddWindowDialog::StoreSelectedWindow()
{
	int textLength = 0;
	auto textBuffer = this->classTextbox->GetText(&textLength);
	this->selectedWindowClass = std::unique_ptr<TCHAR[]>(new TCHAR[textLength + 1]);
	StringCchCopy(this->selectedWindowClass.get(), textLength + 1, textBuffer.get());
}

LPTSTR CAddWindowDialog::GetSelectedWindowClass()
{
	return this->selectedWindowClass.get();
}
