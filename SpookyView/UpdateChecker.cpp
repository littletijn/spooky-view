#include "stdafx.h"
#include "UpdateChecker.h"
#include <winhttp.h>
#include <stdio.h>
#include <nlohmann/json.hpp>
#include "UpdateResponse.h"
#include "Unicode.h"
#include "CMainWindow.h"
#include "Defines.h"
#include "SpookyView.h"

#ifdef UNICODE

void CreateUpdateCheckerThread()
{
	DWORD threadId;
	auto hThread = CreateThread(NULL, 0, UpdateCheckerThread, NULL, 0, &threadId);
}

DWORD WINAPI UpdateCheckerThread(LPVOID lpParam)
{
	std::unique_ptr<UpdateChecker> autoUpdater;
	autoUpdater->CheckForNewerVersion();
	return 0;
}

void UpdateChecker::CheckForNewerVersion()
{
	DownloadAndParseJson();
}

bool UpdateChecker::GetProductVersion(tstring* version)
{
	// get the filename of the executable containing the version resource
	TCHAR szFilename[MAX_PATH + 1] = { 0 };
	if (GetModuleFileName(NULL, szFilename, MAX_PATH) == 0)
	{
		return false;
	}

	// allocate a block of memory for the version info
	DWORD dummy;
	DWORD dwSize = GetFileVersionInfoSize(szFilename, &dummy);
	if (dwSize == 0)
	{
		return false;
	}
	std::vector<BYTE> data(dwSize);

	// load the version info
	if (!GetFileVersionInfo(szFilename, NULL, dwSize, &data[0]))
	{
		return false;
	}

	VS_FIXEDFILEINFO* versioInfo = NULL;
	unsigned int versionInfoLength = 0;

	VerQueryValue(&data[0], _T("\\"), (LPVOID*)&versioInfo, &versionInfoLength);

	*version += 
		to_tstring((versioInfo->dwProductVersionMS >> 16) & 0xffff) + _T(".") +
		to_tstring((versioInfo->dwProductVersionMS >> 0) & 0xffff) + _T(".") +
		to_tstring((versioInfo->dwProductVersionLS >> 16) & 0xffff) + _T(".") +
		to_tstring((versioInfo->dwProductVersionLS >> 0) & 0xffff);

	return TRUE;
}

void UpdateChecker::DownloadAndParseJson()
{
	tstring version;
	tstring url(_T("spookyview/status?version="));
	GetProductVersion(&version);
	url.append(version);
	url.append(_T("&arch="));
#ifdef _M_IX86 
	url.append(_T("x86"));
#elif _M_AMD64
		url.append(_T("x64"));
#elif _M_ARM64 
	url.append(_T("AArch64"));
#else
	url.append(_T("unknown"));
#endif
	url.append(_T("&packaging="));
#ifdef PACKAGING_STORE
	url.append(_T("store"));
#elif PACKAGING_MSIX
	url.append(_T("msix"));
#elif PACKAGING_MSI
	url.append(_T("msi"));
#elif PACKAGING_PORTABLE
	url.append(_T("portable"));
#else
	url.append(_T("unknown"));
#endif

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	BOOL  bResults = FALSE;
	std::string response;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	tstring userAgent = _T("SpookyView/");
	userAgent.append(version);
	hSession = WinHttpOpen(userAgent.c_str(),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
	{
		hConnect = WinHttpConnect(hSession, _T("updates.tyndomyn.net"), INTERNET_DEFAULT_HTTPS_PORT, 0);
	}

	// Create an HTTP request handle.
	if (hConnect)
	{
		hRequest = WinHttpOpenRequest(hConnect, _T("GET"), url.c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);
	}

	// Send a request.
	if (hRequest)
	{
		bResults = WinHttpSendRequest(hRequest,
			WINHTTP_NO_ADDITIONAL_HEADERS,
			0, WINHTTP_NO_REQUEST_DATA, 0,
			0, 0);
	}


	// End the request.
	if (bResults)
	{
		bResults = WinHttpReceiveResponse(hRequest, NULL);
	}

	// Keep checking for data until there is nothing left.
	if (bResults)
	{
		do
		{
			// Check for available data.
			dwSize = 0;
			if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
			{
				printf("Error %u in WinHttpQueryDataAvailable.\n", GetLastError());
			}

			// Allocate space for the buffer.
			auto pszOutBuffer = std::make_unique<char[]>(dwSize + 1);
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the Data.
				ZeroMemory(pszOutBuffer.get(), dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer.get(), dwSize, &dwDownloaded))
				{
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				}
				else
				{
					response = response + std::string(pszOutBuffer.get());
				}
			}

		} while (dwSize > 0);
	}

	// Report any errors.
	if (!bResults)
	{
		printf("Error %d has occurred.\n", GetLastError());
	}

	// Close any open handles.
	if (hRequest) WinHttpCloseHandle(hRequest);
	if (hConnect) WinHttpCloseHandle(hConnect);
	if (hSession) WinHttpCloseHandle(hSession);

	if (!response.empty())
	{
		try
		{
			auto j = nlohmann::json::parse(response);
			updateResponse = j.get<UpdateResponse>();
			if (updateResponse.update_available)
			{
				PostMessage(mainWindow->GetHwnd(), WM_UPDATE_AVAILABLE, NULL, NULL);
			}
		}
		catch (nlohmann::json::exception& e)
		{
			auto test = e;
		}
	}
}

#endif
