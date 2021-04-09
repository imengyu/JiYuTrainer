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
#include "JyUdpAttack.h"
#include "RegHlp.h"
#include "TxtUtils.h"
#include "XUnzip.h"
#include <Shlwapi.h>
#include <winioctl.h>
#include <CommCtrl.h>
#include <ShellAPI.h>
#include <dbghelp.h>
#include <locale>
#include "../JiYuTrainerUI/MainWindow.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#define CMD_HELP L"\
工具命令：\n\
-install-full 开始完整安装命令\n\
-config       打开 JiYuTrainer 高级配置\n\
-hidden       静默运行模式\n\
-killst       杀死极域电子教室\n\
\n\
调试命令：\n\
-bugreport -bugfile [bugFilePath]\n\
-break\n\
-crash-test\n\
-force-md5-check\n\
\n\
程序内部命令：\n\
-f [sourceFilePath]\n\
-r[1|2|3]\n\
-ic\n\
-rc\n\
-b\n\
-h\n\
"

extern LoggerInternal * currentLogger;
extern JTApp * currentApp;

extern NtCloseFun NtClose;

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
		WCHAR szTempMainStartBatPath[MAX_PATH];
		wcscpy_s(szTempMainPath, szTempPath);
		wcscat_s(szTempMainPath, L"\\JiYuTrainer.exe");
		wcscpy_s(szTempMainStartBatPath, szTempPath);
		wcscat_s(szTempMainStartBatPath, L"\\JiYuTrainerStart.bat");
		//复制本体
		if (!CopyFile(fullPath.c_str(), szTempMainPath, FALSE)) {
			appLogger->LogError2(L"创建主程序失败：%s (%d)", PRINT_LAST_ERROR_STR);
			return -1;
		}

		bool useBatStart = false;

		//写入启动bat
		std::wstring szTempMainStartBatPathwz;
		std::wstring szTempMainStartBatPathct = FormatString(L"start \"\" \"%s\" -f \"%s\"\nexit", szTempMainPath, fullPath.c_str());
		szTempMainStartBatPathwz = szTempMainStartBatPath;
		if (TxtUtils::WriteStringToTxt(szTempMainStartBatPathwz, szTempMainStartBatPathct)) useBatStart = true;

		appLogger->Log(L"Installer finish, start app : %s", szTempMainPath);

		//启动 exe 并转交控制权
		std::wstring runBatContent;
		if(useBatStart)
			runBatContent = FormatString(L"/c start \"\" \"%s\"", szTempMainStartBatPath);
		else 
			runBatContent = FormatString(L"/c start \"\" \"%s\" -f \"%s\"", szTempMainPath, fullPath.c_str());
		appIsInstaller = true;
		if (!SysHlp::RunApplicationPriviledge(L"cmd", runBatContent.c_str())) {
			appLogger->LogError2(L"启动主程序失败：%s (%d)", PRINT_LAST_ERROR_STR);
			return -1;
		}
		return 0;
	}

	//安装HOOK dll
	if (!Path::Exists(fullHookerPath) && InstallResFile(hInstance, MAKEINTRESOURCE(IDR_DLL_HOOKS), L"BIN", fullHookerPath.c_str()) != EXTRACT_RES::ExtractSuccess)
		return -1;
	//安装Sciter dll
	if (!Path::Exists(fullSciterPath) && !InstallSciter())
		return -1;

	//加载两个模块
	LoadLibrary(fullHookerPath.c_str());
	if (!LoadLibrary(fullSciterPath.c_str())) {
		appLogger->LogError2(L"模块 Sciter.dll 加载失败：%s (%d)", PRINT_LAST_ERROR_STR);
		return -1;
	}

	//安装驱动文件
	if (XTestDriverCanUse() && !Path::Exists(fullDriverPath))
		InstallResFile(hInstance, MAKEINTRESOURCE(IDR_DLL_DRIVER), L"BIN", fullDriverPath.c_str());

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
		//加载sciter
		HMODULE pSciterdll = LoadLibrary(L"sciter.dll");
		if (pSciterdll != NULL)
		{
			pSciterAPI = GetProcAddress(pSciterdll, "SciterAPI");
			if(pSciterAPI) return 0;
		}
		/*
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
		}*/
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

	if (Path::Exists(fullDriverPath)) DeleteFile(fullDriverPath.c_str());
	if (Path::Exists(fullLogPath)) DeleteFile(fullLogPath.c_str());
	if (Path::Exists(fullIniPath)) DeleteFile(fullIniPath.c_str());

	//写入删除本体exe的bat
	std::wstring uninstallBatPath = fullDir + L"\\uninstall-final.bat";
	std::wstring uninstallBatContent = FormatString(L"@echo off\n@ping 127.0.0.1 -n 6 > nul\ndel /F /Q %s\
\ndel /F /Q %s\ndel /F /Q %s\ndel %%%%0", fullPath.c_str(), fullHookerPath.c_str(), fullSciterPath.c_str());

	if(TxtUtils::WriteStringToTxt(uninstallBatPath, uninstallBatContent))
		SysHlp::RunApplicationPriviledge(uninstallBatPath.c_str(), NULL);

	TerminateProcess(GetCurrentProcess(), 0);
}

