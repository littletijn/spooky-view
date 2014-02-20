#include "stdafx.h"
#include "CNotifyIcon.h"
#include <stdio.h>

CNotifyIcon::CNotifyIcon(HWND hWnd, HICON hIcon, TCHAR *tooltipText)
{
	//Create NOTIFYICON struc
	m_sNotifyIcon.cbSize = sizeof(NOTIFYICONDATA_V2_SIZE);
	//Make sure we use the Windows 2000 and newer version
	m_sNotifyIcon.uVersion = NOTIFYICON_VERSION_4;
	m_sNotifyIcon.hWnd = hWnd;
	m_sNotifyIcon.hIcon = hIcon;
	m_sNotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_sNotifyIcon.uCallbackMessage = WM_NOTIFYICON;
	_stprintf_s(m_sNotifyIcon.szTip, _countof(m_sNotifyIcon.szTip), tooltipText);
}

CNotifyIcon::~CNotifyIcon()
{
	//Remove created NotifyIcon
	Shell_NotifyIcon(NIM_DELETE, &m_sNotifyIcon);
}

BOOL CNotifyIcon::Init()
{
	//Call Shell_NotifyIcon to add new icon in Notification area
	return Shell_NotifyIcon(NIM_ADD, &m_sNotifyIcon);
}

BOOL CNotifyIcon::SetFocus()
{
	//Call Shell_NotifyIcon to set focus back to notification area
	return Shell_NotifyIcon(NIM_SETFOCUS, &m_sNotifyIcon);
}

