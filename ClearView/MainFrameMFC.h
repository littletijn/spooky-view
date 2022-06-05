#pragma once

#include "CNotifyIcon.h"
//#include "SetupDlg.h"

// CMainFrame frame

class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();
	virtual ~CMainFrame();
	BOOL CreateNotifyIcon();

protected:
	//Members
	CMenu *m_pNotifyMenu;
	CNotifyIcon *m_pNotifyIcon;
	CSetupDlg *m_cSetupDlg;
	
	//Methods
	BOOL CreateNotifyContextMenu();
	void ShowContextMenu(POINT pt);

	//overrides
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	//message handlers
	afx_msg void OnOpen();
	afx_msg void OnExit();
	afx_msg LRESULT OnNotifyIcon(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};


