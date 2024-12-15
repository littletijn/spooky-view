#ifndef CSETUPDIALOG_H
#define CSETUPDIALOG_H

#include "CModelessDialog.h"
#include <commctrl.h>
#include "MultiPlatformString.h"
#include "CModificationSettings.h"
#include "CProgramSetting.h"
#include "ListView.h"
#include "Checkbox.h"
#include <memory>
#include "CSettings.h"

class CSetupDialog : public CModelessDialog
{
public:
	CSetupDialog(HINSTANCE hInstance, HWND hParent);
	~CSetupDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	TCHAR allOtherAppsString[160];
	TCHAR allOtherWindowsString[160];
	CModificationSettings* currentModificationSettings;
	CProgramSetting* currentProgram;
	t_string currentProgramName;
	t_string currentWindowClassName;
	std::unique_ptr<CSettings> newSettings;
	std::unique_ptr<ListView> appsListView;
	std::unique_ptr<ListView> windowsListView;
	std::unique_ptr<Checkbox> enabledCheckbox;
	std::unique_ptr<Checkbox> alwaysOnTopCheckbox;
	std::unique_ptr<Checkbox> separateBackgroundValueCheckbox;

	bool ApplySettings();
	void CopySettings();
	void WindowsListNotified();
	void EnabledCheckboxNotified();
	void AlwaysOnTopCheckboxNotified();
	void SeparateBackgroundValueCheckboxNotified();
	void ProgramsListNotified();
	void PopulateProcessList();
	void PopulateWindowsList(CProgramSetting* settings = 0);
	void SetTrackbars();
	void SetCheckboxes();
	void SetAlpha(BYTE value, HWND trackbar);
	void SetFormVisibility(bool show);
	void SetFormElementVisibility(int itemId, bool show);
	void SetButtonEnableState(int controlId, bool show);
	void SetFormElementsEnableState();
	void SetTrackbarRanges(HWND hWnd);
};

#endif