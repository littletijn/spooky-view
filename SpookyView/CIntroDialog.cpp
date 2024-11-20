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
		SetStyle(hDlg);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
			case IDCLOSE:
				ApplyChoice();
				DestroyWindow(hDlg);
				return TRUE;
				break;
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
		}
		break;
	}
	return FALSE;
}

void CIntroDialog::SetStyle(HWND hDlg)
{
	ZeroMemory(&introFont, sizeof(LOGFONT));
	introFont.lfHeight = 20;
	introFont.lfWeight = FW_BOLD;
	HFONT hFont = CreateFontIndirect(&introFont);
	if ((HFONT)0 != hFont)
	{
		HWND introStaticText = GetDlgItem(hDlg, ID_STATIC_INTRO_TITLE);
		SetWindowFont(introStaticText, hFont, TRUE);
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
