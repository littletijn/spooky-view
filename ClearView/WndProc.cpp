#include "stdafx.h"
#include "CWindow.h"
#include <crtdbg.h>
#include "CPtrMap.h"

extern CPtrMap g_ptrmap;

// The helper window procedure
// It is called by Windows, and thus it's a non-member function
// This message handler will only be called after successful SetWindowLong call
// We can assume that pointer returned by GetWindowLong is valid
// It will route messages to our member message handler
LRESULT CALLBACK WndProcRedirector(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
	// Get a window pointer associated with this window
	CWnd *w = (CWnd *)GetWindowLong(hwnd, GWL_USERDATA);
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
	CWnd *w = g_ptrmap.Extract();
	// Stash global Window pointer into per-window data area
	SetWindowLong(hwnd, GWL_USERDATA, (long)w);
	// Reset the window message handler
	SetWindowLong(hwnd, GWL_WNDPROC, (long)WndProcRedirector);
	// Dispatch first message to the member message handler
	return WndProcRedirector(hwnd, msg, wp, lp);
}