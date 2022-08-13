#include "stdafx.h"
#include "CNotifyIcon.h"
#include <stdio.h>

CNotifyIcon::CNotifyIcon(HWND hWnd, HICON hIcon, TCHAR *tooltipText)
{
	this->hWnd = hWnd;
	this->tooltipText = tstring(tooltipText);
	auto nid = CreateStructure();
	//Make sure we use the Windows 2000 and newer version
	nid.uVersion = NOTIFYICON_VERSION;
	nid.hIcon = hIcon;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_NOTIFYICON;
	_stprintf_s(nid.szTip, _countof(nid.szTip), tooltipText);
	this->addIconNid = nid;
}

CNotifyIcon::~CNotifyIcon()
{
	auto nid = CreateStructure();
	//Remove created NotifyIcon
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

NOTIFYICONDATA CNotifyIcon::CreateStructure()
{
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uID = 1;
	return nid;
}

BOOL CNotifyIcon::Init()
{
	//Call Shell_NotifyIcon to add new icon in Notification area
	BOOL result = Shell_NotifyIcon(NIM_ADD, &addIconNid);
	Shell_NotifyIcon(NIM_SETVERSION, &addIconNid);
	return result;
	
}

BOOL CNotifyIcon::SetFocus()
{
	//Call Shell_NotifyIcon to set focus back to notification area
	auto nid = CreateStructure();
	return Shell_NotifyIcon(NIM_SETFOCUS, &nid);
}

BOOL CNotifyIcon::SetTooltipText(TCHAR* tooltipText)
{
	//Call Shell_NotifyIcon to set focus back to notification area
	auto nid = CreateStructure();
	tstring fullTooltipText = tstring(this->tooltipText);
	if (tooltipText != NULL)
	{
		fullTooltipText.append(_T(" - "));
		fullTooltipText.append(tooltipText);
	}
	nid.uFlags = NIF_TIP;
	_stprintf_s(nid.szTip, _countof(nid.szTip), fullTooltipText.c_str());
	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

BOOL CNotifyIcon::ShowBalloon(TCHAR *title, TCHAR *text)
{
	auto nid = CreateStructure();
	nid.uFlags = NIF_INFO;
	nid.uTimeout = 20000; //20 seconds
	nid.dwInfoFlags = NIIF_INFO;
	_stprintf_s(nid.szInfoTitle, _countof(nid.szInfoTitle), title);
	_stprintf_s(nid.szInfo, _countof(nid.szInfo), text);
	BOOL result = Shell_NotifyIcon(NIM_MODIFY, &nid);
	return result;
}

