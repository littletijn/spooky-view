#include "CModelessDialog.h"
#include <CommCtrl.h>
#include "String.h"
#include "CAlphaSettings.h"
#include "CProgramSetting.h"
#include "ListView.h"
#include <memory>

class CSetupDialog : public CModelessDialog
{
public:
	CSetupDialog(HINSTANCE hInstance);
	~CSetupDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	CAlphaSettings currentAlphaSettings;
	CProgramSetting currentProgram;
	std::unique_ptr<ListView> appsListView;
	std::unique_ptr<ListView> windowsListView;

	void WindowsListNotified(LPARAM lParam);
	void ProgramsListNotified(LPARAM lParam);
	void PopulateProcessList(HWND hDlg);
	void PopulateWindowsList(CProgramSetting* settings);
	void SetTrackbars();
	void SetAlpha(BYTE value, HWND trackbar);
	void SetTrackbarRanges(HWND hWnd);
};

