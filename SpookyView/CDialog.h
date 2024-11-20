#ifndef CDIALOG_H
#define CDIALOG_H

#include "stdafx.h"
#include "CWnd.h"

class CDialog : protected CWnd
{
public:
	CDialog(HINSTANCE hInstance);
	BOOL hasInitInstance();
	BOOL InitInstance();
	void SetForeground();
protected:
	LPCTSTR dialogResource;
	BOOL hasInit;
	virtual BOOL SetupDialog() = 0;
	virtual BOOL Create() = 0;
};

#endif