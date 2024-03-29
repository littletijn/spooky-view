#ifndef DEFINES_H
#define DEFINES_H

//http://msdn.microsoft.com/en-us/library/windows/desktop/ms633577(v=vs.85).aspx
//Max length of className is 256 characters
#define MAX_WINDOW_CLASS_NAME 256

#define WM_UPDATE_AVAILABLE WM_USER + 1
#define WM_MODELESS_DIALOG_DESTROYED WM_USER + 2

//LoadAddress declaration
typedef BOOL(WINAPI* PGNSI)(HANDLE);

#endif