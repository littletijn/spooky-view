#include "CModelessDialog.h"
#include <CommCtrl.h>
#include "String.h"
#include "CAlphaSettings.h"
#include "CProgramSetting.h"
#include "ListView.h"
#include "Checkbox.h"
#include <memory>

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
	std::unique_ptr<ListView> appsListView;
	std::unique_ptr<ListView> windowsListView;
	std::unique_ptr<Checkbox> enabledCheckbox;

	void WindowsListNotified(LPARAM lParam);
	void EnabledCheckboxNotified();
	void ProgramsListNotified(LPARAM lParam);
	void PopulateProcessList(HWND hDlg);
	void PopulateWindowsList(CProgramSetting* settings = 0);
	void SetTrackbars();
	void SetCheckboxes();
	void SetAlpha(BYTE value, HWND trackbar);
	void SetFormVisibility(bool show);
	void SetTrackbarRanges(HWND hWnd);
};

