// JiYuTrainer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuTrainer.h"
#include "App.h"
#include <stdarg.h>

JTApp * currentApp = nullptr;
LoggerInternal * currentLogger = nullptr;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	currentApp = new JTAppInternal(hInstance);
	currentApp->Run();
	int rs = currentApp->GetResult();
	delete currentApp;
	return rs;
}

EXPORT_CFUNC(void*) JTGetCurrentApp() { return currentApp; }
EXPORT_CFUNC(void) JTForceExit() { currentApp->Exit(0); }
EXPORT_CFUNC(void) JTLog(const wchar_t *str, ...) {
	if (currentLogger) {
		va_list arg;
		va_start(arg, str);
		currentLogger->Log(str, arg);
		va_end(arg);
	}
}
EXPORT_CFUNC(void) JTLogInfo(const wchar_t *str, ...) {
	if (currentLogger) {
		va_list arg;
		va_start(arg, str);
		currentLogger->LogInfo(str, arg);
		va_end(arg);
	}
}
EXPORT_CFUNC(void) JTLogWarn(const wchar_t *str, ...) {
	if (currentLogger) {
		va_list arg;
		va_start(arg, str);
		currentLogger->LogWarn(str, arg);
		va_end(arg);
	}
}
EXPORT_CFUNC(void) JTLogError(const wchar_t *str, ...) {
	if (currentLogger) {
		va_list arg;
		va_start(arg, str);
		currentLogger->LogError(str, arg);
		va_end(arg);
	}
}