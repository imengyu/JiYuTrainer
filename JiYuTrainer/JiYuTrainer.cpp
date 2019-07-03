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
	currentLogger = new LoggerInternal();
	currentLogger->SetLogOutPut(LogOutPutConsolne);
	currentLogger->SetLogLevel(LogLevelText);
	currentApp = new JTAppInternal(hInstance);
	currentApp->Run(nCmdShow);
	int rs = currentApp->GetResult();
	delete currentApp;
	delete currentLogger;
	return rs;
}

EXPORT_CFUNC(void*) JTGetApp() { return currentApp; }
EXPORT_CFUNC(void) JTForceExit() { currentApp->Exit(0); }