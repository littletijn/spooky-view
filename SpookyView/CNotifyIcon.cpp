#include "stdafx.h"
#include "CNotifyIcon.h"
#include <stdio.h>

CNotifyIcon::CNotifyIcon(HWND hWnd, HICON hIcon, TCHAR *tooltipText)
{
	m_sNotifyIcon = {};
	//Create NOTIFYICON struc
	m_sNotifyIcon.cbSize = sizeof(m_sNotifyIcon);
	//Make sure we use the Windows 2000 and newer version
	m_sNotifyIcon.uVersion = NOTIFYICON_VERSION;
	m_sNotifyIcon.hWnd = hWnd;
	m_sNotifyIcon.hIcon = hIcon;
	m_sNotifyIcon.uID = 1;
	m_sNotifyIcon.uFlags = NIF_ICON | NIF_MESSAGE;
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
	BOOL result = Shell_NotifyIcon(NIM_ADD, &m_sNotifyIcon);
	Shell_NotifyIcon(NIM_SETVERSION, &m_sNotifyIcon);
	return result;
	
}

BOOL CNotifyIcon::SetFocus()
{
	//Call Shell_NotifyIcon to set focus back to notification area
	return Shell_NotifyIcon(NIM_SETFOCUS, &m_sNotifyIcon);
}

BOOL CNotifyIcon::ShowBalloon(TCHAR *title, TCHAR *text)
{
	m_sNotifyIcon.uFlags = NIF_INFO;
	m_sNotifyIcon.uTimeout = 20000; //20 seconds
	m_sNotifyIcon.dwInfoFlags = NIIF_INFO;
	_stprintf_s(m_sNotifyIcon.szInfoTitle, _countof(m_sNotifyIcon.szInfoTitle), title);
	_stprintf_s(m_sNotifyIcon.szInfo, _countof(m_sNotifyIcon.szInfo), text);
	BOOL result = Shell_NotifyIcon(NIM_MODIFY, &m_sNotifyIcon);
	return result;
}

