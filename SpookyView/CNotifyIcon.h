#include "shellapi.h"
#pragma once
#include "stdafx.h"
#include "Unicode.h"

#define WM_NOTIFYICON WM_USER

class CNotifyIcon
{
public:
	CNotifyIcon(HWND hWnd, HICON hIcon, TCHAR *tooltipText);
	~CNotifyIcon();
	BOOL Init();
	BOOL SetFocus();
	BOOL SetTooltipText(TCHAR* tooltipText);
	BOOL ShowBalloon(TCHAR *title, TCHAR *text);
protected:
	HWND hWnd;
	NOTIFYICONDATA addIconNid;
	tstring tooltipText;
	NOTIFYICONDATA CreateStructure();
};
