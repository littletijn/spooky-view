#pragma once

#include "stdafx.h"
#include "Shellapi.h"

#define WM_NOTIFYICON WM_USER

class CNotifyIcon
{
public:
	CNotifyIcon(HWND hWnd, HICON hIcon, TCHAR *tooltipText);
	~CNotifyIcon();
	BOOL Init();
	BOOL SetFocus();
protected:
	//members
	NOTIFYICONDATA m_sNotifyIcon;
};
