#include "stdafx.h"
#include "CAddAppDialog.h"
#include "tlhelp32.h"
#include "commctrl.h"
#include "strsafe.h"
#include <string.h>
#include "Unicode.h"
#include <unordered_set>
#include <map>

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
		this->appsListView->InsertColumn(0, _T("File"));
		this->appsListView->InsertColumn(1, _T("Name"));
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
		std::map<tstring, tstring> programs;
		do
		{
			t_string programName;
			GetProcessProgramName(sProcess, &programName);
			programs.insert(std::pair<t_string, t_string>(tstring(sProcess.szExeFile), programName));
		} while (Process32Next(hProcessesSnapshot, &sProcess));
		for (auto program = programs.begin(); program != programs.end(); ++program)
		{
			int itemIndex = this->appsListView->AddItem(program->first.c_str());
			this->appsListView->SetItem(itemIndex, 1, program->second.c_str());
		}
	}

	CloseHandle(hProcessesSnapshot);
}

void CAddAppDialog::BrowseFile()
{
}

void CAddAppDialog::StoreSelectedProcess()
{
	int index = this->appsListView->GetSelectedIndex();
	TCHAR textBuffer[MAX_PATH];
	LPTSTR text = this->appsListView->GetTextByIndex(index, textBuffer);
	this->selectedProcess = std::unique_ptr<TCHAR[]>(new TCHAR[MAX_PATH]);
	StringCchCopy(this->selectedProcess.get(), MAX_PATH, textBuffer);
}

LPTSTR CAddAppDialog::GetSelectedProcess()
{
	return this->selectedProcess.get();
}

void CAddAppDialog::GetProcessProgramName(PROCESSENTRY32 sProcess, t_string* programName)
{
	MODULEENTRY32 sModule;
	sModule.dwSize = sizeof(MODULEENTRY32);

	struct LANGANDCODEPAGE {
		WORD wLanguage;
		WORD wCodePage;
	} *lpTranslate;

	LPDWORD dwDummy = 0;
	DWORD dDummyHandle = 0;
	UINT dwBytes;
	BOOL module32FirstResult;


	HANDLE hModulesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, sProcess.th32ProcessID);
	do {
		module32FirstResult = Module32First(hModulesSnapshot, &sModule);
	} while (GetLastError() == ERROR_BAD_LENGTH);

	if (module32FirstResult)
	{
		DWORD dInfoSize = GetFileVersionInfoSize(sModule.szExePath, dwDummy);
		if (dInfoSize)
		{
			//Create buffer for storing versionInfo
			std::unique_ptr<BYTE[]> versionInfoBuffer(new BYTE[dInfoSize]);
			if (GetFileVersionInfo(sModule.szExePath, dDummyHandle, dInfoSize, versionInfoBuffer.get()))
			{
				//Get translations of module
				if (VerQueryValue(versionInfoBuffer.get(), _T("\\VarFileInfo\\Translation"), (LPVOID*)&lpTranslate, &dwBytes))
				{
					//SubBlock contains name of value to retrieve
					TCHAR subBlock[200];
					size_t subBlockSize = 200;
					HRESULT hr;

					//Loop trough each translation
					for (UINT i = 0; i < (dwBytes / sizeof(struct LANGANDCODEPAGE)); i++)
					{
						//Put correct path to FileDescription in subBlock
						hr = StringCchPrintf(
							subBlock,
							subBlockSize,
							_T("\\StringFileInfo\\%04x%04x\\FileDescription"),
							lpTranslate[i].wLanguage,
							lpTranslate[i].wCodePage
						);
						if (!FAILED(hr))
						{
							UINT programNameBufferSize;
							LPVOID programNameBuffer;
							//Try to read value of path in subBlock
							if (VerQueryValue(versionInfoBuffer.get(), subBlock, &programNameBuffer, &programNameBufferSize))
							{
								//We got the name of the program!
								programName->append(reinterpret_cast<TCHAR*>(programNameBuffer));
							}
						}
					}
				}
			}
		}
	}
	else
	{
		//MessageBox(0, _T("Broke"), _T("Broke"), MB_OK);
	}
}
