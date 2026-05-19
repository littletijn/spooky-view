#ifndef CRASH_HANDLER_H
#define CRASH_HANDLER_H

#include <windows.h>

namespace CrashHandler
{
	// Install SetUnhandledExceptionFilter so unhandled SEH exceptions get
	// logged and a minidump is written next to spookyview.log.
	// Idempotent. Call once at startup, after Logger::Init().
	void Install();
}

#endif
