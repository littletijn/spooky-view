#include "stdafx.h"
#include "CSetupDialog.h"
#include "CAddAppDialog.h"
#include "CAddWindowDialog.h"
#include <commctrl.h>
#include "CSettings.h"
#include "CProgramSetting.h"
#include "ISettingsManager.h"
#include "MultiPlatformString.h"
#include <vector>
#include <memory>
#include <windowsx.h>
#include "Defines.h"
#include "WindowsEnum.h"
#include <cmath>
#include "SpookyView.h"

#define TRANSPARENCY_TRACKER_STEPS (100.0 / 255.0)

CSetupDialog::CSetupDialog(HINSTANCE hInstance, HWND hParent) : CModelessDialog(hInstance, hParent)
{
	this->CopySettings();
}

CSetupDialog::~CSetupDialog()
{
}

BOOL CSetupDialog::SetupDialog()
{
	LoadString(hInst, IDS_ALL_OTHER_APPS, allOtherAppsString, sizeof(allOtherAppsString) / sizeof(TCHAR));
	LoadString(hInst, IDS_ALL_OTHER_WINDOWS, allOtherWindowsString, sizeof(allOtherWindowsString) / sizeof(TCHAR));
	key = IDD_SETUP;
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
		if (notifyMessage->code == LVN_ITEMCHANGED)
		{
			switch (notifyMessage->idFrom)
			{
			case IDC_LIST_APPS:
				ProgramsListNotified();
				return TRUE;

			case IDC_LIST_WINDOWS:
				WindowsListNotified();
				return TRUE;
			}
		}
	}
	break;

	case WM_HSCROLL:
	{
		auto senderHwnd = (HWND)lParam;
		LONG identifier = GetWindowLong(senderHwnd, GWL_ID);
		if (identifier == IDC_SLIDER_FOREGROUND || identifier == IDC_SLIDER_BACKGROUND)
		{
			if (LOWORD(wParam) == TB_THUMBPOSITION || LOWORD(wParam) == TB_THUMBTRACK)
			{
				BYTE value = HIWORD(wParam) & 0xff;
				SetAlpha(value, senderHwnd);
			}
			else
			{
				BYTE value = SendMessage(senderHwnd, TBM_GETPOS, 0, 0) & 0xff;
				SetAlpha(value, senderHwnd);
			}
		}
		return TRUE;
	}

	case WM_INITDIALOG:
		//Create the listviews
		appsListView = std::make_unique<ListView>(hDlg, IDC_LIST_APPS);
		windowsListView = std::make_unique<ListView>(hDlg, IDC_LIST_WINDOWS);
		enabledCheckbox = std::make_unique<Checkbox>(hDlg, IDC_CHECKBOX_ENABLE_TRANSPARENCY);
		separateBackgroundValueCheckbox = std::make_unique<Checkbox>(hDlg, IDC_CHECKBOX_SEPARATE_BACKGROUND_VALUE);

		PopulateProcessList();
		SetTrackbarRanges(hDlg);
		//Set the trackbars on the global settings
		this->currentAlphaSettings = &newSettings->alphaSettings;
		SetTrackbars();
		SetCheckboxes();
		return TRUE;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
		{
			switch (LOWORD(wParam))
			{
			case IDC_CHECKBOX_ENABLE_TRANSPARENCY:
				EnabledCheckboxNotified();
				return TRUE;
			case IDC_CHECKBOX_SEPARATE_BACKGROUND_VALUE:
				SeparateBackgroundValueCheckboxNotified();
				return TRUE;
			case IDAPPLY:
				ApplySettings();
				return TRUE;
			case IDOK:
				if (ApplySettings())
				{
					DestroyWindow(hDlg);
				}
				return TRUE;
			case IDCANCEL:
				DestroyWindow(hDlg);
				return TRUE;
			case IDC_BUTTON_APP_REMOVE:
				newSettings->programs->erase(this->currentProgramName);
				this->appsListView->DeleteSelectedItem();
				return TRUE;
			case IDC_BUTTON_WINDOW_REMOVE:
				this->currentProgram->windows->erase(this->currentWindowClassName);
				this->windowsListView->DeleteSelectedItem();
				return TRUE;
			case IDC_BUTTON_APP_ADD:
				{
					auto appDialog = std::make_unique<CAddAppDialog>(this->hInstance, this->hWnd);
					appDialog->InitInstance();
					if (appDialog->GetResult() == 1)
					{
						LPTSTR programName = appDialog->GetSelectedProcess();
						//Make process name lower case
						auto lowerCaseProgramName = newSettings->ToLowerCase(programName);
						auto existingProgram = newSettings->programs->find(*lowerCaseProgramName);
						if (existingProgram == newSettings->programs->end())
						{
							newSettings->programs->insert(std::pair<t_string, std::unique_ptr<CProgramSetting>>(*lowerCaseProgramName, std::make_unique<CProgramSetting>()));
							this->appsListView->AddItem(*lowerCaseProgramName);
						}
					}
				}
				return TRUE;
			case IDC_BUTTON_WINDOW_ADD:
				{
					auto windowDialog = std::make_unique<CAddWindowDialog>(this->hInstance, this->hWnd);
					windowDialog->InitInstance(this->currentProgramName);
					if (windowDialog->GetResult() == 1)
					{
						auto windowClassName = windowDialog->GetSelectedWindowClass();
						auto existingWindowClassName = this->currentProgram->windows->find(windowClassName);
						if (existingWindowClassName == this->currentProgram->windows->end())
						{
							this->currentProgram->windows->insert(std::pair<t_string, std::unique_ptr<CWindowSetting>>(windowClassName, std::make_unique<CWindowSetting>()));
							this->windowsListView->AddItem(windowClassName);
						}
					}
				}
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

bool CSetupDialog::ApplySettings()
{
	windowsEnum.ResetWindowsTransparency();
	settingsManager->ApplyNewSettings(newSettings.get());
	if (!settingsManager->SaveSettings())
	{
		TCHAR titleString[10];
		TCHAR messageString[80];
		LoadString(hInst, IDS_ERROR_TITLE, titleString, sizeof(titleString) / sizeof(TCHAR));
		LoadString(hInst, IDS_ERROR_CANNOT_SAVE_SETTINGS, messageString, sizeof(messageString) / sizeof(TCHAR));
		MessageBox(NULL, messageString, titleString, MB_OK | MB_ICONWARNING);
		return false;
	}
	WindowsEnum::SetWindowsTransparency();
	return true;
}

void CSetupDialog::CopySettings()
{
	newSettings = std::make_unique<CSettings>(*settingsManager->GetSettings());
}

void CSetupDialog::ProgramsListNotified()
{
	int index = this->appsListView->GetSelectedIndex();
	if (index > 0)
	{
		TCHAR textBuffer[MAX_PATH];
		LPTSTR text = this->appsListView->GetTextByIndex(index, textBuffer);

		auto program = newSettings->programs->find(text);
		if (program != newSettings->programs->end())
		{
			this->currentProgramName = program->first;
			this->currentProgram = program->second.get();
			this->currentAlphaSettings = &program->second->alphaSettings;
			SetTrackbars();
			SetCheckboxes();
			PopulateWindowsList(program->second.get());
		}
		this->SetFormVisibility(TRUE);
	}
	else if (index == 0)
	{
		//When first item (all programs) is selected, get the global settings
		this->currentProgram = 0;
		this->currentAlphaSettings = &newSettings->alphaSettings;
		SetTrackbars();
		SetCheckboxes();
		PopulateWindowsList();
		this->SetFormVisibility(TRUE);
	} 
	else
	{
		//When no item is selected, hide the controls and clear windows list
		this->SetFormVisibility(FALSE);
		this->windowsListView->DeleteAllItems();
		SetButtonEnableState(IDC_BUTTON_WINDOW_ADD, false);
		SetButtonEnableState(IDC_BUTTON_WINDOW_REMOVE, false);
	}
	if (index >= 0) {
		//Select the first item available
		this->windowsListView->SetSelectedItem(0);
	}
	//Enable the remove button when a program is selected
	SetButtonEnableState(IDC_BUTTON_APP_REMOVE, index > 0);
}

void CSetupDialog::WindowsListNotified()
{
	int index = this->windowsListView->GetSelectedIndex();
	if (index > 0)
	{
		TCHAR textBuffer[MAX_WINDOW_CLASS_NAME];
		LPTSTR text = this->windowsListView->GetTextByIndex(index, textBuffer);

		auto window = currentProgram->windows->find(text);
		if (window != currentProgram->windows->end())
		{
			this->currentWindowClassName = window->first;
			this->currentAlphaSettings = &window->second->alphaSettings;
			SetTrackbars();
			SetCheckboxes();
		}
		this->SetFormVisibility(TRUE);
	}
	else if (index == 0)
	{
		//When first item (all windows) is selected, get the program global settings
		//If no program is selected, get the global settings
		if (this->currentProgram) {
			this->currentAlphaSettings = &currentProgram->alphaSettings;
		}
		else {
			this->currentAlphaSettings = &newSettings->alphaSettings;
		}
		SetTrackbars();
		SetCheckboxes();
		this->SetFormVisibility(TRUE);
	}
	else
	{
		//When no item is selected, hide the form
		this->SetFormVisibility(FALSE);
	}
	//Enable the remove button when a window is selected
	SetButtonEnableState(IDC_BUTTON_WINDOW_REMOVE, index > 0);
	//When a program is selected, enable the add button
	if (this->currentProgram) {
		SetButtonEnableState(IDC_BUTTON_WINDOW_ADD, true);
	}
	else
	{
		SetButtonEnableState(IDC_BUTTON_WINDOW_ADD, false);
	}
}

void CSetupDialog::EnabledCheckboxNotified()
{
	currentAlphaSettings->enabled = enabledCheckbox->GetCheckState();
	SetFormElementsEnableState();
}

void CSetupDialog::SeparateBackgroundValueCheckboxNotified()
{
	currentAlphaSettings->separateBackgroundValue = separateBackgroundValueCheckbox->GetCheckState();
	SetFormElementsEnableState();
}

void CSetupDialog::PopulateProcessList()
{
	this->appsListView->AddItem(allOtherAppsString);
	for (auto const& program : *newSettings->programs.get())
	{
		this->appsListView->AddItem(program.first);
	}
}

void CSetupDialog::PopulateWindowsList(CProgramSetting* settings)
{
	this->windowsListView->DeleteAllItems();
	this->windowsListView->AddItem(allOtherWindowsString);
	if (settings) {
		for (auto const& window : *settings->windows)
		{
			this->windowsListView->AddItem(window.first);
		}
	}
}

void CSetupDialog::SetTrackbarRanges(HWND hWnd)
{
	HWND foregroundTrackbar = GetDlgItem(hWnd, IDC_SLIDER_FOREGROUND);
	HWND backgroundTrackbar = GetDlgItem(hWnd, IDC_SLIDER_BACKGROUND);
	SendMessage(foregroundTrackbar, TBM_SETRANGEMAX, (WPARAM)FALSE, (LPARAM)100);
	SendMessage(backgroundTrackbar, TBM_SETRANGEMAX, (WPARAM)FALSE, (LPARAM)100);
	SendMessage(foregroundTrackbar, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)10);
	SendMessage(backgroundTrackbar, TBM_SETRANGEMIN, (WPARAM)TRUE, (LPARAM)10);
}

void CSetupDialog::SetTrackbars()
{
	HWND foregroundTrackbar = GetDlgItem(this->hWnd, IDC_SLIDER_FOREGROUND);
	HWND backgroundTrackbar = GetDlgItem(this->hWnd, IDC_SLIDER_BACKGROUND);
	SendMessage(foregroundTrackbar, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)std::round(currentAlphaSettings->foreground * TRANSPARENCY_TRACKER_STEPS));
	SendMessage(backgroundTrackbar, TBM_SETPOS, (WPARAM)TRUE, (LPARAM)std::round(currentAlphaSettings->background * TRANSPARENCY_TRACKER_STEPS));
	SetFormElementsEnableState();
}

