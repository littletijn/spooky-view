#include "stdafx.h"
#include "CAddAppDialog.h"
#include "Tlhelp32.h"
#include "Commctrl.h"
#include "ListView.h"
#include "strsafe.h"

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
		this->appsListView = new ListView(hDlg, IDC_LIST_APPS);
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

/*
Load the list of programs currently running
*/
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
				TCHAR *programName = NULL;
				DWORD dInfoSize = GetFileVersionInfoSize(sModule.szExePath, dwDummy);
				if (dInfoSize)
				{
					LPVOID lpVersionInfo = new BYTE[dInfoSize];
					if (GetFileVersionInfo(sModule.szExePath, dDummyHandle, dInfoSize, lpVersionInfo))
					{
						//Get translations of module
						if (VerQueryValue(lpVersionInfo, TEXT("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &dwBytes))
						{
							//SubBlock contains name of value to retrieve
							TCHAR subBlock[200];
							size_t subBlockSize = 200;
							HRESULT hr;

							//Loop trough each translation
							for (int i = 0; i < (dwBytes / sizeof(struct LANGANDCODEPAGE)); i++)
							{
								//Put correct path to FileDescription in subBlock
								hr = StringCchPrintf(
									subBlock, 
									subBlockSize, 
									TEXT("\\StringFileInfo\\%04x%04x\\FileDescription"),
									lpTranslate[i].wLanguage,
									lpTranslate[i].wCodePage
								);
								if (!FAILED(hr))
								{
									UINT programNameBufferSize;
									LPVOID programNameBuffer;
									//Try to read value of path in subBlock
									if (VerQueryValue(lpVersionInfo, subBlock, &programNameBuffer, &programNameBufferSize))
									{
										//We got the name of the program!
										programName = reinterpret_cast<TCHAR *>(programNameBuffer);
									}
								}
							}
						}
					}
					
				}
				this->AddProcessToList(&sModule, programName);
			}
			CloseHandle(hModulesSnapshot);
		} while (Process32Next(hProcessesSnapshot, &sProcess));
	}

	CloseHandle(hProcessesSnapshot);
}

/*
Open dialog to browse for a program file to add
*/
void CAddAppDialog::BrowseFile()
{
}

/*
Add process with given module name and given program name to list of programs in dialog
*/
void CAddAppDialog::AddProcessToList(MODULEENTRY32 *module, TCHAR *programName)
{
	int result = this->appsListView->AddItem(module->szModule);
}
