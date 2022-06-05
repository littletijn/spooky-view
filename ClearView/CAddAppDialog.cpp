#include "stdafx.h"
#include "CAddAppDialog.h"
#include "Tlhelp32.h"
#include "Commctrl.h"
#include "strsafe.h"
#include <string.h>

CAddAppDialog::CAddAppDialog(HINSTANCE hInstance, HWND hParent) : CModalDialog(hInstance, hParent)
{
}


CAddAppDialog::~CAddAppDialog()
{
}

BOOL CAddAppDialog::SetupDialog()
{
	this->dialogResource = MAKEINTRESOURCE(IDD_ADD_APP);
	return TRUE;
}

/*
Handles Window Messages
*/
INT_PTR CALLBACK CAddAppDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	this->hWnd = hDlg;
	switch (message)
	{
	case WM_INITDIALOG:
		this->appsListView = std::make_unique<ListView>(hDlg, IDC_LIST_ADD_APPS);
		LoadModules();
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			StoreSelectedProcess();
			EndDialog(hDlg, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hDlg, 2);
			return TRUE;

		}
		case IDC_BUTTON_BROWSE:
			BrowseFile();
			return TRUE;
		break;
	}
	return FALSE;
}

/*
Load the list of programs currently running
*/
void CAddAppDialog::LoadModules()
{
	PROCESSENTRY32 sProcess;
	sProcess.dwSize = sizeof(PROCESSENTRY32);

	HANDLE hProcessesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(hProcessesSnapshot, &sProcess))
	{
		do
		{
			this->AddProcessToList(sProcess.szExeFile);
		} while (Process32Next(hProcessesSnapshot, &sProcess));
	}

	CloseHandle(hProcessesSnapshot);
}

void CAddAppDialog::BrowseFile()
{
}

void CAddAppDialog::BrowseFile()
{
}

void CAddAppDialog::StoreSelectedProcess()
{
	int index = this->appsListView->GetSelectedIndex();
	TCHAR textBuffer[MAX_PATH];
	LPWSTR text = this->appsListView->GetTextByIndex(index, textBuffer);
	this->selectedProcess = std::unique_ptr<TCHAR[]>(new TCHAR[MAX_PATH]);
	StringCchCopy(this->selectedProcess.get(), MAX_PATH, textBuffer);
}

LPWSTR CAddAppDialog::GetSelectedProcess()
{
	return this->selectedProcess.get();
}

void CAddAppDialog::AddProcessToList(WCHAR *exeName)
{
	this->appsListView->AddItem(exeName);
}