BOOL JTAppInternal::InstallSciter() {

	HRSRC hResource = NULL;
	HGLOBAL hGlobal = NULL;
	LPVOID pData = NULL;
	DWORD dwSize = NULL;
	HZIP hZip = NULL;
	int sciterZipIndex = 0;
	ZIPENTRYW sciterZipEntry = { 0 };
	ZRESULT zipResult = 0;
	void*buff = nullptr;

	HANDLE hFile = CreateFile(fullSciterPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		appLogger->LogError2(L"创建模块文件 失败：%s (%d)  %s", PRINT_LAST_ERROR_STR, fullSciterPath.c_str());
		return FALSE;
	}

	hResource = FindResourceW(hInstance, MAKEINTRESOURCE(IDR_DLL_SCITER), L"BIN");
	if (!hResource) goto RETURN;
	hGlobal = LoadResource(hInstance, hResource);
	if (!hGlobal) goto RETURN;
	pData = LockResource(hGlobal);
	if (!pData) goto RETURN;
	dwSize = SizeofResource(hInstance, hResource);
	if (!dwSize) goto RETURN;

	hZip = OpenZip(pData, dwSize, ZIP_MEMORY);
	if(!hZip) goto RETURN;

	zipResult = FindZipItem(hZip, L"sciter.dll", false, &sciterZipIndex, &sciterZipEntry);
	if (zipResult != ZR_OK) {
		appLogger->LogError2(L"FindZipItem failed : %d (0x%08X)", zipResult, zipResult);
		goto CLOSE_ZIP_RETURN;
	}
	if (sciterZipEntry.unc_size <= 0) {
		appLogger->LogError2(L"sciterZipEntry.unc_size unknow ");
		goto CLOSE_ZIP_RETURN;
	}

	zipResult = UnzipItem(hZip, sciterZipIndex, hFile, 0, ZIP_HANDLE);
	if (zipResult != ZR_OK) {
		appLogger->LogError2(L"UnzipItem failed : %d (0x%08X)", zipResult, zipResult);
		goto CLOSE_ZIP_RETURN;
	}

	CloseHandle(hFile);
	CloseZip(hZip);
	return TRUE;

CLOSE_ZIP_RETURN:
	CloseZip(hZip);
RETURN:
	CloseHandle(hFile);
	DeleteFile(fullSciterPath.c_str());
	return FALSE;
}
EXTRACT_RES JTAppInternal::InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo)
{
	appLogger->Log(L"安装模块文件：(%d) %s", resId, extractTo);

	EXTRACT_RES result = ExtractUnknow;
	HRSRC hResource = NULL;
	HGLOBAL hGlobal = NULL;
	LPVOID pData = NULL;
	DWORD dwSize = NULL;
	DWORD writed;
	HANDLE hFile = CreateFile(extractTo, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		result = ExtractCreateFileError;
		appLogger->LogError2(L"创建模块文件 失败：%s (%d)  %s", PRINT_LAST_ERROR_STR, extractTo);
		return result;
	}

	hResource = FindResourceW(resModule, resId, resType);
	if (!hResource) {
		result = ExtractReadResError;
		goto RETURN;
	}
	hGlobal = LoadResource(resModule, hResource);
	if (!hGlobal) {
		result = ExtractReadResError;
		goto RETURN;
	}
	pData = LockResource(hGlobal);
	if (!pData)
	{
		result = ExtractReadResError;
		goto RETURN;
	}
	dwSize = SizeofResource(resModule, hResource);
	if (!WriteFile(hFile, pData, dwSize, &writed, NULL)) {
		result = ExtractWriteFileError;
		appLogger->LogError2(L"创建模块文件失败，写入文件错误：%s (%d) %s ", PRINT_LAST_ERROR_STR, extractTo);
		CloseHandle(hFile);
		return result;
	}

	SetFileAttributes(extractTo, FILE_ATTRIBUTE_HIDDEN);
	CloseHandle(hFile);
	result = ExtractSuccess;
	return result;

RETURN:
	appLogger->LogError2(L"创建模块文件失败，资源提取错误：%s (%d) %s ", PRINT_LAST_ERROR_STR, extractTo);
	CloseHandle(hFile);
	return result;
}
bool JTAppInternal::IsCommandExists(LPCWSTR cmd)
{
	return FindArgInCommandLine(appArgList, appArgCount, cmd) >= 0;
}
int JTAppInternal::FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg) {
	WCHAR argBufferS[32];
	WCHAR argBufferR[32];
	swprintf_s(argBufferS, L"-%s", arg);
	swprintf_s(argBufferR, L"/%s", arg);
	for (int i = 0; i < argCount; i++) {
		if (wcscmp(szArgList[i], argBufferS) == 0 || wcscmp(szArgList[i], argBufferR) == 0)
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

void JTAppInternal::CloseSourceDir() {
	
	if (Path::Exists(fullSourceInstallerDir)) {
		HANDLE hDir = CreateFile(fullSourceInstallerDir.c_str(), GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hDir != INVALID_HANDLE_VALUE) {
			NtClose(hDir);
			NtClose(hDir);
		}
	}
}
bool JTAppInternal::CheckAntiVirusSoftware(bool showTip) {

	/*
	WCHAR wstr3601[8];
	WCHAR wstr3602[8];
	WCHAR wstr3603[8];
	WCHAR wstrDb1[8];

	copyStrFromIntArr(wstr3601, str3601, sizeof(str3601) / sizeof(int));
	copyStrFromIntArr(wstr3602, str3602, sizeof(str3602) / sizeof(int));
	copyStrFromIntArr(wstr3603, str3603, sizeof(str3603) / sizeof(int));
	copyStrFromIntArr(wstrDb1, strDb1, sizeof(wstrDb1) / sizeof(int));

	if (MRegCheckUninstallItemExists(wstr3601)
		|| MRegCheckUninstallItemExists(wstr3602)
		|| MRegCheckUninstallItemExists(wstr3603))
		existsAntiVirus += L"360";
	if (MRegCheckUninstallItemExists(wstrDb1))
		existsAntiVirus += L"、金山毒霸";

	if (existsAntiVirus != L"") {
		if (showTip)  _DialogBoxParamW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_AVTIP), NULL, AVTipWndProc, (LPARAM)this);
		return true;
	}
	
	*/
	return false;
}
void JTAppInternal::copyStrFromIntArr(wchar_t * buffer, int * arr, size_t len)
{
	for (size_t i = 0; i < len; i++)
		buffer[i] = (WCHAR)arr[i];
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

	MLoadNt();

	if (SysHlp::GetSystemVersion() == SystemVersionNotSupport) {
		appLogger->LogError2(L"系统版本不支持本软件的运行");
		return APP_FAIL_SYSTEM_NOT_SUPPORT;
	}

	InitPrivileges();
	InitLogger();
	InitPath();

	//指定日志为文件模式
	appLogger->SetLogOutPut(LogOutPutFile);
	appLogger->SetLogOutPutFile(fullLogPath.c_str());

	InitCommandLine();
	InitArgs();
	InitSettings();

	EnableVisualStyles();
	if (!CheckAppCorrectness())
		return APP_FAIL_PIRACY_VERSION;

	if (GetAsyncKeyState(VK_LMENU) && 0x8000 || GetAsyncKeyState(VK_RMENU) && 0x8000)
		appIsConfigMode = true;
	if (appArgBreak) {
#ifdef _DEBUG
		if (MessageBox(NULL, L"This is a Debug version", L"JiYuTrainer - Debug Break", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DebugBreak();
#else
		if (MessageBox(NULL, L"This is a Release version", L"JiYuTrainer - Debug Break", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			DebugBreak();
#endif
	}
	if (appCmdHelpMode) {
		wprintf_s(CMD_HELP);
		wprintf_s(L"\n");
		MessageBox(NULL, CMD_HELP, L"JiYuTrainer -命令行提示", MB_ICONINFORMATION);
		return 0;
	}
	if (appKillStMode) {
		TrainerWorkerInternal t;
		if (t.KillStAuto()) printf_s("已成功结束极域电子教室\n");
		else printf_s("无法结束极域电子教室，详情请查看日志\n");
		return 0;
	}
	if (!appArgInstallMode) {
		appForceNoDriver = CheckAntiVirusSoftware(appShowAvTest);
		if (appFirstUse) appSetting->SetSettingBool(L"FirstUse", false, L"JTArgeement");
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
	if (appArgRemoveUpdater) 
	{
		Sleep(1000);//Sleep for a while

		//删除原有更新程序的本体以及日志
		WCHAR updaterLogPath[MAX_PATH];
		wcscpy_s(updaterLogPath, updaterPath.c_str());
		PathRenameExtension(updaterLogPath, L".log");
		if (Path::Exists(updaterLogPath) && !DeleteFileW(updaterLogPath))
			currentLogger->LogError2(L"Remove updater file %s failed : %d", updaterPath.c_str(), GetLastError());
		if (!DeleteFileW(updaterPath.c_str()))
			currentLogger->LogError2(L"Remove updater file %s failed : %d", updaterPath.c_str(), GetLastError());
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
	//Close old dir
	CloseSourceDir();
	

	//appLogger->Log(L"SetUnhandledExceptionFilter Prevented: %d", PreventSetUnhandledExceptionFilter());
	appWorker = new TrainerWorkerInternal();
	appJyUdpAttack = new JyUdpAttack();
	appLogger->Log(L"初始化正常");

RUN_MAIN:

	if (appCrashTestMode)return JiYuTrainerUICommonEntry(4);
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
	if (appJyUdpAttack) {
		delete appJyUdpAttack;
		appJyUdpAttack = nullptr;
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
	case AppOperation3: {
		char*voidp = (char*)0x65e413f;
		*voidp = '\0';
		break;
	}
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
	/*
	if(appSetting->GetSettingStr(L"IgnoreCorrectness") == L"20190916")
		return true;

	SYSTEMTIME time;
	HANDLE hFileRead = CreateFileW(fullPath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	FILETIME file_time;
	FILETIME locationtime;

	GetFileTime(hFileRead, NULL, NULL, &file_time);//获得文件修改时间  
	FileTimeToLocalFileTime(&file_time, &locationtime);//将文件时间转换为本地文件时间  
	FileTimeToSystemTime(&locationtime, &time);

	CloseHandle(hFileRead);

	if (time.wYear > 2019 && time.wMonth > 9 && time.wDay > 16)
		return false;
	*/
	return true;
}

void JTAppInternal::MergePathString()
{
	fullDriverPath = fullDir + L"\\JiYuTrainerDriver.sys";
	fullHookerPath = fullDir + L"\\JiYuTrainerHooks.dll";
	fullSciterPath = fullDir + L"\\sciter.dll";
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
	if (FindArgInCommandLine(appArgList, appArgCount, L"install-full") != -1) appArgInstallMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"force-md5-check") != -1) appArgForceCheckFileMd5 = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"b") != -1) appArgBreak = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"break") != -1) appArgBreak = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"r1") != -1) appIsRecover = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"h") != -1) appIsHiddenMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"hidden") != -1) appIsHiddenMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"config") != -1) appIsConfigMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"bugreport") != -1) appIsBugReportMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"crash-test") != -1) appCrashTestMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"killst") != -1) appKillStMode = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"?") != -1) appCmdHelpMode = true;

	int argFIndex = FindArgInCommandLine(appArgList, appArgCount, L"f");
	if (argFIndex >= 0 && (argFIndex + 1) < appArgCount) {
		fullSourceInstallerPath = appArgList[argFIndex + 1];
		if (Path::Exists(fullSourceInstallerPath)) {
			WCHAR buffer[MAX_PATH];
			wcscpy_s(buffer, fullSourceInstallerPath.c_str());
			PathRenameExtension(buffer, L".ini");
			if (Path::Exists(fullSourceInstallerPath))
				fullIniPath = buffer;

			wcscpy_s(buffer, fullSourceInstallerPath.c_str());
			PathRemoveFileSpec(buffer);
			fullSourceInstallerDir = buffer;
		}
	}
	argFIndex = FindArgInCommandLine(appArgList, appArgCount, L"rc");
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

	appFirstUse = appSetting->GetSettingBool(L"FirstUse", true, L"JTArgeement");
	appShowAvTest = appSetting->GetSettingBool(L"ShowAvsTest", true);
	appArgeementArgeed = appSetting->GetSettingBool(L"Argeed", false, L"JTArgeement");
	appForceNoDriver = appSetting->GetSettingBool(L"DisableDriver", false);
	appForceNoSelfProtect = !appSetting->GetSettingBool(L"SelfProtect", true);
	appForceIntallInCurrentDir = appSetting->GetSettingBool(L"ForceInstallInCurrentDri", false);
}

