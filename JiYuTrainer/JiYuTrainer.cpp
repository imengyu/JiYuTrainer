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
	JTShowAppStartupFail(rs);
	delete currentApp;
	delete currentLogger;
	return rs;
}

EXPORT_CFUNC(VOID) JTShowAppStartupFail(int rs) {
	if (rs == APP_FAIL_SYSTEM_NOT_SUPPORT)
		MessageBox(NULL, L"运行本程序最低要求 Windows XP，请使用更高版本的系统", L"JiYuTrainer - 错误", MB_ICONERROR);
	else if (rs == APP_FAIL_MAIN_PART_BROKED)
		MessageBox(NULL, L"加载主部件发生错误。主部件损坏", L"JiYuTrainer - 错误", MB_ICONERROR);
	else if (rs == APP_FAIL_MAIN_PART_LOADERR)
		MessageBox(NULL, currentApp->GetStartupErr(), L"JiYuTrainer - 错误", MB_ICONERROR);
	else if (rs == APP_FAIL_ALEDAY_RUN)
		MessageBox(0, L"已经有一个程序正在运行，同时只能运行一个实例，请关闭之前那个", L"JiYuTrainer - 错误", MB_ICONERROR);
}
EXPORT_CFUNC(void*) JTGetApp() { return currentApp; }
EXPORT_CFUNC(void) JTForceExit() { currentApp->Exit(0); }