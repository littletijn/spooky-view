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
extern std::unique_ptr<CSetupDialog> cSetupDialog;
extern std::unique_ptr<CIntroDialog> cIntroDialog;
extern WindowsEnum windowsEnum;
extern PGNSI isImmersive;
extern bool isWindows8;
extern UpdateResponse updateResponse;

#endif