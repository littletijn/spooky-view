#include "CModelessDialog.h"
#include "Unicode.h"

class CUpdateAvailableDialog : public CModelessDialog
{
	tstring message;
	tstring downloadUrl;
	tstring versionNumber;
public:
	CUpdateAvailableDialog(HINSTANCE hInstance);
	INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	void SetMessage(tstring message);
	void SetDownloadUrl(tstring url);
	void SetVersionNumber(tstring versionNumber);
protected:
	BOOL Create();
	BOOL SetupDialog();
	void ShowMessage();
};

