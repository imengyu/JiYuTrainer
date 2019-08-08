#include "stdafx.h"
#include "resource.h"
#include "JiYuTrainer.h"
#include "App.h"
#include "PathHelper.h"
#include "MD5Utils.h"
#include "SysHlp.h"
#include "StringHlp.h"
#include "NtHlp.h"
#include "KernelUtils.h"
#include "DriverLoader.h"
#include "TxtUtils.h"
#include <Shlwapi.h>
#include <winioctl.h>
#include <CommCtrl.h>
#include <ShellAPI.h>
#include <dbghelp.h>
#include "../JiYuTrainerUI/MainWindow.h"

extern LoggerInternal * currentLogger;
extern JTApp * currentApp;

JTAppInternal::JTAppInternal(HINSTANCE hInstance)
{
	this->hInstance = hInstance;
	this->_DialogBoxParamW = (fnDialogBoxParamW)GetProcAddress(GetModuleHandle(L"user32.dll"), "DialogBoxParamW");
}
JTAppInternal::~JTAppInternal()
{
	ExitClear();
}

int JTAppInternal::CheckAndInstall()
{
	//是可移动设备中比如U盘
	if (!appIsInstaller && !appForceIntallInCurrentDir && SysHlp::CheckIsPortabilityDevice(fullDir.c_str()))
	{
		//则复制本体和ini至临时目录，然后使用bat启动（可以不占用u盘，方便弹出）
		//创建临时目录
		WCHAR szTempPath[MAX_PATH];
		GetTempPath(MAX_PATH, szTempPath);
		wcscat_s(szTempPath, L"\\JiYuTrainer");
		if (!Path::Exists(szTempPath) && !CreateDirectory(szTempPath, NULL)) {
			appLogger->LogError2(L"创建临时目录失败：%s (%d)", PRINT_LAST_ERROR_STR);
			return -1;
		}
		WCHAR szTempMainPath[MAX_PATH];
		wcscpy_s(szTempMainPath, szTempPath);
		wcscat_s(szTempMainPath, L"\\JiYuTrainer.exe");
		//复制本体
		if (!CopyFile(fullPath.c_str(), szTempMainPath, FALSE)) {
			appLogger->LogError2(L"创建主程序失败：%s (%d)", PRINT_LAST_ERROR_STR);
			return -1;
		}

		std::wstring runBatContent = FormatString(L"/c start \"\" \"%s\" -f \"%s\"", szTempMainPath, fullPath.c_str());
		//启动 exe 并转交控制权
		appIsInstaller = true;
		if (!SysHlp::RunApplicationPriviledge(L"cmd", runBatContent.c_str())) {
			appLogger->LogError2(L"启动主程序失败：%s (%d)", PRINT_LAST_ERROR_STR);
			return -1;
		}
		return 0;
	}

	//安装HOOK dll
	//安装驱动文件
	if (!Path::Exists(fullHookerPath) && InstallResFile(hInstance, MAKEINTRESOURCE(IDR_DLL_HOOKS), L"BIN", fullHookerPath.c_str()) != EXTRACT_RES::ExtractSuccess)
		return -1;
	if (!Path::Exists(fullDriverPath) && InstallResFile(hInstance, MAKEINTRESOURCE(IDR_DLL_DRIVER), L"BIN", fullDriverPath.c_str()) != EXTRACT_RES::ExtractSuccess)
		return -1;

	//更新器
	if (appIsInstaller) {
		//须更新主exe
		std::wstring mainExePath = fullDir + L"\\JiYuTrainer.exe";
		if (Path::Exists(mainExePath) && !DeleteFile(mainExePath.c_str())) {
			appLogger->LogError2(L"无法更新原主exe ：%s (%d)", PRINT_LAST_ERROR_STR);
			return -1;
		}
		//更新来源exe
		if (Path::Exists(fullSourceInstallerPath) && DeleteFile(fullSourceInstallerPath.c_str()) && !CopyFile(fullPath.c_str(), fullSourceInstallerPath.c_str(), TRUE)) 
			appLogger->LogError2(L"无法更新原源 exe ：%s (%d) %s", PRINT_LAST_ERROR_STR, fullSourceInstallerPath.c_str());
		if (CopyFile(fullPath.c_str(), mainExePath.c_str(), TRUE)) {
			//启动已更新完成的主程序，并删除本体
			SysHlp::RunApplicationPriviledge(mainExePath.c_str(), FormatString(L"-rc %s", 300, fullPath.c_str()).c_str());
			return 0;
		}
		else {
			appLogger->LogError2(L"创建主 exe 失败：%s (%d) %s", PRINT_LAST_ERROR_STR, mainExePath.c_str());
			return -1;
		}
	}
	else {
		//加载sciter到内存
		HRSRC hResource = FindResourceW(hInstance, MAKEINTRESOURCE(IDR_DLL_SCITER), L"BIN");
		if (hResource) {
			HGLOBAL hg = LoadResource(hInstance, hResource);
			if (hg) {
				LPVOID pData = LockResource(hg);
				if (pData)
				{
					DWORD dwSize = SizeofResource(hInstance, hResource);
					pMemSciterdll = MemoryLoadLibrary(pData, dwSize);
					if (pMemSciterdll != NULL)
					{
						pSciterAPI = MemoryGetProcAddress(pMemSciterdll, "SciterAPI");
						return 0;
					}
				}
			}
		}
		appLogger->LogError2(L"读取文件失败，资源提取错误：%s (%d)", PRINT_LAST_ERROR_STR);
	}

	return -1;
}
void JTAppInternal::UnInstall() 
{
	//卸载病毒
	if (appWorker) {
		appWorker->RunOperation(TrainerWorkerOpVirusBoom);
		appWorker->RunOperation(TrainerWorkerOpForceUnLoadVirus);
	}
	//稍后删除本体
	Sleep(1000);

	//删除模块
	if (Path::Exists(fullDriverPath)) DeleteFile(fullDriverPath.c_str());
	if (Path::Exists(fullHookerPath)) DeleteFile(fullHookerPath.c_str());

	//写入删除本体exe的bat
	std::wstring uninstallBatPath = fullDir + L"\\uninstall-final.bat";
	std::wstring uninstallBatContent = L"@echo off\n\
@ping 127.0.0.1 -n 6 > nul\n\
del /F /Q " + fullPath + L"\n\
del /F /Q " + fullIniPath + L"\n\
del %0";

	if(TxtUtils::WriteStringToTxt(uninstallBatPath, uninstallBatContent))
		SysHlp::RunApplicationPriviledge(uninstallBatPath.c_str(), NULL);

	TerminateProcess(GetCurrentProcess(), 0);
}

