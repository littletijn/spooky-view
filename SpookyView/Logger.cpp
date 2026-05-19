#include "stdafx.h"
#include "Logger.h"
#include <shlobj.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace
{
	const char* Basename(const char* path)
	{
		const char* s = path;
		for (const char* p = path; *p; ++p)
		{
			if (*p == '\\' || *p == '/')
				s = p + 1;
		}
		return s;
	}

	const char* LevelTag(LogLevel level)
	{
		switch (level)
		{
		case LogLevel::Info:  return "INFO ";
		case LogLevel::Warn:  return "WARN ";
		case LogLevel::Error: return "ERROR";
		}
		return "?    ";
	}
}

Logger& Logger::Instance()
{
	static Logger instance;
	return instance;
}

Logger::Logger() : hFile(INVALID_HANDLE_VALUE), initialized(FALSE)
{
	InitializeCriticalSection(&cs);
	logDir[0] = L'\0';
	logPath[0] = L'\0';
}

Logger::~Logger()
{
	Shutdown();
	DeleteCriticalSection(&cs);
}

void Logger::Init()
{
	EnterCriticalSection(&cs);
	if (initialized)
	{
		LeaveCriticalSection(&cs);
		return;
	}

	wchar_t appData[MAX_PATH];
	appData[0] = L'\0';
	if (FAILED(SHGetFolderPathW(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, appData)))
	{
		// Fall back to temp.
		if (GetTempPathW(MAX_PATH, appData) == 0)
		{
			OutputDebugStringW(L"[SpookyView] Logger: no log directory available\n");
			LeaveCriticalSection(&cs);
			return;
		}
	}

	// %LOCALAPPDATA%\SpookyView\  -- built with kernel32 string APIs only.
	if (lstrlenW(appData) >= MAX_PATH - 32)
	{
		LeaveCriticalSection(&cs);
		return;
	}
	lstrcpynW(logDir, appData, MAX_PATH);
	lstrcatW(logDir, L"\\SpookyView");
	CreateDirectoryW(logDir, NULL);

	lstrcpynW(logPath, logDir, MAX_PATH);
	lstrcatW(logPath, L"\\spookyview.log");

	// Naive rotation: if > 1 MiB, shift generations .1 → .2 → .3 and start fresh.
	// Keeps at most 4 files (current + .1/.2/.3), bounding disk use to ~4 MiB.
	WIN32_FILE_ATTRIBUTE_DATA fa;
	if (GetFileAttributesExW(logPath, GetFileExInfoStandard, &fa))
	{
		ULARGE_INTEGER size;
		size.LowPart = fa.nFileSizeLow;
		size.HighPart = fa.nFileSizeHigh;
		if (size.QuadPart > (1ULL << 20))
		{
			// Need room for ".3" (2 wchars) plus the existing NUL.
			if (lstrlenW(logPath) + 2 < MAX_PATH)
			{
				wchar_t p1[MAX_PATH], p2[MAX_PATH], p3[MAX_PATH];
				lstrcpynW(p1, logPath, MAX_PATH); lstrcatW(p1, L".1");
				lstrcpynW(p2, logPath, MAX_PATH); lstrcatW(p2, L".2");
				lstrcpynW(p3, logPath, MAX_PATH); lstrcatW(p3, L".3");

				DeleteFileW(p3);        // ignore failure (may not exist)
				MoveFileW(p2, p3);      // ignore failure (may not exist)
				MoveFileW(p1, p2);      // ignore failure (may not exist)
				MoveFileW(logPath, p1); // real move; CreateFile below recreates current
			}
			// If the path is too long to safely append a suffix, skip rotation
			// rather than risk a truncated path. The log may grow past 1 MiB.
		}
	}

	hFile = CreateFileW(
		logPath,
		FILE_APPEND_DATA,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		OutputDebugStringW(L"[SpookyView] Logger: failed to open log file\n");
		LeaveCriticalSection(&cs);
		return;
	}

	SetFilePointer(hFile, 0, NULL, FILE_END);
	initialized = TRUE;
	LeaveCriticalSection(&cs);

	LOG_INFO("---- Logger initialized, pid=%lu ----", GetCurrentProcessId());
}

void Logger::Shutdown()
{
	EnterCriticalSection(&cs);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		hFile = INVALID_HANDLE_VALUE;
	}
	initialized = FALSE;
	LeaveCriticalSection(&cs);
}

void Logger::Write(LogLevel level, const char* file, int line, const char* fmt, ...)
{
	char body[2048];
	va_list args;
	va_start(args, fmt);
	int written = vsnprintf(body, sizeof(body), fmt, args);
	va_end(args);
	if (written < 0 || written >= (int)sizeof(body))
	{
		body[sizeof(body) - 1] = '\0';
	}

	SYSTEMTIME st;
	GetLocalTime(&st);

	char line_buf[2560];
	int n = snprintf(
		line_buf,
		sizeof(line_buf),
		"%04u-%02u-%02u %02u:%02u:%02u.%03u %s [%s:%d] %s\r\n",
		st.wYear, st.wMonth, st.wDay,
		st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
		LevelTag(level),
		Basename(file ? file : "?"),
		line,
		body);
	if (n < 0 || n >= (int)sizeof(line_buf))
	{
		line_buf[sizeof(line_buf) - 1] = '\0';
		n = (int)strlen(line_buf);
	}

	EnterCriticalSection(&cs);
	if (initialized && hFile != INVALID_HANDLE_VALUE)
	{
		DWORD junk = 0;
		WriteFile(hFile, line_buf, (DWORD)n, &junk, NULL);
	}
	else
	{
		OutputDebugStringA(line_buf);
	}
	LeaveCriticalSection(&cs);
}
