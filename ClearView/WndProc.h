#include "CPtrMap.h"
#include "CWindow.h"
#include "CDialog.h"
#include "stdafx.h"

//Message handler maps
extern CPtrMap<CWindow> g_ptrmap;
extern CPtrMap<CDialog> g_ptrmapDlg;

LRESULT CALLBACK WndProcRedirector(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK WndProcMain(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

INT_PTR CALLBACK DlgProcRedirector(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
INT_PTR CALLBACK DlgProcMain(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
