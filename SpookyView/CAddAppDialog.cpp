#include "stdafx.h"
#include "CAddAppDialog.h"
#include "tlhelp32.h"
#include "commctrl.h"
#include "strsafe.h"
#include <string.h>
#include "Unicode.h"
#include <unordered_set>
#include <map>
#include "Textbox.h"
#include <commdlg.h>
#include "SpookyView.h"
#include "WindowsEnum.h"
#include <winnls.h>

extern PGNSI isImmersive;
extern WindowsEnum windowsEnum;

const int US_LANGUAGE_CODE = 1033; // English (United States)

CAddAppDialog::CAddAppDialog(HINSTANCE hInstance, HWND hParent) : CModalDialog(hInstance, hParent)
{
	currentUserLanguage = GetUserDefaultUILanguage();
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
		this->programTextbox = std::make_unique<Textbox>(hDlg, IDC_EDIT_EXECUTABLE_NAME);
		LoadModules();
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
				SetSelectedProgram();
				return TRUE;
			}
		}
	}
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
		case IDC_BUTTON_BROWSE:
			BrowseFile();
			return TRUE;
			break;
		}
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
			if (sProcess.th32ProcessID == 0) {
				continue;
			}
			BOOL hasUsableCoreWindow = FALSE;
			if (isImmersive != NULL)
			{
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, sProcess.th32ProcessID);
				if (hProcess != NULL)
				{
					if (isImmersive(hProcess))
					{
						// Check if current process is a Windows Store App on Windows 8 or Windows 8.1.
						// They should not be transparent because they are always running full-screen

						// When not Windows 8 or Windows 8.1, check if it is the ApplicationFrameHost. If it is, skip it.
						// ApplicationFrameHost does not have a window of it own. 
						// All child windows of the ApplicationFrameHost process with class "Windows.UI.Core.CoreWindow" are the windows of the UWP apps running and visible.
						if (WindowsEnum::isWindows8 || IsApplicationFrameHost(sProcess))
						{
							CloseHandle(hProcess);
							continue;
						}
						//Check if the UWP application has a Windows.UI.Core.CoreWindow open
						else if (WindowsEnum::HasProcessUWPCoreWindow(sProcess.th32ProcessID))
						{
							hasUsableCoreWindow = TRUE;
						}
					}
					CloseHandle(hProcess);
				}
			}
			//Check if the process has usable windows
			if (!hasUsableCoreWindow && !windowsEnum.HasProcessUsableWindows(sProcess.th32ProcessID))
			{
				continue;
			}
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

BOOL CAddAppDialog::IsApplicationFrameHost(PROCESSENTRY32 sProcess)
{
	return _tcsicmp(sProcess.szExeFile, _T("ApplicationFrameHost.exe")) == 0;
}

void CAddAppDialog::BrowseFile()
{
	TCHAR fileName[MAX_PATH];
	TCHAR filePath[MAX_PATH];
	SecureZeroMemory(fileName, MAX_PATH);
	SecureZeroMemory(filePath, MAX_PATH);
	OPENFILENAME ofn;
	SecureZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.nMaxFile = MAX_PATH - 1;
	ofn.lpstrFile = filePath;
	ofn.lpstrFileTitle = fileName;
	ofn.nMaxFileTitle = MAX_PATH - 1;
	ofn.lpstrTitle = _T("Select program");
	ofn.lpstrFilter = _T("Executables (*.exe)\0*.exe\0All files (*.*)\0*.*\0");
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrDefExt = _T("exe");
	ofn.Flags = OFN_FILEMUSTEXIST;
	if (GetOpenFileName(&ofn))
	{
		programTextbox->SetText(fileName);
	}
	else
	{
		auto errorCode = CommDlgExtendedError();
	}

}

void CAddAppDialog::SetSelectedProgram()
{
	int index = this->appsListView->GetSelectedIndex();
	if (index >= 0)
	{
		TCHAR textBuffer[MAX_PATH];
		LPTSTR text = this->appsListView->GetTextByIndex(index, textBuffer);
		this->programTextbox->SetText(text);
	}
}

void CAddAppDialog::StoreSelectedProcess()
{
	int textLength = 0;
	auto textBuffer = this->programTextbox->GetText(&textLength);
	this->selectedProcess = std::make_unique<TCHAR[]>(textLength + 1);
	StringCchCopy(this->selectedProcess.get(), textLength + 1, textBuffer.get());
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
	t_string foundProgramName;

	HANDLE hModulesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, sProcess.th32ProcessID);
	if (hModulesSnapshot == INVALID_HANDLE_VALUE) {
		return;
	}
	do {
		module32FirstResult = Module32First(hModulesSnapshot, &sModule);
	} while (GetLastError() == ERROR_BAD_LENGTH);

	if (module32FirstResult)
	{
		DWORD dInfoSize = GetFileVersionInfoSize(sModule.szExePath, dwDummy);
		if (dInfoSize)
		{
			//Create buffer for storing versionInfo
			auto versionInfoBuffer = std::make_unique<BYTE[]>(dInfoSize);
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
								//We got the name of the program! Return it or store it as one of the fallback languages
								if (lpTranslate[i].wLanguage == currentUserLanguage)
								{
									//We got the translation in the user language. Break loop and use this value
									foundProgramName.clear();
									foundProgramName.append(reinterpret_cast<TCHAR*>(programNameBuffer));
									break;
								}
								else if (lpTranslate[i].wLanguage == 0) //Neutral language
								{
									//We got the neutral translation. Store it as the fallback translation when a translation in the user language does not exists
									foundProgramName.clear();
									foundProgramName.append(reinterpret_cast<TCHAR*>(programNameBuffer));
								}
								else if (lpTranslate[i].wLanguage == US_LANGUAGE_CODE && foundProgramName.length() == 0)
								{
									//We got the US translation. Store it as the fallback translation when a translation in the user language or neutral does not exists
									foundProgramName.append(reinterpret_cast<TCHAR*>(programNameBuffer));
								}
							}
						}
					}
				}
			}
		}
	}
	CloseHandle(hModulesSnapshot);
	if (foundProgramName.length() > 0)
	{
		programName->append(foundProgramName);
	}
}
