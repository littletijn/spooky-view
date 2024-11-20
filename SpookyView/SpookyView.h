#ifndef SPOOKYVIEW_H
#define SPOOKYVIEW_H

#include "CMainWindow.h"
#include "WindowsEnum.h"
#include "ISettingsManager.h"
#include "UpdateResponse.h"

extern HICON spookyIcon;
extern HINSTANCE hInst;
extern HWND mainHwnd;
extern std::unique_ptr<ISettingsManager> settingsManager;
extern std::unique_ptr<CMainWindow> mainWindow;
extern std::unique_ptr<CSettingsDialog> cSettingsDialog;
extern WindowsEnum windowsEnum;
extern PGNSI isImmersive;
extern bool isWindows8;
extern bool isWindows10orNewer;
extern UpdateResponse updateResponse;

#endif