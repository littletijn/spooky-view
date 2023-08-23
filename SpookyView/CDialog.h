#ifndef CDIALOG_H
#define CDIALOG_H

#include "stdafx.h"
#include "CWnd.h"

class CDialog : protected CWnd
{
public:
	CDialog(HINSTANCE hInstance);
	BOOL InitInstance();
	void SetForeground();
protected:
	LPCTSTR dialogResource;
	virtual BOOL SetupDialog() = 0;
	virtual BOOL Create() = 0;
};

#endif