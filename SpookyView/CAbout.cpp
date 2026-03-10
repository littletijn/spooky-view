#include "stdafx.h"
#include "CAbout.h"

CAbout::CAbout(HINSTANCE hInstance, HWND hParent) : CModelessDialog(hInstance, hParent)
{
};

BOOL CAbout::SetupDialog()
{
	key = IDD_ABOUTBOX;
	this->dialogResource = MAKEINTRESOURCE(IDD_ABOUTBOX);
	return TRUE;
}

BOOL CAbout::Create()
{
	BOOL result = CModelessDialog::Create();
	if (result)
	{
		SetVariant();
	}
	return result;
}

void CAbout::SetVariant()
{
	variant = _T("");
#ifdef _M_IX86 
	variant.append(_T("x86 "));
#elif _M_AMD64
	variant.append(_T("x64 "));
#elif _M_ARM64 
	variant.append(_T("ARM64 "));
#else
	variant.append(_T("Unknown "));
#endif

#ifdef PACKAGING_STORE
	variant.append(_T("Microsoft Store "));
#elif PACKAGING_MSIX
	variant.append(_T("MSIX "));
#elif PACKAGING_MSI
	variant.append(_T("MSI "));
#elif PACKAGING_PORTABLE
	variant.append(_T("Portable "));
#else
	variant.append(_T("Unknown "));
#endif
#ifdef _DEBUG
	variant.append(_T("Debug "));
#endif
#ifdef PRE_RELEASE
	variant.append(_T("Pre-release"));
#endif
	SetDlgItemText(this->hWnd, IDC_STATIC_VARIANT, variant.c_str());
}

INT_PTR CALLBACK CAbout::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			DestroyWindow(hDlg);
			return TRUE;
		}
		break;
	}
	return FALSE;
}