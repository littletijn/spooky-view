#include "CModelessDialog.h"
#include <CommCtrl.h>
#include "String.h"
#include "CAlphaSettings.h"
#include "CProgramSetting.h"

class CSetupDialog : public CModelessDialog
{
public:
	CSetupDialog(HINSTANCE hInstance);
	~CSetupDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	CAlphaSettings *currentAlphaSettings;
	CProgramSetting *currentProgram;
	void WindowsListNotified(LPARAM lParam);
	void ProgramsListNotified(LPARAM lParam);
	void PopulateProcessList(HWND hDlg);
	void PopulateWindowsList(CProgramSetting* settings);
	void SetTrackbars();
	void SetAlpha(WORD value, HWND trackbar);
	void SetTrackbarRanges(HWND hWnd);
	void CreateListViewItem(t_string text, LVITEM &item);
	int GetSelectedItemIndex(LPARAM lParam);
	LPWSTR GetSelectedItemText(int index, HWND hWnd, TCHAR* textBuffer);
};

