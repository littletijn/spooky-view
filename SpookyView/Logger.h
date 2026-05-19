#ifndef LOGGER_H
#define LOGGER_H

#include <windows.h>

enum class LogLevel { Info, Warn, Error };

class Logger
{
public:
	static Logger& Instance();

	// Idempotent. Picks %LOCALAPPDATA%\SpookyView\ and opens spookyview.log
	// in append mode. Safe to call before WinMain returns.
	void Init();
	void Shutdown();

	// printf-style. Never throws; falls back to OutputDebugString if the
	// file is unavailable.
	void Write(LogLevel level, const char* file, int line, const char* fmt, ...);

	const wchar_t* LogPath() const { return logPath; }
	const wchar_t* LogDirectory() const { return logDir; }

private:
	Logger();
	~Logger();
	Logger(const Logger&) = delete;
	Logger& operator=(const Logger&) = delete;

	CRITICAL_SECTION cs;
	HANDLE hFile;
	BOOL initialized;
	wchar_t logDir[MAX_PATH];
	wchar_t logPath[MAX_PATH];
};

// __FILE__ may be a long absolute path; the logger trims to basename.
#define LOG_INFO(...)  Logger::Instance().Write(LogLevel::Info,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARN(...)  Logger::Instance().Write(LogLevel::Warn,  __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) Logger::Instance().Write(LogLevel::Error, __FILE__, __LINE__, __VA_ARGS__)

#endif
