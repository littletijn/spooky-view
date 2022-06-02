#include "CModelessDialog.h"
#include "Unicode.h"

class CUpdateAvailableDialog : public CModelessDialog
{
	tstring message;
	tstring downloadUrl;
public:
	CUpdateAvailableDialog(HINSTANCE hInstance);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void SetMessage(tstring message);
	void SetDownloadUrl(tstring url);
protected:
	BOOL Create();
	BOOL SetupDialog();
	void ShowMessage();
	void AddSkipVersionKey();
};