void CSetupDialog::SetCheckboxes()
{
	enabledCheckbox->SetCheckState(currentAlphaSettings->enabled);
	separateBackgroundValueCheckbox->SetCheckState(currentAlphaSettings->separateBackgroundValue);
}

void CSetupDialog::SetAlpha(BYTE value, HWND trackbar)
{
	LONG identifier = GetWindowLong(trackbar, GWL_ID);
	switch (identifier)
	{
	case IDC_SLIDER_FOREGROUND:
		currentAlphaSettings->foreground = (BYTE)std::round(value / TRANSPARENCY_TRACKER_STEPS);
		break;
	case IDC_SLIDER_BACKGROUND:
		currentAlphaSettings->background = (BYTE)std::round(value / TRANSPARENCY_TRACKER_STEPS);
		break;
	}
}

void CSetupDialog::SetFormVisibility(bool show)
{
	const int itemIds[] = {
		IDC_CHECKBOX_ENABLE_TRANSPARENCY,
		IDC_CHECKBOX_SEPARATE_BACKGROUND_VALUE,
		IDC_STATIC_TRANSPARENCY,
		IDC_STATIC_FOREGROUND,
		IDC_SLIDER_FOREGROUND,
		IDC_STATIC_BACKGROUND,
		IDC_SLIDER_BACKGROUND,
		IDC_STATIC_FOREGORUND_TRANSPARENT,
		IDC_STATIC_FOREGROUND_OPAQUE,
		IDC_STATIC_BACKGROUND_TRANSPARENT,
		IDC_STATIC_BACKGROUND_OPAQUE
	};
	for (const auto& itemId : itemIds) {
		auto item = GetDlgItem(this->hWnd, itemId);
		ShowWindow(item, show ? SW_SHOW : SW_HIDE);
	}
}

void CSetupDialog::SetButtonEnableState(int controlId, bool show)
{
	auto item = GetDlgItem(this->hWnd, controlId);
	EnableWindow(item, show);
}

void CSetupDialog::SetFormElementsEnableState()
{
	const int itemIds[] = { IDC_SLIDER_FOREGROUND, IDC_CHECKBOX_SEPARATE_BACKGROUND_VALUE };
	for (const auto& itemId : itemIds) {
		auto item = GetDlgItem(this->hWnd, itemId);
		EnableWindow(item, currentAlphaSettings->enabled);
	}

	auto slideBackground = GetDlgItem(this->hWnd, IDC_SLIDER_BACKGROUND);
	if (currentAlphaSettings->enabled)
	{
		EnableWindow(slideBackground, currentAlphaSettings->separateBackgroundValue);
	}
	else
	{
		EnableWindow(slideBackground, FALSE);
	}
}

