#include "stdafx.h"
#include "CUpdateAvailableDialog.h"
#include <shellapi.h>
#include <memory>
#include "ISettingsManager.h"

extern std::unique_ptr<ISettingsManager> settingsManager;

CUpdateAvailableDialog::CUpdateAvailableDialog(HINSTANCE hInstance, HWND hParent) : CModelessDialog(hInstance, hParent)
{
};

BOOL CUpdateAvailableDialog::SetupDialog()
{
	key = IDD_UPDATE_AVAILABLE;
	this->dialogResource = MAKEINTRESOURCE(IDD_UPDATE_AVAILABLE);
	return TRUE;
}

INT_PTR CALLBACK CUpdateAvailableDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		ShowMessage();
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_SKIP_VERSION:
			settingsManager->AddSkipVersionKey(versionNumber);
			DestroyWindow(hDlg);
			return TRUE;
			break;
		case ID_DOWNLOAD:
			ShellExecute(NULL, _T("open"), this->downloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
			DestroyWindow(hDlg);
			return TRUE;
		case IDCANCEL:
		case ID_CLOSE:
			DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

BOOL CUpdateAvailableDialog::Create()
{
	BOOL result = CModelessDialog::Create();
	if (result)
	{
		ShowMessage();
	}
	return result;
}

void CUpdateAvailableDialog::SetMessage(tstring message)
{
	this->message = message;
}

void CUpdateAvailableDialog::SetDownloadUrl(tstring url)
{
	this->downloadUrl = url;
}

void CUpdateAvailableDialog::SetVersionNumber(tstring versionNumber)
{
	this->versionNumber = versionNumber;
}

void CUpdateAvailableDialog::ShowMessage()
{
	SetDlgItemText(this->hWnd, IDC_UPDATE_MESSAGE, this->message.c_str());
}
