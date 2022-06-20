#pragma once

#include "stdafx.h"
#include "CWnd.h"

class CDialog : protected CWnd
{
public:
	CDialog(HINSTANCE hInstance);
	~CDialog();
	BOOL InitInstance();
protected:
	LPCTSTR dialogResource;
	virtual BOOL SetupDialog() = 0;
	virtual BOOL Create() = 0;
};

