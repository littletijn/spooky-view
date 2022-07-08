// SpookyView.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "SpookyView.h"
#include "AppMain.h"
#include "CMainWindow.h"
#include "ISettingsManager.h"
#include "UpdateResponse.h"
#include "WindowsEnum.h"
#pragma once

// Global Variables:
HINSTANCE hInst; // current instance
HICON spookyIcon;
PGNSI isImmersive;
std::unique_ptr<CMainWindow> mainWindow;
std::unique_ptr<ISettingsManager> settingsManager;
UpdateResponse updateResponse;
WindowsEnum windowsEnum;
bool isWindows8;
bool isWindows10orNewer;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;
	AppMain appMain;
	return appMain.Run();
}
