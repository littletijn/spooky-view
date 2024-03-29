#ifndef CWND_H
#define CWND_H

#include "stdafx.h"
#include "Resource.h"

class CWnd
{
public:
	//Methods
	CWnd(HINSTANCE hInstance);
	virtual BOOL InitInstance() = 0;
protected:
	//members
	HINSTANCE hInstance;
	HWND hParent;
	HWND hWnd;
};

#endif