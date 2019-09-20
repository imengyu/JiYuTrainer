// JiYuTrainer.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuTrainer.h"
#include "App.h"
#include <stdarg.h>
#include <ShellAPI.h>
#include <dbghelp.h>

JTApp * currentApp = nullptr;
LoggerInternal * currentLogger = nullptr;

BOOL GenerateCrashInfo(PEXCEPTION_POINTERS pExInfo, LPCWSTR info_file_name, LPCWSTR file_name, SYSTEMTIME tm, LPCWSTR dir) {
	
	currentLogger->LogError(L"应用程序崩溃 ：Address : 0x%08x  Code : 0x%08X  (0x%08X)",
		(ULONG_PTR)pExInfo->ExceptionRecord->ExceptionAddress, pExInfo->ExceptionRecord->ExceptionCode,
		pExInfo->ExceptionRecord->ExceptionFlags);
	currentLogger->LogError(L"已创建运行转储文件：%s", file_name);

	FILE*fp = NULL;
	_wfopen_s(&fp, info_file_name, L"w");
	if (fp) {
		fwprintf_s(fp, L"=== JiYuTrainer ===== %04d/%02d/%02d %02d:%02d:%02d ===========", tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);
		fwprintf_s(fp, L"\r\n应用程序错误 ：Address : 0x%08x  Code : 0x%08X  (0x%08X)",
			(ULONG_PTR)pExInfo->ExceptionRecord->ExceptionAddress, pExInfo->ExceptionRecord->ExceptionCode,
			pExInfo->ExceptionRecord->ExceptionFlags);
		fwprintf_s(fp, L"\r\n=== JiYuTrainer =====================================");
		fwprintf_s(fp, L"\r\n我们生成了关于描述这个错误的错误报告(不包含您的个人信息)：");
		fwprintf_s(fp, L"\r\n=== 文件内容 =====================================");
		fwprintf_s(fp, L"\r\n[错误转储文件] %s", file_name);
		fwprintf_s(fp, L"\r\n[程序运行日志] %s\\JiYuTrainer.log", dir);
		fwprintf_s(fp, L"\r\n=== %hs =================================", CURRENT_VERSION);
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}
LONG GenerateMiniDump(PEXCEPTION_POINTERS pExInfo)
{
	TCHAR dmp_path[MAX_PATH];
	wcscpy_s(dmp_path, currentApp->GetCurrentDir());

	SYSTEMTIME tm;
	GetLocalTime(&tm);//获取时间
	TCHAR file_name[128];
	swprintf_s(file_name, L"%s\\JiYuTrainerCrashDump%d%02d%02d-%02d%02d%02d.dmp", dmp_path,
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);//设置dmp文件名称
	TCHAR info_file_name[128];
	swprintf_s(info_file_name, L"%s\\JiYuTrainerCrashInfo%d%02d%02d-%02d%02d%02d.txt", dmp_path,
		tm.wYear, tm.wMonth, tm.wDay, tm.wHour, tm.wMinute, tm.wSecond);

	//Create file
	HANDLE hFile = CreateFile(file_name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	//Generate Crash info
	BOOL hasCrashInfo = GenerateCrashInfo(pExInfo, info_file_name, file_name, tm, dmp_path);

	//Gen Dump File and show dialog

	TCHAR expInfo[128];
	swprintf_s(expInfo, L"Exception !!! Address : 0x%08x  Code : 0x%08X  (0x%08X)",
		(ULONG_PTR)pExInfo->ExceptionRecord->ExceptionAddress, pExInfo->ExceptionRecord->ExceptionCode,
		pExInfo->ExceptionRecord->ExceptionFlags);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION expParam;
		expParam.ThreadId = GetCurrentThreadId();
		expParam.ExceptionPointers = pExInfo;
		expParam.ClientPointers = FALSE;
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpWithDataSegs, (pExInfo ? &expParam : NULL), NULL, NULL);
		CloseHandle(hFile);

		TCHAR info[300];
		swprintf_s(info, L"应用程序出现了一个错误，%s。\n%s", (hasCrashInfo ? L"需要关闭，已生成关于此错误的报告" : L"并且无法生成错误转储文件"), expInfo);
		MessageBoxTimeoutW(NULL, info, L"JiYuTrainer 应用程序错误", MB_ICONERROR | MB_SYSTEMMODAL, 0, 3600);

		if (hasCrashInfo)
		{
			WCHAR arg[320];
			swprintf_s(arg, L"-bugreport -bugfile \"%s\"", info_file_name);
			ShellExecute(NULL, L"open", currentApp->GetFullPath(), arg, NULL, SW_NORMAL);
		}
	}
	else
	{
		TCHAR info[300];
		swprintf_s(info, L"应用程序出现了一个错误，并且无法生成错误转储文件。\n%s\nFail to create dump file: %s \nLast Error : %d\n现在应用程序即将关闭。", expInfo, file_name, GetLastError());
		MessageBox(NULL, info, L"JiYuTrainer 应用程序错误", MB_ICONERROR | MB_SYSTEMMODAL);
	}
	return EXCEPTION_EXECUTE_HANDLER;
}
EXPORT_CFUNC(LONG) WINAPI AppUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;
	return GenerateMiniDump(pExInfo);
}

EXPORT_CFUNC(void*) GetSciterAPI()
{
	return currentApp->GetSciterAPI();
}
EXPORT_CFUNC(VOID) ShowAppStartupFail(int rs) {
	if (rs == APP_FAIL_SYSTEM_NOT_SUPPORT)
		MessageBox(NULL, L"运行本程序最低要求 Windows XP，请使用更高版本的系统", L"JiYuTrainer - 错误", MB_ICONERROR);
	else if (rs == APP_FAIL_ALEDAY_RUN)
		MessageBox(0, L"已经有一个程序正在运行，同时只能运行一个实例，请关闭之前那个", L"JiYuTrainer - 错误", MB_ICONERROR);
	else if (rs == APP_FAIL_PIRACY_VERSION)
		MessageBox(0, L"您可能是盗版软件的受害者，您现在运行的是被修改过的 JiYuTrainer ，我们建议您至官网下载使用安全版本。", L"JiYuTrainer - 错误", MB_ICONERROR);
	else if (rs == APP_FAIL_INSTALL) {
		FAST_STR_BINDER(err, L"安装失败，具体错误请查看日志文件 : \n%s", 300, currentApp->GetPartFullPath(PART_LOG));
		MessageBox(0, err, L"JiYuTrainer - 错误", MB_ICONERROR);
	}
}
EXPORT_CFUNC(void*) GetApp() { return currentApp; }
EXPORT_CFUNC(void) ForceExit() { currentApp->Exit(0); }

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	SetUnhandledExceptionFilter(NULL);
	SetUnhandledExceptionFilter(AppUnhandledExceptionFilter);

	currentLogger = new LoggerInternal();
	currentLogger->SetLogOutPut(LogOutPutConsolne);
	currentLogger->SetLogLevel(LogLevelText);
	currentApp = new JTAppInternal(hInstance);
	currentApp->Run(nCmdShow);
	int rs = currentApp->GetResult();
	ShowAppStartupFail(rs);
	delete currentApp;
	delete currentLogger;

	return rs;
}