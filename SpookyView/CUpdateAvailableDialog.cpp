#include "stdafx.h"
#include "CUpdateAvailableDialog.h"
#include <shellapi.h>
#include <shlwapi.h>
#include <memory>
#include "ISettingsManager.h"
#include "SpookyView.h"
#include "Logger.h"

namespace
{
	// Only accept https URLs that point at our update host. Anything else
	// coming back from the update server is treated as untrusted: no UNC,
	// no file://, no foreign protocol handlers, no alternate hosts.
	BOOL IsTrustedDownloadUrl(LPCTSTR url)
	{
		if (url == NULL)
		{
			return FALSE;
		}
		static const TCHAR kPrefix[] = _T("https://updates.tyndomyn.net/");
		const size_t prefixLen = _countof(kPrefix) - 1;
		if (_tcslen(url) < prefixLen)
		{
			return FALSE;
		}
		return StrCmpNI(url, kPrefix, (int)prefixLen) == 0;
	}
}

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
#ifdef PACKAGING_STORE
		{
			HWND downloadButton = GetDlgItem(hDlg, ID_DOWNLOAD);
			ShowWindow(downloadButton, SW_HIDE);
		}
#endif // !PACKAGING_STORE
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
#ifndef PACKAGING_STORE
			if (IsTrustedDownloadUrl(this->downloadUrl.c_str()))
			{
				LOG_INFO("Opening trusted update download URL");
				ShellExecute(NULL, _T("open"), this->downloadUrl.c_str(), NULL, NULL, SW_SHOWNORMAL);
			}
			else
			{
				LOG_WARN("Refused untrusted update download URL");
			}
#endif // !PACKAGING_STORE
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