EXTRACT_RES JTAppInternal::InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo)
{
	appLogger->Log(L"安装模块文件：(%d) %s", resId, extractTo);

	EXTRACT_RES result = ExtractUnknow;
	HANDLE hFile = CreateFile(extractTo, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) 
	{
		result = ExtractCreateFileError;
		appLogger->LogError2(L"创建模块文件 失败：%s (%d)  %s", PRINT_LAST_ERROR_STR, extractTo);
		return result;
	}

	HRSRC hResource = FindResourceW(resModule, resId, resType);
	if (hResource) {
		HGLOBAL hg = LoadResource(resModule, hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
			{
				DWORD dwSize = SizeofResource(resModule, hResource);
				DWORD writed;
				if (WriteFile(hFile, pData, dwSize, &writed, NULL)) {
					SetFileAttributes(extractTo, FILE_ATTRIBUTE_HIDDEN);
					CloseHandle(hFile);
					result = ExtractSuccess;
					return result;
				}
				else result = ExtractWriteFileError;;
			}
			else result = ExtractReadResError;
		}
		else result = ExtractReadResError;
	}
	else result = ExtractReadResError;
	appLogger->LogError2(L"创建模块文件失败，资源提取错误：%s (%d) %s ", PRINT_LAST_ERROR_STR, extractTo);
	CloseHandle(hFile);
	return result;
}
bool JTAppInternal::IsCommandExists(LPCWSTR cmd)
{
	return FindArgInCommandLine(appArgList, appArgCount, cmd) >= 0;
}
int JTAppInternal::FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg) {
	for (int i = 0; i < argCount; i++) {
		if (wcscmp(szArgList[i], arg) == 0)
			return i;
	}
	return -1;
}

