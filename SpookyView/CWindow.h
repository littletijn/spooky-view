#pragma once

#include "stdafx.h"
#include "Resource.h"
#include "CWnd.h"

#define MAX_LOADSTRING 100

class CWindow : protected CWnd
{
public:
	CWindow(HINSTANCE hInstance, HWND hParent = NULL);
	ATOM RegisterWindowClass();
	virtual BOOL InitInstance(int nCmdShow);
	virtual LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) = 0;
protected:
	//Members
	TCHAR szWindowClass[MAX_LOADSTRING];			// the window class name
	TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
	WNDCLASSEX wcex;
};
