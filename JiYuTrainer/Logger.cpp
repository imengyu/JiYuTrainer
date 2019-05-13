#include "stdafx.h"
#include "Logger.h"
#include <time.h>
#include "StringHlp.h"
#include "PathHelper.h"

using namespace std;

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
	wcsncpy_s(logFilePath, filePath, MAX_PATH);
	if (Path::Exists(logFilePath)) {
		CloseLogFile();
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

#if _DEBUG
	OutputDebugString(out.c_str());
#else 
	if (outPut == LogOutPutConsolne)
		OutputDebugString(out.c_str());
#endif
	if (outPut == LogOutPutFile && logFile)
		fwprintf_s(logFile, out.c_str());
	else if (outPut == LogOutPutCallback && callBack)
		callBack(out.c_str(), level, callBackData);
	else 
		WritePendingLog(out.c_str(), level);
}

void LoggerInternal::CloseLogFile()
{
	if (logFile) {
		fclose(logFile);
		logFile = nullptr;
	}
}
