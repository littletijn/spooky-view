#include "stdafx.h"
#include "CSetupDialog.h"
#include "CWnd.h"
#include "Tlhelp32.h"

CSetupDialog::CSetupDialog(HINSTANCE hInstance) : CDialog(hInstance)
{
}

CSetupDialog::~CSetupDialog()
{
}

BOOL CSetupDialog::InitInstance()
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
				//this->AddProcessToList(&sModule);
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
	return TRUE;
}