LPCWSTR JTAppInternal::MakeFromSourceArg(LPCWSTR arg)
{
	if (!fullSourceInstallerPath.empty()) {
		fullArgBuffer = L"-f " + fullSourceInstallerPath + L" " + arg;
		return fullArgBuffer.c_str();
	}
	return arg;
}

int JTAppInternal::Run(int nCmdShow)
{
	this->appShowCmd = nCmdShow;
	appResult = RunInternal();
	this->Exit(appResult);
	return appResult;
}
int JTAppInternal::RunCheckRunningApp()//如果程序已经有一个在运行，则返回true
{
	HWND oldWindow = FindWindow(MAIN_WND_CLS_NAME, MAIN_WND_NAME);
	if (oldWindow != NULL) {
		if (!IsWindowVisible(oldWindow)) ShowWindow(oldWindow, SW_SHOW);
		if (IsIconic(oldWindow)) ShowWindow(oldWindow, SW_RESTORE);
		SetForegroundWindow(oldWindow);
		return -1;
	}
	HANDLE  hMutex = CreateMutex(NULL, FALSE, L"JYTMutex");
	if (hMutex && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		CloseHandle(hMutex);
		hMutex = NULL;
		return 1;
	}
	return 0;
}
bool JTAppInternal::RunArgeementDialog()
{
	int rs = _DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_ARGEEMENT), NULL, ArgeementWndProc, NULL);
	if (rs == IDYES) {
		appSetting->SetSettingBool(L"Argeed", true, L"JTArgeement");
		return true;
	}
	return false;
}

