// MainFrame.cpp : implementation file
//

#include "stdafx.h"
#include "MainFrame.h"
#include "CNotifyIcon.h"
//#include "SetupDlg.h"

// CMainFrame
CMainFrame::CMainFrame()
{
	m_cSetupDlg = NULL;
}

CMainFrame::~CMainFrame()
{
	delete this->m_pNotifyIcon;
	if (this->m_pNotifyMenu){
		this->m_pNotifyMenu->DestroyMenu();
	}
	delete this->m_pNotifyMenu;
}

BOOL CMainFrame::CreateNotifyIcon()
{
	HICON hIcon;
	LoadIconMetric(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME), LIM_SMALL, &hIcon);
	if (hIcon) {
		this->m_pNotifyIcon = new CNotifyIcon(this->GetSafeHwnd(), hIcon);
		if (this->m_pNotifyIcon->Init()){
			return this->CreateNotifyContextMenu();
		}
	}
	return FALSE;
}

BOOL CMainFrame::CreateNotifyContextMenu()
{
	this->m_pNotifyMenu = new CMenu();
	if (this->m_pNotifyMenu->LoadMenu(IDR_NOTIFY_MENU)){
		return this->m_pNotifyMenu->GetSubMenu(0)->SetDefaultItem(ID__OPEN, FALSE);
	}
	return FALSE;
}

void CMainFrame::ShowContextMenu(POINT pt)
{
	this->SetForegroundWindow();
	this->m_pNotifyMenu->GetSubMenu(0)->TrackPopupMenu(0, pt.x, pt.y, this);
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	if (this->CreateNotifyIcon())
	{
		return CFrameWnd::OnCreateClient(lpcs, pContext);
	}
	return FALSE;
}

//Message map
const UINT wm_TaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	ON_MESSAGE(WM_NOTIFYICON, OnNotifyIcon)
	ON_COMMAND(ID__OPEN, OnOpen)
	ON_COMMAND(ID__EXIT, OnExit)
	ON_REGISTERED_MESSAGE(wm_TaskbarCreated, OnTaskbarCreated)
END_MESSAGE_MAP()

//Message handlers
void CMainFrame::OnOpen()
{
	if (!m_cSetupDlg)
	{
		m_cSetupDlg = new CSetupDlg;
		m_cSetupDlg->DoModal();
		delete m_cSetupDlg;
		m_cSetupDlg = NULL;
	} 
	else
	{
		m_cSetupDlg->SetFocus();
	}
}

void CMainFrame::OnExit()
{
	this->DestroyWindow();
}

LRESULT CMainFrame::OnNotifyIcon(WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
		case WM_LBUTTONUP:
			break;
		case WM_RBUTTONUP:
			POINT buttonPt;
			GetCursorPos(&buttonPt);
			ShowContextMenu(buttonPt);
			break;
	}
	return FALSE;
}

LRESULT CMainFrame::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}
