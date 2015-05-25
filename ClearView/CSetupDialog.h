#include "CModelessDialog.h"
#include <CommCtrl.h>
#include "String.h"

class CSetupDialog : public CModelessDialog
{
public:
	CSetupDialog(HINSTANCE hInstance);
	~CSetupDialog();
	BOOL SetupDialog();
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	void WindowsListNotified(LPARAM lParam);
	void ProgramsListNotified(LPARAM lParam);
	void PopulateProcessList(HWND hDlg);
	void PopulateWindowsList(LPWSTR itemName);
	void CreateListViewItem(t_string text, LVITEM &item);
	int GetSelectedItemIndex(LPARAM lParam);
	LPWSTR GetSelectedItemText(int index, HWND hWnd, TCHAR* textBuffer);
};