int JTAppInternal::RunInternal()
{
	setlocale(LC_ALL, "chs");
	SetUnhandledExceptionFilter(NULL);
	SetUnhandledExceptionFilter(UnhandledExceptionFilter);

	MLoadNt();

	if (SysHlp::GetSystemVersion() == SystemVersionNotSupport)
		return APP_FAIL_SYSTEM_NOT_SUPPORT;

	InitPrivileges();
	InitLogger();
	InitPath();
	InitCommandLine();
	InitArgs();
	InitSettings();

	if (!CheckAppCorrectness())
		return APP_FAIL_PIRACY_VERSION;

	if (appArgBreak) {
#ifdef _DEBUG
		if (MessageBox(NULL, L"This is a Debug version", L"JiYuTrainer - Debug Break", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DebugBreak();
#else
		if (MessageBox(NULL, L"This is a Release version", L"JiYuTrainer - Debug Break", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DebugBreak();
#endif
	}
	if (!appArgInstallMode && !appArgeementArgeed && !RunArgeementDialog())
		return 0;

	//模式选择 
	if (appIsBugReportMode) {
		appStartType = AppStartTypeBugReport;
		goto RUN_MAIN;
	}
	if (appIsConfigMode) {
		appStartType = AppStartTypeConfig;
		goto RUN_MAIN;
	}

	//指定日志为文件模式
	appLogger->SetLogOutPut(LogOutPutFile);
	appLogger->SetLogOutPutFile(fullLogPath.c_str());

	if (appArgRemoveUpdater) 
	{
		Sleep(1000);//Sleep for a while

		//删除原有更新程序的本体以及日志
		WCHAR updaterLogPath[MAX_PATH];
		wcscpy_s(updaterLogPath, updaterPath.c_str());
		PathRenameExtension(updaterLogPath, L".log");
		if (Path::Exists(updaterLogPath) && !DeleteFileW(updaterLogPath))
			currentLogger->LogError(L"Remove updater file %s failed : %d", updaterPath.c_str(), GetLastError());
		if (!DeleteFileW(updaterPath.c_str()))
			currentLogger->LogError(L"Remove updater file %s failed : %d", updaterPath.c_str(), GetLastError());
	}
	if (!appIsRecover) {

		int oldStatus = RunCheckRunningApp();
		if (oldStatus == 1)
			return APP_FAIL_ALEDAY_RUN;
		if (oldStatus == -1)
			return 0;
	}

	//Install modules
	if (CheckAndInstall()) return APP_FAIL_INSTALL;
	if (appIsInstaller) return 0;

	//appLogger->Log(L"SetUnhandledExceptionFilter Prevented: %d", PreventSetUnhandledExceptionFilter());
	appWorker = new TrainerWorkerInternal();
	appLogger->Log(L"初始化正常");

RUN_MAIN:

	if (appStartType == AppStartTypeNormal) return JiYuTrainerUICommonEntry(0);
	else if (appStartType == AppStartTypeUpdater)  return JiYuTrainerUICommonEntry(1);
	else if (appStartType == AppStartTypeConfig) return JiYuTrainerUICommonEntry(2);
	else if (appStartType == AppStartTypeBugReport)  return JiYuTrainerUICommonEntry(3);

	return 0;
}

void JTAppInternal::Exit(int code)
{
	ExitInternal();
	ExitClear();
	//ExitProcess(code);
}
bool JTAppInternal::ExitInternal()
{

	return false;
}
void JTAppInternal::ExitClear()
{
	if (pMemSciterdll) {
		MemoryFreeLibrary(pMemSciterdll);
		pMemSciterdll = NULL;
	}
	if (XDriverLoaded()) {
		XCloseDriverHandle();
		//XUnLoadDriver();
	}
	if (appArgList) {
		LocalFree(appArgList);
		appArgList = nullptr;
	}
	if (appWorker) {
		delete appWorker;
		appWorker = nullptr;
	}
	if (appSetting) {
		delete appSetting;
		appSetting = nullptr;
	}
}

LPCWSTR JTAppInternal::GetPartFullPath(int partId)
{
	if (partId == PART_MAIN)
		return fullPath.c_str();
	if (partId == PART_INI) 
		return fullIniPath.c_str();
	if (partId == PART_HOOKER)
		return fullHookerPath.c_str();
	if (partId == PART_DRIVER)
		return fullDriverPath.c_str();
	if (partId == PART_LOG)
		return fullLogPath.c_str();
	return NULL;
}
LPVOID JTAppInternal::RunOperation(AppOperation op)
{
	switch (op)
	{
	case AppOperation1: LoadDriver(); break;
	case AppOperation2: MUnLoadKernelDriver(L"TDProcHook"); break;
	case AppOperationUnLoadDriver: {
		XCloseDriverHandle();
		if (XUnLoadDriver())
			currentLogger->Log(L"驱动卸载成功");
		break;
	}
	case AppOperationKReboot:  KFReboot(); break;
	case AppOperationKShutdown: KFShutdown();  break;
	case AppOperationForceLoadDriver: XLoadDriver(); break;
	default:
		break;
	}
	return nullptr;
}

void JTAppInternal::LoadDriver()
{
	if (!appForceNoDriver && XLoadDriver())
		if (!appForceNoSelfProtect && !XInitSelfProtect())
			currentLogger->LogWarn(L"驱动自我保护失败！");
}
bool JTAppInternal::CheckAppCorrectness() 
{
	if(appSetting->GetSettingStr(L"IgnoreCorrectness") == L"20190711")
		return true;

	SYSTEMTIME time;
	HANDLE hFileRead = CreateFileW(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	FILETIME file_time;
	FILETIME locationtime;

	GetFileTime(hFileRead, NULL, NULL, &file_time);//获得文件修改时间  
	FileTimeToLocalFileTime(&file_time, &locationtime);//将文件时间转换为本地文件时间  
	FileTimeToSystemTime(&locationtime, &time);

	CloseHandle(hFileRead);

	if (time.wYear > 2019 || time.wMonth > 7 || time.wDay > 11)
		return false;

	return true;
}

void JTAppInternal::MergePathString()
{
	fullDriverPath = fullDir + L"\\JiYuTrainerDriver.sys";
	fullHookerPath = fullDir + L"\\JiYuTrainerHooks.dll";
}
void JTAppInternal::InitPath()
{
	WCHAR buffer[MAX_PATH];
	GetModuleFileName(hInstance, buffer, MAX_PATH);
	fullPath = buffer;

	PathRemoveFileSpec(buffer);
	fullDir = buffer;

	GetModuleFileName(hInstance, buffer, MAX_PATH);
	PathRenameExtension(buffer, L".ini");
	fullIniPath = buffer;
	PathRenameExtension(buffer, L".log");
	fullLogPath = buffer;

	appIsInstaller = Path::GetFileName(fullPath) == L"JiYuTrainerUpdater.exe";
	appNeedInstallIniTemple = !Path::Exists(fullIniPath);

	MergePathString();
}
void JTAppInternal::InitCommandLine()
{
	appArgList = CommandLineToArgvW(GetCommandLine(), &appArgCount);
	if (appArgList == NULL)
	{
		MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
		ExitProcess( -1);
	}
}
void JTAppInternal::InitArgs()
{
	if (FindArgInCommandLine(appArgList, appArgCount, L"-install-full") != -1) appArgInstallMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-force-md5-check") != -1) appArgForceCheckFileMd5 = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-b") != -1) appArgBreak = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-break") != -1) appArgBreak = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-r1") != -1) appIsRecover = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-h") != -1) appIsHiddenMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-hidden") != -1) appIsHiddenMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-config") != -1) appIsConfigMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-bugreport") != -1) appIsBugReportMode = true;

	int argFIndex = FindArgInCommandLine(appArgList, appArgCount, L"-f");
	if (argFIndex >= 0 && (argFIndex + 1) < appArgCount) {
		fullSourceInstallerPath = appArgList[argFIndex + 1];
		if (Path::Exists(fullSourceInstallerPath)) {
			WCHAR buffer[MAX_PATH];
			wcscpy_s(buffer, fullSourceInstallerPath.c_str());
			PathRenameExtension(buffer, L".ini");
			if (Path::Exists(fullSourceInstallerPath))
				fullIniPath = buffer;
		}
	}
	argFIndex = FindArgInCommandLine(appArgList, appArgCount, L"-rc");
	if (argFIndex >= 0 && (argFIndex + 1) < appArgCount) {
		LPCWSTR updaterFullPath = appArgList[argFIndex + 1];
		if (Path::Exists(updaterFullPath)) {
			updaterPath = updaterFullPath;
			appArgRemoveUpdater = true;
		}
	}
}
void JTAppInternal::InitLogger()
{
	appLogger = currentLogger;
	appLogger->SetLogOutPut(LogOutPutConsolne);
}
void JTAppInternal::InitPrivileges()
{
	SysHlp::EnableDebugPriv(SE_DEBUG_NAME);
	SysHlp::EnableDebugPriv(SE_SHUTDOWN_NAME);
	SysHlp::EnableDebugPriv(SE_LOAD_DRIVER_NAME);
}
void JTAppInternal::InitSettings()
{
	appSetting = new SettingHlpInternal(fullIniPath.c_str());

	appArgeementArgeed = appSetting->GetSettingBool(L"Argeed", false, L"JTArgeement");
	appForceNoDriver = appSetting->GetSettingBool(L"DisableDriver", false);
	appForceNoSelfProtect = !appSetting->GetSettingBool(L"SelfProtect", true);
	appForceIntallInCurrentDir = appSetting->GetSettingBool(L"ForceInstallInCurrentDri", false);
}

