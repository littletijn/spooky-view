#include "stdafx.h"
#include "CWindow.h"
#include "CDialog.h"
#include <crtdbg.h>
#include "CPtrMap.h"
#include "WndProc.h"

//http://stackoverflow.com/questions/1433204/how-do-i-share-a-variable-between-source-files-in-c-with-extern-but-how
//Message handler maps
CPtrMap<CWindow> g_ptrmap;
CPtrMap<CDialog> g_ptrmapDlg;

// The helper window procedure
// It is called by Windows, and thus it's a non-member function
// This message handler will only be called after successful SetWindowLong call
// We can assume that pointer returned by GetWindowLong is valid
// It will route messages to our member message handler
LRESULT CALLBACK WndProcRedirector(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Get a window pointer associated with this window
	CWindow *w = (CWindow *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	// It should be valid, assert so
	_ASSERT(w);
	// Redirect messages to the window procedure of the associated window
	return w->WndProc(hwnd, msg, wp, lp);
}

// The helper window procedure
// It is called by Windows, and thus it's a non-member function
// It will provide default handling for messages until WM_NCCREATE comes around,
// in which case it will set per-window this pointer to whatever was passed in CREATESTRUCT
// and reset the window's message handler to WndProcMain
LRESULT CALLBACK WndProcMain(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	CWindow *w = g_ptrmap.Extract();
	// Stash global Window pointer into per-window data area
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (long)w);
	// Reset the window message handler
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (long)WndProcRedirector);
	// Dispatch first message to the member message handler
	return WndProcRedirector(hwnd, msg, wp, lp);
}

INT_PTR CALLBACK DlgProcRedirector(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Get a window pointer associated with this window
	CDialog *w = (CDialog *)GetWindowLongPtr(hwnd, DWLP_USER);
	// It should be valid, assert so
	_ASSERT(w);
	// Redirect messages to the window procedure of the associated window
	return w->DlgProc(hwnd, msg, wp, lp);
}

INT_PTR CALLBACK DlgProcMain(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	CDialog *w = g_ptrmapDlg.Extract();
	// Stash global Window pointer into per-window data area
	SetWindowLongPtr(hwnd, DWLP_USER, (long)w);
	// Reset the window message handler
	SetWindowLongPtr(hwnd, DWLP_DLGPROC, (long)DlgProcRedirector);
	// Dispatch first message to the member message handler
	return DlgProcRedirector(hwnd, msg, wp, lp);
}