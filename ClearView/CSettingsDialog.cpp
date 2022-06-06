#include "stdafx.h"
#include "CSettingsDialog.h"
#include <windowsx.h>
#include <shlwapi.h>
#include <memory>
#include "ISettingsManager.h"

extern std::unique_ptr<ISettingsManager> settingsManager;

CSettingsDialog::CSettingsDialog(HINSTANCE hInstance) : CModelessDialog(hInstance)
{
}

BOOL CSettingsDialog::SetupDialog()
{
	this->dialogResource = MAKEINTRESOURCE(IDD_SETTINGS);
	return TRUE;
}

INT_PTR CALLBACK CSettingsDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetFormValues(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDOK:
				ApplySettings();
			case IDCANCEL:
				DestroyWindow(hDlg);
				return TRUE;
			break;
		}
		break;
	}
	return FALSE;
}

void CSettingsDialog::ApplySettings()
{
	HWND autoStartupCheckbox = GetDlgItem(hWnd, IDC_CHECKBOX_AUTO_STARTUP);
	if (Button_GetCheck(autoStartupCheckbox))
	{
		AddAutoRun();
	}
	else
	{
		RemoveAutoRun();
	}
	HWND disableUpdateCheck = GetDlgItem(hWnd, IDC_CHECKBOX_DISABLE_UPDATE_CHECK);
	settingsManager->SetDisableUpdateCheck(Button_GetCheck(disableUpdateCheck));
}

void CSettingsDialog::SetFormValues(HWND hDlg)
{
	if (HasAutoRunValue())
	{
		HWND autoStartupCheckbox = GetDlgItem(hDlg, IDC_CHECKBOX_AUTO_STARTUP);
		Button_SetCheck(autoStartupCheckbox, TRUE);
	}
	if (settingsManager->GetDisableUpdateCheck())
	{
		HWND disableUpdateCheckbox = GetDlgItem(hDlg, IDC_CHECKBOX_DISABLE_UPDATE_CHECK);
		Button_SetCheck(disableUpdateCheckbox, TRUE);
	}
}

BOOL CSettingsDialog::HasAutoRunValue()
{
	DWORD keyType;
	TCHAR keyData[MAX_PATH + 1];
	DWORD keyDataSize = sizeof(keyData);
	auto result = SHGetValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), _T("ClearView"), &keyType, keyData, &keyDataSize);
	if (result == ERROR_SUCCESS && keyType == REG_SZ)
	{
		return TRUE;
	}
	return FALSE;
}

void CSettingsDialog::AddAutoRun()
{
	HKEY hKey;
	TCHAR programPath[MAX_PATH];
	GetModuleFileName(0, programPath, sizeof(programPath));
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hKey, _T("ClearView"), 0, REG_SZ, (LPBYTE)programPath, sizeof(programPath)) != ERROR_SUCCESS)
		{
			//Show error message
		}
		RegCloseKey(hKey);
	}
}

void CSettingsDialog::RemoveAutoRun()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		if (RegDeleteValue(hKey, _T("ClearView")) != ERROR_SUCCESS)
		{
			//Show error message
		}
		RegCloseKey(hKey);
	}
}