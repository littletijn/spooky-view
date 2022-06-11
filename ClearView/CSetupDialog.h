#include "CModelessDialog.h"
#include <commctrl.h>
#include "String.h"
#include "CAlphaSettings.h"
#include "CProgramSetting.h"
#include "ListView.h"
#include "Checkbox.h"
#include <memory>
#include "CSettings.h"

class CSetupDialog : public CModelessDialog
{
public:
	CSetupDialog(HINSTANCE hInstance);
	~CSetupDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	CAlphaSettings* currentAlphaSettings;
	CProgramSetting* currentProgram;
	t_string currentProgramName;
	t_string currentWindowClassName;
	std::unique_ptr<CSettings> newSettings;
	std::unique_ptr<ListView> appsListView;
	std::unique_ptr<ListView> windowsListView;
	std::unique_ptr<Checkbox> enabledCheckbox;

	void ApplySettings();
	void CopySettings();
	void WindowsListNotified();
	void EnabledCheckboxNotified();
	void ProgramsListNotified();
	void PopulateProcessList();
	void PopulateWindowsList(CProgramSetting* settings = 0);
	void SetTrackbars();
	void SetCheckboxes();
	void SetAlpha(BYTE value, HWND trackbar);
	void SetFormVisibility(bool show);
	void SetButtonEnableState(int controlId, bool show);
	void SetTrackbarRanges(HWND hWnd);
};