HFONT JTAppInternal::hFontRed = NULL;
HINSTANCE JTAppInternal::hInstance = NULL;

INT_PTR CALLBACK JTAppInternal::ArgeementWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;

	switch (message)
	{
	case WM_INITDIALOG: {

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_MAIN)));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_MAIN)));

		hFontRed = CreateFontW(20, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"宋体");//创建字体
		SendDlgItemMessage(hDlg, IDC_STATIC_RED, WM_SETFONT, (WPARAM)hFontRed, TRUE);//发送设置字体消息

		lResult = TRUE;
		break;
	}
	case WM_COMMAND: EndDialog(hDlg, wParam); lResult = wParam;  break;
	case WM_CTLCOLORSTATIC: {
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_STATIC_RED))  SetTextColor((HDC)wParam, RGB(255, 0, 0));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}
	case WM_CTLCOLORDLG: {
		return (INT_PTR)(HBRUSH)GetStockObject(WHITE_BRUSH);
	}
	case WM_DESTROY: {
		DeleteObject(hFontRed);
		break;
	}
	default: return DefWindowProc(hDlg, message, wParam, lParam);
	}
	return lResult;
}

BOOL appGenerateMiniDumpLock = FALSE;

LPTOP_LEVEL_EXCEPTION_FILTER WINAPI JTAppInternal::MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter)
{
	return NULL;
}
BOOL JTAppInternal::PreventSetUnhandledExceptionFilter()
{
	HMODULE hKernel32 = LoadLibrary(L"kernel32.dll");
	if (hKernel32 == NULL) return FALSE;
	void *pOrgEntry = GetProcAddress(hKernel32, "SetUnhandledExceptionFilter");
	if (pOrgEntry == NULL) return FALSE;
	unsigned char newJump[100];
	DWORD dwOrgEntryAddr = (DWORD)pOrgEntry;
	dwOrgEntryAddr += 5; // add 5 for 5 op-codes for jmp far
	void *pNewFunc = &MyDummySetUnhandledExceptionFilter;
	DWORD dwNewEntryAddr = (DWORD)pNewFunc;
	DWORD dwRelativeAddr = dwNewEntryAddr - dwOrgEntryAddr;
	newJump[0] = 0xE9;  // JMP absolute
	memcpy(&newJump[1], &dwRelativeAddr, sizeof(pNewFunc));
	SIZE_T bytesWritten;
	BOOL bRet = WriteProcessMemory(GetCurrentProcess(), pOrgEntry, newJump, sizeof(pNewFunc) + 1, &bytesWritten);
	return bRet;
}
LONG WINAPI JTAppInternal::UnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo)
{
	// 这里做一些异常的过滤或提示
	if (IsDebuggerPresent())
		return EXCEPTION_CONTINUE_SEARCH;
	if (!appGenerateMiniDumpLock) {
		appGenerateMiniDumpLock = TRUE;
		return GenerateMiniDump(pExInfo);
	}
	return EXCEPTION_CONTINUE_SEARCH;
}
LONG JTAppInternal::GenerateMiniDump(PEXCEPTION_POINTERS pExInfo)
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
BOOL JTAppInternal::GenerateCrashInfo(PEXCEPTION_POINTERS pExInfo, LPCWSTR info_file_name, LPCWSTR file_name, SYSTEMTIME tm, LPCWSTR dir) {
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
		fwprintf_s(fp, L"\r\n[程序运行日志] %s\\%s", dir, L"JiYuTrainer.log");
		fwprintf_s(fp, L"\r\n=== %hs =================================", CURRENT_VERSION);
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

