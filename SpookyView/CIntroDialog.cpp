#include "stdafx.h"
#include "CIntroDialog.h"
#include <WindowsX.h>
#include "ISettingsManager.h"
#include "SpookyView.h"

CIntroDialog::CIntroDialog(HINSTANCE hInstance, HWND hParent) : CModelessDialog(hInstance, hParent)
{
};

BOOL CIntroDialog::SetupDialog()
{
	key = IDD_INTRO;
	this->dialogResource = MAKEINTRESOURCE(IDD_INTRO);
	return TRUE;
}

INT_PTR CALLBACK CIntroDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetupWindow(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDCANCEL:
			case IDCLOSE:
				ApplyChoice();
				DestroyWindow(hDlg);
				return TRUE;
			case IDC_BUTTON_INTRO_SETTINGS:
				if (!cSettingsDialog)
				{
					cSettingsDialog = std::make_unique<CSettingsDialog>(this->hInstance, mainHwnd);
				}
				if (!cSettingsDialog->hasInitInstance())
				{
					cSettingsDialog->InitInstance();
				}
				else
				{
					cSettingsDialog->SetForeground();
				}
				break;
			case IDC_BUTTON_SETUP_APPS:
				if (!cSetupDialog)
				{
					cSetupDialog = std::make_unique<CSetupDialog>(this->hInstance, mainHwnd);
				}
				if (!cSetupDialog->hasInitInstance())
				{
					cSetupDialog->InitInstance();
				}
				else
				{
					cSetupDialog->SetForeground();
				}
			break;
		}
		break;
	}
	return FALSE;
}

void CIntroDialog::SetupWindow(HWND hDlg)
{
	//Set font of header
	ZeroMemory(&introFont, sizeof(LOGFONT));
	introFont.lfHeight = 20;
	introFont.lfWeight = FW_BOLD;
	HFONT hFont = CreateFontIndirect(&introFont);
	if ((HFONT)0 != hFont)
	{
		HWND introStaticText = GetDlgItem(hDlg, ID_STATIC_INTRO_TITLE);
		SetWindowFont(introStaticText, hFont, TRUE);
	}
	//Set state of checkbox
	HWND skipIntroCheckbox = GetDlgItem(hDlg, IDC_SKIP_INTRO_CHECKBOX);
	if (settingsManager->GetSkipWelcome())
	{
		Button_SetCheck(skipIntroCheckbox, TRUE);
	}
}

void CIntroDialog::ApplyChoice()
{
	HWND skipIntroCheckbox = GetDlgItem(hWnd, IDC_SKIP_INTRO_CHECKBOX);
	if (Button_GetCheck(skipIntroCheckbox))
	{
		settingsManager->SetSkipWelcome(TRUE);
	}
}
