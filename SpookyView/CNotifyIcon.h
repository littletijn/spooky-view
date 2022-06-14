#pragma once

#include "stdafx.h"
#include "shellapi.h"

#define WM_NOTIFYICON WM_USER

class CNotifyIcon
{
public:
	CNotifyIcon(HWND hWnd, HICON hIcon, TCHAR *tooltipText);
	~CNotifyIcon();
	BOOL Init();
	BOOL SetFocus();
	BOOL ShowBalloon(TCHAR *title, TCHAR *text);
protected:
	//members
	NOTIFYICONDATA m_sNotifyIcon;
};
