#include "stdafx.h"
#include "CSetupDialog.h"
#include "CAddAppDialog.h"
#include <CommCtrl.h>
#include "CSettings.h"
#include "CProgramSetting.h"
#include "ISettingsManager.h"
#include "String.h"
#include <vector>

using namespace std;

extern CSettings *settings;
extern ISettingsManager *settingsManager;

CSetupDialog::CSetupDialog(HINSTANCE hInstance) : CModelessDialog(hInstance)
{
}

CSetupDialog::~CSetupDialog()
{
	delete this->appsListView;
	delete this->windowsListView;
}

BOOL CSetupDialog::SetupDialog()
{
	this->dialogResource = MAKEINTRESOURCE(IDD_SETUP);
	return TRUE;
}

INT_PTR CALLBACK CSetupDialog::DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
	{
		LPNMHDR notifyMessage = (LPNMHDR)lParam;
		if (notifyMessage->code == NM_CLICK)
		{
			switch (notifyMessage->idFrom)
			{
			case IDC_LIST_APPS:
				ProgramsListNotified(lParam);
				return TRUE;

			case IDC_LIST_WINDOWS:
				WindowsListNotified(lParam);
				return TRUE;
			}
		}
	}
	break;

	case WM_HSCROLL:
		if (LOWORD(wParam) == TB_THUMBPOSITION || LOWORD(wParam) == TB_THUMBTRACK)
		{
			WORD value = HIWORD(wParam);
			SetAlpha(value, (HWND)lParam);
		}
		return TRUE;

	case WM_INITDIALOG:
		//Create the listviews
		this->appsListView = new ListView(hDlg, IDC_LIST_APPS);
		this->windowsListView = new ListView(hDlg, IDC_LIST_WINDOWS);

		PopulateProcessList(hDlg);
		SetTrackbarRanges(hDlg);
		//Set the trackbars on the global settings
		this->currentAlphaSettings = &settings->alphaSettings;
		SetTrackbars();
		return TRUE;
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			case IDAPPLY:
				settingsManager->SaveSettings(settings);
				return TRUE;
			case IDOK:
				settingsManager->SaveSettings(settings);
			case IDCANCEL:
				DestroyWindow(hDlg);
				return TRUE;
			case IDC_BUTTON_APP_ADD:
				CAddAppDialog *appDialog = new CAddAppDialog(this->hInstance, this->hWnd);
				appDialog->InitInstance();
				if (appDialog->GetResult() == 1)
				{

				}
				return TRUE;
		}
		break;
	}
	return FALSE;
}

void CSetupDialog::ProgramsListNotified(LPARAM lParam)
{
	int index = this->appsListView->GetSelectedIndex(lParam);
	if (index > -1)
	{
		TCHAR textBuffer[MAX_PATH];
		LPWSTR text = this->appsListView->GetTextByIndex(index, textBuffer);

		auto program = settings->programs->find(text);
		if (program != settings->programs->end())
		{
			this->currentProgram = program->second;
			this->currentAlphaSettings = &program->second->alphaSettings;
			SetTrackbars();
		}
		PopulateWindowsList(program->second);
	}
	else
	{
		//When no item is selected, get the global settings
		this->currentAlphaSettings = &settings->alphaSettings;
		SetTrackbars();
	}
}

void CSetupDialog::WindowsListNotified(LPARAM lParam)
{
	int index = this->windowsListView->GetSelectedIndex(lParam);
	if (index > -1)
	{
		TCHAR textBuffer[MAX_PATH];
		LPWSTR text = this->windowsListView->GetTextByIndex(index, textBuffer);

		auto window = this->currentProgram->windows->find(text);
		if (window != this->currentProgram->windows->end())
		{
			this->currentAlphaSettings = &window->second->alphaSettings;
			SetTrackbars();
		}
	}
	else
	{
		//When no item is selected, get the program global settings
		this->currentAlphaSettings = &this->currentProgram->alphaSettings;
		SetTrackbars();
	}
}

void CSetupDialog::PopulateProcessList(HWND hDlg)
{
	for (auto const &program : *settings->programs)
	{
		this->appsListView->AddItem(program.first);
	}	
}

void CSetupDialog::PopulateWindowsList(CProgramSetting* settings)
{
	this->windowsListView->DeleteAllItems();
	for (auto const &window : *settings->windows)
	{
		this->windowsListView->AddItem(window.first);
	}
}

void CSetupDialog::SetTrackbarRanges(HWND hWnd)
{
	HWND foregroundTrackbar = GetDlgItem(hWnd, IDC_SLIDER_FOREGROUND);
	HWND backgroundTrackbar = GetDlgItem(hWnd, IDC_SLIDER_BACKGROUND);
	SendMessage(foregroundTrackbar, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)255);
	SendMessage(backgroundTrackbar, TBM_SETRANGEMAX, (WPARAM)TRUE, (LPARAM)255);
}

void CSetupDialog::SetTrackbars()
{
	HWND foregroundTrackbar = GetDlgItem(this->hWnd, IDC_SLIDER_FOREGROUND);
	HWND backgroundTrackbar = GetDlgItem(this->hWnd, IDC_SLIDER_BACKGROUND);
	SendMessage(foregroundTrackbar, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)this->currentAlphaSettings->foreground);
	SendMessage(backgroundTrackbar, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)this->currentAlphaSettings->background);
}

void CSetupDialog::SetAlpha(WORD value, HWND trackbar)
{
	LONG identifier = GetWindowLong(trackbar, GWL_ID);
	switch (identifier)
	{
	case IDC_SLIDER_FOREGROUND:
		this->currentAlphaSettings->foreground = value;
		break;
	case IDC_SLIDER_BACKGROUND:
		this->currentAlphaSettings->background = value;
		break;
	}
}