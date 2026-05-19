#include "stdafx.h"
#include "CrashHandler.h"
#include "Logger.h"
#include <dbghelp.h>

#pragma comment(lib, "dbghelp.lib")

namespace
{
	LPTOP_LEVEL_EXCEPTION_FILTER s_previousFilter = NULL;
	PVOID s_vehHandle = NULL;

	LONG WINAPI UnhandledFilter(EXCEPTION_POINTERS* info)
	{
		if (info && info->ExceptionRecord)
		{
			LOG_ERROR("Unhandled exception: code=0x%08lX flags=0x%08lX addr=%p",
				info->ExceptionRecord->ExceptionCode,
				info->ExceptionRecord->ExceptionFlags,
				info->ExceptionRecord->ExceptionAddress);
		}
		else
		{
			LOG_ERROR("Unhandled exception (no record)");
		}

		// Write a minidump next to the log file.
		wchar_t dumpPath[MAX_PATH];
		const wchar_t* dir = Logger::Instance().LogDirectory();
		if (dir && dir[0])
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			// %ls is portable in MSVC; under MinGW UCRT _snwprintf_s with %ls
			// is reliable for wchar_t* substitution.
			_snwprintf(dumpPath, MAX_PATH,
				L"%ls\\spookyview-crash-%04u%02u%02u-%02u%02u%02u-%lu.dmp",
				dir,
				st.wYear, st.wMonth, st.wDay,
				st.wHour, st.wMinute, st.wSecond,
				GetCurrentProcessId());
			dumpPath[MAX_PATH - 1] = L'\0';

			HANDLE hDump = CreateFileW(dumpPath, GENERIC_WRITE, 0, NULL,
				CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hDump != INVALID_HANDLE_VALUE)
			{
				MINIDUMP_EXCEPTION_INFORMATION mei;
				mei.ThreadId = GetCurrentThreadId();
				mei.ExceptionPointers = info;
				mei.ClientPointers = FALSE;

				BOOL ok = MiniDumpWriteDump(
					GetCurrentProcess(),
					GetCurrentProcessId(),
					hDump,
					MiniDumpNormal,
					info ? &mei : NULL,
					NULL,
					NULL);
				CloseHandle(hDump);
				if (ok)
				{
					LOG_ERROR("Minidump written: %ls", dumpPath);
				}
				else
				{
					LOG_ERROR("MiniDumpWriteDump failed: lastError=%lu", GetLastError());
				}
			}
		}

		if (s_previousFilter)
		{
			return s_previousFilter(info);
		}
		return EXCEPTION_EXECUTE_HANDLER;
	}

	LONG CALLBACK VectoredHandler(EXCEPTION_POINTERS* info)
	{
		// Only act on real fatal exceptions (skip C++ EH propagation 0xE06D7363,
		// MSVC thread name 0x406D1388, breakpoints, single-step, debug strings).
		if (!info || !info->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;
		DWORD code = info->ExceptionRecord->ExceptionCode;
		switch (code)
		{
		case EXCEPTION_ACCESS_VIOLATION:
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
		case EXCEPTION_DATATYPE_MISALIGNMENT:
		case EXCEPTION_FLT_DIVIDE_BY_ZERO:
		case EXCEPTION_ILLEGAL_INSTRUCTION:
		case EXCEPTION_IN_PAGE_ERROR:
		case EXCEPTION_INT_DIVIDE_BY_ZERO:
		case EXCEPTION_NONCONTINUABLE_EXCEPTION:
		case EXCEPTION_PRIV_INSTRUCTION:
		case EXCEPTION_STACK_OVERFLOW:
			// First-chance fatal exception: log + minidump, then let it
			// propagate so the OS terminates the process predictably.
			UnhandledFilter(info);
			return EXCEPTION_CONTINUE_SEARCH;
		default:
			return EXCEPTION_CONTINUE_SEARCH;
		}
	}
}

void CrashHandler::Install()
{
	static bool installed = false;
	if (installed) return;
	s_previousFilter = SetUnhandledExceptionFilter(UnhandledFilter);

	// Vectored handler runs first-chance, before the kernel callback filter
	// can swallow exceptions thrown inside dialog/window procs on x64.
	s_vehHandle = AddVectoredExceptionHandler(1, VectoredHandler);

	// Best-effort: clear PROCESS_CALLBACK_FILTER_ENABLED so the unhandled
	// exception filter still fires for crashes inside dialog procs on older
	// Windows. Silent no-op if the API isn't exported by this build of
	// kernel32 (Windows 11 moved/dropped some legacy APIs).
	typedef BOOL(WINAPI* SetPolicyFn)(DWORD);
	typedef BOOL(WINAPI* GetPolicyFn)(LPDWORD);
	HMODULE hKernel = GetModuleHandleW(L"kernel32.dll");
	if (hKernel)
	{
		auto pSet = (SetPolicyFn)GetProcAddress(hKernel, "SetProcessUserModeExceptionPolicy");
		auto pGet = (GetPolicyFn)GetProcAddress(hKernel, "GetProcessUserModeExceptionPolicy");
		if (pSet && pGet)
		{
			DWORD flags = 0;
			if (pGet(&flags))
			{
				const DWORD PROCESS_CALLBACK_FILTER_ENABLED = 0x1;
				pSet(flags & ~PROCESS_CALLBACK_FILTER_ENABLED);
			}
		}
	}

	installed = true;
	LOG_INFO("Crash handler installed (VEH=%p)", s_vehHandle);
}
