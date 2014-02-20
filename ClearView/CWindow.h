#pragma once

#include "stdafx.h"
#include "Resource.h"
#include "CWnd.h"

#define MAX_LOADSTRING 100

class CWindow : protected CWnd
{
public:
	CWindow(HINSTANCE hInstance, HWND hParent);
	virtual BOOL CWindow::InitInstance(int nCmdShow);
	ATOM RegisterWindowClass();
protected:	
};
