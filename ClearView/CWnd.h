#pragma once

#include "stdafx.h"
#include "Resource.h"

class CWnd
{
public:
	//Methods
	CWnd(HINSTANCE hInstance);
	virtual BOOL CWnd::InitInstance() = 0;
protected:
	//members
	HINSTANCE hInstance;
	HWND hParent;
	HWND hWnd;
};
