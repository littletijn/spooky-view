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
#include "Logger.h"

#ifndef WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3
#define WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3 0x00002000
#endif

#ifndef WINHTTP_OPTION_ENABLE_FEATURE
#define WINHTTP_OPTION_ENABLE_FEATURE 79
#endif

#ifndef WINHTTP_ENABLE_SSL_REVOCATION
#define WINHTTP_ENABLE_SSL_REVOCATION 0x00000001
#endif

namespace {
	constexpr size_t kMaxResponseBytes = 256 * 1024;
	constexpr size_t kMaxChunkBytes = 64 * 1024;

	// RFC 3986 unreserved + safe query characters: A-Z a-z 0-9 - _ . ~
	static tstring UrlEncodeComponent(const TCHAR* in)
	{
		tstring out;
		if (!in) return out;
		for (const TCHAR* p = in; *p; ++p)
		{
			TCHAR c = *p;
			bool unreserved = (c >= _T('A') && c <= _T('Z')) ||
							  (c >= _T('a') && c <= _T('z')) ||
							  (c >= _T('0') && c <= _T('9')) ||
							  c == _T('-') || c == _T('_') || c == _T('.') || c == _T('~');
			if (unreserved) {
				out += c;
			} else {
				TCHAR buf[8];
				unsigned char b1 = static_cast<unsigned char>(c & 0xFF);
				unsigned char b2 = static_cast<unsigned char>((c >> 8) & 0xFF);
				if (b2) {
					_stprintf_s(buf, _countof(buf), _T("%%%02X%%%02X"), b2, b1);
				} else {
					_stprintf_s(buf, _countof(buf), _T("%%%02X"), b1);
				}
				out += buf;
			}
		}
		return out;
	}
}

#ifdef UNICODE

void CreateUpdateCheckerThread()
{
	DWORD threadId;
	HANDLE hThread = CreateThread(NULL, 0, UpdateCheckerThread, NULL, 0, &threadId);
	if (hThread != NULL)
	{
		// Fire-and-forget; the thread cleans itself up on return.
		CloseHandle(hThread);
	}
	else
	{
		LOG_WARN("CreateThread(UpdateCheckerThread) failed: err=%lu", GetLastError());
	}
}

DWORD WINAPI UpdateCheckerThread(LPVOID lpParam)
{
	try
	{
		UpdateChecker autoUpdater;
		autoUpdater.CheckForNewerVersion();
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("UpdateCheckerThread unhandled std::exception: %s", e.what());
	}
	catch (...)
	{
		LOG_ERROR("UpdateCheckerThread unhandled non-std exception");
	}
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
	if (!GetFileVersionInfo(szFilename, 0, dwSize, &data[0]))
	{
		return false;
	}

	VS_FIXEDFILEINFO* versioInfo = NULL;
	unsigned int versionInfoLength = 0;

	if (!VerQueryValue(&data[0], _T("\\"), (LPVOID*)&versioInfo, &versionInfoLength) || versioInfo == NULL)
	{
		return false;
	}

	*version += 
		to_tstring((versioInfo->dwProductVersionMS >> 16) & 0xffff) + _T(".") +
		to_tstring((versioInfo->dwProductVersionMS >> 0) & 0xffff) + _T(".") +
		to_tstring((versioInfo->dwProductVersionLS >> 16) & 0xffff) + _T(".") +
		to_tstring((versioInfo->dwProductVersionLS >> 0) & 0xffff);

	return TRUE;
}

void UpdateChecker::GetUserLanguage(tstring* version)
{
	TCHAR bufferLanguage[10];
	TCHAR bufferCountry[10];
	auto languageId = GetUserDefaultUILanguage();
	GetLocaleInfo(languageId, LOCALE_SISO639LANGNAME, bufferLanguage, sizeof(bufferLanguage) / sizeof(TCHAR));
	GetLocaleInfo(languageId, LOCALE_SISO3166CTRYNAME, bufferCountry, sizeof(bufferCountry) / sizeof(TCHAR));
	version->append(bufferLanguage);
	version->append(_T("-"));
	version->append(bufferCountry);
}

