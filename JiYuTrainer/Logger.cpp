#include "stdafx.h"
#include "Logger.h"
#include <time.h>
#include "StringHlp.h"
#include "PathHelper.h"

using namespace std;

#undef LogError2
#undef LogWarn2
#undef LogInfo2
#undef Log2

LoggerInternal::LoggerInternal()
{
}
LoggerInternal::~LoggerInternal()
{
	CloseLogFile();
}

void LoggerInternal::Log(const wchar_t * str, ...)
{
	if (level <= LogLevelText) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelText, str, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogWarn(const wchar_t * str, ...)
{
	if (level <= LogLevelWarn) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelWarn, str, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogError(const wchar_t * str, ...)
{
	if (level <= LogLevelError) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelError, str, arg);
		va_end(arg); 
	}
}
void LoggerInternal::LogInfo(const wchar_t * str, ...)
{
	if (level <= LogLevelInfo) {
		va_list arg;
		va_start(arg, str);
		LogInternal(LogLevelInfo, str, arg);
		va_end(arg);
	}
}

void LoggerInternal::Log2(const wchar_t * str, const char * file, int line, const char * functon, ...)
{
	if (level <= LogLevelText) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelText, str, file, line, functon, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogWarn2(const wchar_t * str, const char * file, int line, const char * functon, ...)
{
	if (level <= LogLevelWarn) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelWarn, str, file, line, functon, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogError2(const wchar_t * str, const char * file, int line, const char * functon, ...)
{
	if (level <= LogLevelError) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelError, str, file, line, functon, arg);
		va_end(arg);
	}
}
void LoggerInternal::LogInfo2(const wchar_t * str, const char * file, int line, const  char * functon, ...)
{
	if (level <= LogLevelInfo) {
		va_list arg;
		va_start(arg, functon);
		LogInternalWithCodeAndLine(LogLevelInfo, str, file, line, functon, arg);
		va_end(arg);
	}
}

void LoggerInternal::SetLogLevel(LogLevel level)
{
	this->level = level;
}
void LoggerInternal::SetLogOutPut(LogOutPut output)
{
	this->outPut = output;
}
void LoggerInternal::SetLogOutPutFile(const wchar_t * filePath)
{
	if (!StrEqual(logFilePath, filePath))
	{
		CloseLogFile();
		wcsncpy_s(logFilePath, filePath, MAX_PATH);
		_wfopen_s(&logFile, logFilePath, L"w");
	}
}
void LoggerInternal::SetLogOutPutCallback(LogCallBack callback, LPARAM lparam)
{
	callBack = callback;
	callBackData = lparam;
}

void LoggerInternal::ResentNotCaputureLog()
{
	if (outPut == LogOutPutCallback && callBack) {
		std::list< LOG_SLA>::iterator i;
		for (i = logPendingBuffer.begin(); i != logPendingBuffer.end(); i++)
			callBack((*i).str.c_str(), (*i).level, callBackData);
		logPendingBuffer.clear();
	}
}
void LoggerInternal::WritePendingLog(const wchar_t * str, LogLevel level)
{
	LOG_SLA sla = { std::wstring(str), level };
	logPendingBuffer.push_back(sla);
}

void LoggerInternal::LogInternalWithCodeAndLine(LogLevel level, const wchar_t * str, const char * file, int line, const char * functon, va_list arg)
{
	wstring format1 = FormatString(L"%s\n[In] %hs (%d) : %hs", str, file, line, functon);
	LogInternal(level, format1.c_str(), arg);
}
void LoggerInternal::LogInternal(LogLevel level, const wchar_t * str, va_list arg)
{
	const wchar_t*levelStr = L"";
	switch (level)
	{
	case LogLevelInfo: levelStr = L"I"; break;
	case LogLevelWarn: levelStr = L"W"; break;
	case LogLevelError: levelStr = L"E"; break;
	case LogLevelText: levelStr = L"T"; break;
	}
	time_t time_log = time(NULL);
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);

	wstring format1 = FormatString(L"[%02d:%02d:%02d] [%s] %s\n", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec, levelStr, str);
	wstring out = FormatString(format1.c_str(), arg);

	LogOutput(level, out.c_str(), out.size());
}
void LoggerInternal::LogOutput(LogLevel level, const wchar_t * str, size_t len)
{
#if _DEBUG
	OutputDebugString(str);
#else 
	if (outPut == LogOutPutConsolne)
		OutputDebugString(str);
#endif
	if (outPut == LogOutPutFile && logFile)
		fwprintf_s(logFile, L"%s", str);
	else if (outPut == LogOutPutCallback && callBack)
		callBack(str, level, callBackData);
	else
		WritePendingLog(str, level);
}

void LoggerInternal::CloseLogFile()
{
	if (logFile) {
		fclose(logFile);
		logFile = nullptr;
	}
}
