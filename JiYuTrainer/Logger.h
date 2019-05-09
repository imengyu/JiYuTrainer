#pragma once
#include "stdafx.h"
#include <stdio.h>
#include <io.h>
#include <stdarg.h>

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

	virtual void Log(const wchar_t *str, ...) {}
	virtual void LogWarn(const wchar_t *str, ...) {}
	virtual void LogError(const wchar_t *str, ...) {}
	virtual void LogInfo(const wchar_t *str, ...) {}

	virtual void SetLogLevel(LogLevel level) {}
	virtual void SetLogOutPut(LogOutPut output) {}
	virtual void SetLogOutPutCallback(LogCallBack callback, LPARAM lparam) {}
	virtual void SetLogOutPutFile(const wchar_t *filePath) {}
};

class LoggerInternal : public Logger
{
public:

	LoggerInternal();
	~LoggerInternal();

	void Log(const wchar_t *str, ...) override;
	void LogWarn(const wchar_t *str, ...) override;
	void LogError(const wchar_t *str, ...) override;
	void LogInfo(const wchar_t *str, ...) override;

	void SetLogLevel(LogLevel level) override;
	void SetLogOutPut(LogOutPut output) override;
	void SetLogOutPutFile(const wchar_t *filePath) override;
	void SetLogOutPutCallback(LogCallBack callback, LPARAM lparam);

	void LogInternal(LogLevel level, const wchar_t *str, va_list arg);
	void CloseLogFile();

private:

	WCHAR logFilePath[MAX_PATH];
	FILE *logFile = nullptr;
	LogLevel level = LogLevelInfo;
	LogOutPut outPut = LogOutPutConsolne;
	LogCallBack callBack = nullptr;
	LPARAM callBackData;
};

