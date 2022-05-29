#include "stdafx.h"
#include "AutoUpdater.h"
#include <winhttp.h>
#include <stdio.h>
#include <nlohmann/json.hpp>
#include "UpdateResponse.h"
#include "Unicode.h"

void AutoUpdater::CheckForNewerVersion()
{
	DownloadAndParseJson();
}

bool AutoUpdater::GetProductVersion(tstring* version)
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

void AutoUpdater::DownloadAndParseJson()
{
	tstring version;
	tstring url(_T("/updates/clearview/status?version="));
	GetProductVersion(&version);

	DWORD dwSize = 0;
	DWORD dwDownloaded = 0;
	LPSTR pszOutBuffer;
	BOOL  bResults = FALSE;
	std::string response;
	HINTERNET  hSession = NULL,
		hConnect = NULL,
		hRequest = NULL;

	// Use WinHttpOpen to obtain a session handle.
	hSession = WinHttpOpen(_T("ClearView/1.0"),
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);

	// Specify an HTTP server.
	if (hSession)
	{
		hConnect = WinHttpConnect(hSession, _T("www.tyndomyn.net"), INTERNET_DEFAULT_HTTPS_PORT, 0);
	}

	// Create an HTTP request handle.
	if (hConnect)
	{
		hRequest = WinHttpOpenRequest(hConnect, _T("GET"), url.append(version).c_str(),
			NULL, WINHTTP_NO_REFERER,
			WINHTTP_DEFAULT_ACCEPT_TYPES,
			WINHTTP_FLAG_SECURE);
	}

	//Set callback
	WINHTTP_STATUS_CALLBACK callBack = WinHttpSetStatusCallback(hRequest,
		(WINHTTP_STATUS_CALLBACK)AsyncCallback,
		WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS |
		WINHTTP_CALLBACK_FLAG_REDIRECT,
		NULL);

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
			pszOutBuffer = new char[dwSize + 1];
			if (!pszOutBuffer)
			{
				printf("Out of memory\n");
				dwSize = 0;
			}
			else
			{
				// Read the Data.
				ZeroMemory(pszOutBuffer, dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
				{
					printf("Error %u in WinHttpReadData.\n", GetLastError());
				}
				else
				{
					response = response + std::string(pszOutBuffer);
				}
				// Free the memory allocated to the buffer.
				delete[] pszOutBuffer;
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
			auto updateResponse = j.get<UpdateResponse>();
		}
		catch (nlohmann::json::exception& e)
		{
		}
	}
}

void AutoUpdater::AsyncCallback(WINHTTP_STATUS_CALLBACK test)
{
}
