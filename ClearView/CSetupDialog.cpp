#include "stdafx.h"
#include "CSetupDialog.h"
#include "CAddAppDialog.h"
#include <CommCtrl.h>
#include "CSettings.h"
#include "CProgramSetting.h"
#include "String.h"
#include <vector>

using namespace std;

extern CSettings *settings;

CSetupDialog::CSetupDialog(HINSTANCE hInstance) : CModelessDialog(hInstance)
{
}

CSetupDialog::~CSetupDialog()
{
}

BOOL CSetupDialog::SetupDialog()
{
	this->dialogResource = MAKEINTRESOURCE(IDD_SETUP);
	return TRUE;
}

INT_PTR CALLBACK CSetupDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
	{
		LPNMHDR notifyMessage = (LPNMHDR)lParam;
		switch (notifyMessage->idFrom)
		{
			case IDC_LIST_APPS:
				ProgramsListNotified(lParam);
				break;

			case IDC_LIST_WINDOWS:
				WindowsListNotified(lParam);
				break;
		}
	}
	break;

	case WM_INITDIALOG:
		PopulateProcessList(hDlg);
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
			case IDCANCEL:
				DestroyWindow(hDlg);
				return TRUE;
			case IDC_BUTTON_APP_ADD:
				CAddAppDialog *appDialog = new CAddAppDialog(this->hInstance, this->hWnd);
				appDialog->InitInstance();
				return TRUE;
				break;

		}
		break;
	}
	return FALSE;
}

void CSetupDialog::ProgramsListNotified(LPARAM lParam)
{
	int index = GetSelectedItemIndex(lParam);
	if (index > -1)
	{
		LPNMHDR notifyMessage = (LPNMHDR)lParam;
		TCHAR textBuffer[MAX_PATH];
		LPWSTR text = GetSelectedItemText(index, notifyMessage->hwndFrom, textBuffer);
		PopulateWindowsList(text);
	}
}

void CSetupDialog::WindowsListNotified(LPARAM lParam)
{
	int index = GetSelectedItemIndex(lParam);
	if (index > -1)
	{
		LPNMHDR notifyMessage = (LPNMHDR)lParam;
		TCHAR textBuffer[MAX_PATH];
		LPWSTR text = GetSelectedItemText(index, notifyMessage->hwndFrom, textBuffer);
	}
}

void CSetupDialog::PopulateProcessList(HWND hDlg)
{
	HWND listView = GetDlgItem(hDlg, IDC_LIST_APPS);
	for (auto const &program : *settings->programs)
	{
		LVITEM item;
		CreateListViewItem(program.first, item);
		int result = ListView_InsertItem(listView, &item);
		delete item.pszText;
	}	
}

void CSetupDialog::PopulateWindowsList(LPWSTR itemName)
{
	HWND listView = GetDlgItem(this->hWnd, IDC_LIST_WINDOWS);
	ListView_DeleteAllItems(listView);
	auto program = settings->programs->find(itemName);

	if (program != settings->programs->end())
	{
		for (auto const &window : *program->second->windows)
		{
			LVITEM item;
			CreateListViewItem(window.first, item);
			int result = ListView_InsertItem(listView, &item);
			delete item.pszText;
		}
	}
}

void CSetupDialog::CreateListViewItem(t_string text, LVITEM& item)
{
	std::vector<wchar_t>* textBuffer = new std::vector<wchar_t>(text.begin(), text.end());
	textBuffer->push_back(0); //Add null terminator for string
	SecureZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.pszText = textBuffer->data();
}

LPWSTR CSetupDialog::GetSelectedItemText(int index, HWND hWnd, TCHAR* textBuffer)
{
	LVITEM item;
	SecureZeroMemory(&item, sizeof(item));
	item.iItem = index;
	item.mask = LVIF_TEXT;
	item.cchTextMax = MAX_PATH;
	item.pszText = textBuffer;
	ListView_GetItem(hWnd, &item);
	return item.pszText;
}

int CSetupDialog::GetSelectedItemIndex(LPARAM lParam)
{
	LPNMHDR notifyMessage = (LPNMHDR)lParam;
	int index = -1;
	switch (notifyMessage->code)
	{
	case NM_CLICK:
	{
		LPNMITEMACTIVATE item = (LPNMITEMACTIVATE)lParam;
		index = item->iItem;
	}
	break;

	case LVN_ITEMACTIVATE:
		index = ListView_GetNextItem(notifyMessage->hwndFrom, -1, LVNI_SELECTED);
		break;

	}
	return index;
}