void UpdateChecker::DownloadAndParseJson()
{
	tstring version;
	tstring language;
	tstring url(_T("spookyview/status?version="));
	GetProductVersion(&version);
	url.append(UrlEncodeComponent(version.c_str()));
#ifdef PRE_RELEASE
	url.append(_T("&pre_release=1"));
#endif
	url.append(_T("&language="));
	GetUserLanguage(&language);
	url.append(UrlEncodeComponent(language.c_str()));
	url.append(_T("&arch="));
#ifdef _M_IX86 
	url.append(UrlEncodeComponent(_T("x86")));
#elif _M_AMD64
		url.append(UrlEncodeComponent(_T("x64")));
#elif _M_ARM64 
	url.append(UrlEncodeComponent(_T("AArch64")));
#else
	url.append(UrlEncodeComponent(_T("unknown")));
#endif
	url.append(_T("&packaging="));
#ifdef PACKAGING_STORE
	url.append(UrlEncodeComponent(_T("store")));
#elif PACKAGING_MSIX
	url.append(UrlEncodeComponent(_T("msix")));
#elif PACKAGING_MSI
	url.append(UrlEncodeComponent(_T("msi")));
#elif PACKAGING_PORTABLE
	url.append(UrlEncodeComponent(_T("portable")));
#else
	url.append(UrlEncodeComponent(_T("unknown")));
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
		if (!WinHttpSetTimeouts(hSession, 5000, 5000, 15000, 15000))
		{
			LOG_WARN("WinHttpSetTimeouts failed (GetLastError=%lu); continuing with defaults", GetLastError());
		}
		DWORD secureProtocols = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
#ifdef WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3
		secureProtocols |= WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_3;
#endif
		if (!WinHttpSetOption(hSession, WINHTTP_OPTION_SECURE_PROTOCOLS,
							  &secureProtocols, sizeof(secureProtocols)))
		{
			LOG_WARN("WinHttpSetOption(SECURE_PROTOCOLS) failed: err=%lu", GetLastError());
		}
		DWORD enableFeatures = WINHTTP_ENABLE_SSL_REVOCATION;
		if (!WinHttpSetOption(hSession, WINHTTP_OPTION_ENABLE_FEATURE,
							  &enableFeatures, sizeof(enableFeatures)))
		{
			LOG_WARN("WinHttpSetOption(ENABLE_SSL_REVOCATION) failed: err=%lu", GetLastError());
		}
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
				LOG_WARN("WinHttpQueryDataAvailable failed (GetLastError=%lu)", GetLastError());
				break;
			}

			if (dwSize == 0)
			{
				break;
			}

			// Reject implausibly large single chunks (server-controlled value).
			if (dwSize > kMaxChunkBytes)
			{
				LOG_WARN("Update response chunk too large (%lu bytes); aborting read", dwSize);
				break;
			}

			// Enforce cumulative cap on the response body.
			if (response.size() + dwSize > kMaxResponseBytes)
			{
				LOG_WARN("Update response exceeds %zu bytes; aborting read", kMaxResponseBytes);
				break;
			}

			try
			{
				// Allocate space for the buffer.
				auto pszOutBuffer = std::make_unique<char[]>(dwSize + 1);
				ZeroMemory(pszOutBuffer.get(), dwSize + 1);

				if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer.get(), dwSize, &dwDownloaded))
				{
					LOG_WARN("WinHttpReadData failed (GetLastError=%lu)", GetLastError());
					break;
				}

				response.append(pszOutBuffer.get(), dwDownloaded);
			}
			catch (const std::bad_alloc&)
			{
				LOG_WARN("std::bad_alloc reading update response chunk (%lu bytes)", dwSize);
				break;
			}

		} while (dwSize > 0);
	}

	// Report any errors.
	if (!bResults)
	{
		LOG_WARN("Update request failed: err=%lu", GetLastError());
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
				PostMessage(mainWindow->GetHwnd(), WM_UPDATE_AVAILABLE, 0, 0);
			}
		}
		catch (nlohmann::json::exception& e)
		{
			LOG_WARN("Update JSON parse failed: %s", e.what());
			return;
		}
	}
}

#endif
