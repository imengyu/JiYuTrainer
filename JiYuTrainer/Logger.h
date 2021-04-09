#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <string>
#include <io.h>
#include <stdarg.h>
#include <list>

enum LogLevel {
	LogLevelText,
	LogLevelInfo,
	LogLevelWarn,
	LogLevelError,
	LogLevelDisabled,
};
enum LogOutPut {
	LogOutPutConsolne,
	LogOutPutFile,
	LogOutPutCallback,
};

typedef void(*LogCallBack)(const wchar_t *str, LogLevel level, LPARAM lparam);

class Logger
{
public:

	virtual void Log(const wchar_t * str, ...) {}
	virtual void LogWarn(const wchar_t * str, ...) {}
	virtual void LogError(const wchar_t * str, ...) {}	
	virtual void LogInfo(const wchar_t * str, ...) {}
	virtual void Log2(const wchar_t * str, const char*file, int line, const char*functon, ...) {}
	virtual void LogWarn2(const wchar_t * str, const  char*file, int line, const char*functon, ...) {}
	virtual void LogError2(const wchar_t * str, const char*file, int line, const char*functon, ...) {}
	virtual void LogInfo2(const wchar_t * str, const char*file, int line, const char*functon, ...) {}

	virtual LogLevel GetLogLevel() { return LogLevel::LogLevelDisabled; }
	virtual void SetLogLevel(LogLevel level) {}
	virtual void SetLogOutPut(LogOutPut output) {}
	virtual void SetLogOutPutCallback(LogCallBack callback, LPARAM lparam) {}
	virtual void SetLogOutPutFile(const wchar_t *filePath) {}

	virtual void ResentNotCaputureLog(){}
	virtual void WritePendingLog(const wchar_t *str, LogLevel level) {}
};

class LoggerInternal : public Logger
{
public:

	struct LOG_SLA {
		std::wstring str;
		LogLevel level;
	};

	LoggerInternal();
	~LoggerInternal();

	void Log(const wchar_t *str, ...) override;
	void LogWarn(const wchar_t *str, ...) override;
	void LogError(const wchar_t *str, ...) override;
	void LogInfo(const wchar_t *str, ...) override;
    void Log2(const wchar_t * str, const char*file, int line, const char*functon, ...);
	void LogWarn2(const wchar_t * str, const char*file, int line, const char*functon, ...);
	void LogError2(const wchar_t * str, const char*file, int line, const char*functon, ...);
	void LogInfo2(const wchar_t * str, const char*file, int line, const char*functon, ...);

	LogLevel GetLogLevel() { return level; }
	void SetLogLevel(LogLevel level) override;
	void SetLogOutPut(LogOutPut output) override;
	void SetLogOutPutFile(const wchar_t *filePath) override;
	void SetLogOutPutCallback(LogCallBack callback, LPARAM lparam);

	void ResentNotCaputureLog();
	void WritePendingLog(const wchar_t *str, LogLevel level);

	void LogInternalWithCodeAndLine(LogLevel level, const wchar_t * str, const char*file, int line, const char*functon, va_list arg);
	void LogInternal(LogLevel level, const wchar_t *str, va_list arg);
	void LogOutput(LogLevel level, const wchar_t *str, size_t len);
	void CloseLogFile();

private:

	std::list< LOG_SLA> logPendingBuffer;
	WCHAR logFilePath[MAX_PATH];
	FILE *logFile = nullptr;
	LogLevel level = LogLevelInfo;
	LogOutPut outPut = LogOutPutConsolne;
	LogCallBack callBack = nullptr;
	LPARAM callBackData;
};

#define LogError2(str, ...) LogError2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)
#define LogWarn2(str, ...) LogWarn2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)
#define LogInfo2(str, ...) LogInfo2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)
#define Log2(str, ...) Log2(str, __FILE__, __LINE__, __FUNCTION__,__VA_ARGS__)

