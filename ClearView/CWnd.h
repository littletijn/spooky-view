#pragma once

#include "stdafx.h"
#include "Resource.h"

#define MAX_LOADSTRING 100

class CWnd
{
public:
	//Methods
	CWnd(HINSTANCE hInstance);
	ATOM RegisterWindowClass();
	virtual BOOL CWnd::InitInstance();
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
protected:
	//members
	TCHAR szWindowClass[MAX_LOADSTRING];			// the window class name
	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
	HINSTANCE hInstance;
	HWND hParent;
	HWND hWnd;
	WNDCLASSEX wcex;
};
