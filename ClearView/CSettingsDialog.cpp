#include "stdafx.h"
#include "CSettingsDialog.h"
#include <WindowsX.h>

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
}

void CSettingsDialog::SetFormValues(HWND hDlg)
{
	if (HasAutoRunValue())
	{
		HWND autoStartupCheckbox = GetDlgItem(hDlg, IDC_CHECKBOX_AUTO_STARTUP);
		Button_SetCheck(autoStartupCheckbox, TRUE);
	}
}

BOOL CSettingsDialog::HasAutoRunValue()
{
	auto result = RegGetValue(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", L"ClearView", RRF_RT_REG_SZ, NULL, NULL, NULL);
	return result == ERROR_SUCCESS;
}

void CSettingsDialog::AddAutoRun()
{
	HKEY hKey;
	TCHAR programPath[MAX_PATH];
	GetModuleFileName(0, programPath, sizeof(programPath));
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		if (RegSetValueEx(hKey, L"ClearView", 0, REG_SZ, (LPBYTE)programPath, sizeof(programPath)) != ERROR_SUCCESS)
		{
			//Show error message
		}
		RegCloseKey(hKey);
	}
}

void CSettingsDialog::RemoveAutoRun()
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
	{
		if (RegDeleteValue(hKey, L"ClearView") != ERROR_SUCCESS)
		{
			//Show error message
		}
		RegCloseKey(hKey);
	}
}