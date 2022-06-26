// SpookyView.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "SpookyView.h"
#include "AppMain.h"
#pragma once

// Global Variables:
HINSTANCE hInst; // current instance
HICON spookyIcon;
PGNSI isImmersive;
std::unique_ptr<CMainWindow> mainWindow;
std::unique_ptr<ISettingsManager> settingsManager;
UpdateResponse updateResponse;
WindowsEnum windowsEnum;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst = hInstance;
	AppMain appMain;
	return appMain.Run();
}

#ifdef UNICODE
// Convert std::string to a wchar_t* string.
std::unique_ptr<wchar_t[]> string_to_wchar_t(std::string string)
{
	size_t newsize = strlen(string.c_str()) + 1;
	auto wcstring = std::make_unique<wchar_t[]>(newsize);
	size_t convertedChars = 0;
	mbstowcs_s(&convertedChars, wcstring.get(), newsize, string.c_str(), _TRUNCATE);
	return wcstring;
}
#endif //UNICODE
