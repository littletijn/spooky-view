#include "stdafx.h"
#include "CAddAppDialog.h"
#include "Tlhelp32.h"
#include "Commctrl.h"

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

INT_PTR CALLBACK CAddAppDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	this->hWnd = hDlg;
	switch (message)
	{
	case WM_INITDIALOG:
		LoadModules();
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return FALSE;

		}
		case IDC_BUTTON_BROWSE:
			BrowseFile();
			return TRUE;
		break;
	}
	return FALSE;
}


void CAddAppDialog::LoadModules()
{
	PROCESSENTRY32 sProcess = { sizeof(PROCESSENTRY32) };
	MODULEENTRY32 sModule = { sizeof(MODULEENTRY32) };

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	LPDWORD dwDummy = 0;
	DWORD dDummyHandle = 0;
	UINT dwBytes;

	HANDLE hProcessesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(hProcessesSnapshot, &sProcess))
	{
		do
		{
			HANDLE hModulesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, sProcess.th32ProcessID);
			if (Module32First(hModulesSnapshot, &sModule))
			{
				this->AddProcessToList(&sModule);
				DWORD dInfoSize = GetFileVersionInfoSize(sModule.szExePath, dwDummy);
				if (dInfoSize){
					LPVOID lpVersionInfo = new BYTE[dInfoSize];
					if (GetFileVersionInfo(sModule.szExePath, dDummyHandle, dInfoSize, lpVersionInfo))
					{
						if (VerQueryValue(lpVersionInfo, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &dwBytes))
						{

						}
						else
						{
							MessageBox(this->hWnd, _T("Function <VerQueryValue> for Translation unsuccessful!"), _T("ERROR!"), IDOK);
						}
					}
				}

			}
			CloseHandle(hModulesSnapshot);
		} while (Process32Next(hProcessesSnapshot, &sProcess));
	}

	CloseHandle(hProcessesSnapshot);
}

void CAddAppDialog::BrowseFile()
{
}

void CAddAppDialog::AddProcessToList(MODULEENTRY32 *module)
{
	HWND listView = GetDlgItem(this->hWnd, IDC_LIST_ADD_APPS);
	LVITEM item;
	SecureZeroMemory(&item, sizeof(item));
	item.mask = LVIF_TEXT;
	item.pszText = module->szModule;
	int result = ListView_InsertItem(listView, &item);
}