void JTAppInternal::EnableVisualStyles() {

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
}

HFONT JTAppInternal::hFontRed = NULL;
HINSTANCE JTAppInternal::hInstance = NULL;

INT_PTR CALLBACK JTAppInternal::AVTipWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;

	switch (message)
	{
	case WM_INITDIALOG: {

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_MAIN)));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_MAIN)));
		SetDlgItemText(hDlg, IDC_MESSAGE, FormatString(L"我们检测到您的计算机上安装了 %s 杀毒软件，因为 JiYuTrainer 会对极域进行操作，可能会被杀毒软件误识别为病毒。\n因此我们建议您 关闭杀毒软件 或 添加本软件至白名单。", ((JTAppInternal*)lParam)->existsAntiVirus.c_str()).c_str());
		lResult = TRUE;
		break;
	}
	case WM_COMMAND: 
		if(IsDlgButtonChecked(hDlg, IDC_CHECK_DONOT_SHOW_AGAIN) == BST_CHECKED)
			((JTAppInternal*)currentApp)->appSetting->SetSettingBool(L"ShowAvsTest", false);
		EndDialog(hDlg, wParam); 
		lResult = wParam;  
		break;
	default: return DefWindowProc(hDlg, message, wParam, lParam);
	}
	return lResult;
}
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