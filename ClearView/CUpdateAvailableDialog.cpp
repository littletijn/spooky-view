#include "stdafx.h"
#include "CUpdateAvailableDialog.h"
#include <shellapi.h>

CUpdateAvailableDialog::CUpdateAvailableDialog(HINSTANCE hInstance) : CModelessDialog(hInstance)
{
};

BOOL CUpdateAvailableDialog::SetupDialog()
{
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
		case ID_DOWNLOAD:
			ShellExecute(NULL, L"open", this->downloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
		case IDCANCEL:
		case ID_CLOSE:
			ShowMessage();
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

void CUpdateAvailableDialog::ShowMessage()
{
	SetDlgItemText(this->hWnd, IDC_UPDATE_MESSAGE, this->message.c_str());
}
