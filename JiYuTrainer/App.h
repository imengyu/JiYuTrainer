#pragma once
#include "stdafx.h"
#ifdef JTEXPORT
#include "resource.h"
#include <string>
#include "AppPublic.h"
#include "PartsMD5.h"
#include "SysHlp.h"
#include "MD5Utils.h"
#endif


#define APP_TITLE L"JiYuTrainer"
#define APP_FAIL_SYSTEM_NOT_SUPPORT -2
#define APP_FAIL_MAIN_PART_BROKED -3
#define APP_FAIL_MAIN_PART_LOADERR -4
#define APP_FAIL_ALEDAY_RUN -5

#ifdef JTEXPORT

class JTAppInternal : public JTApp
{
public:

	JTAppInternal(HINSTANCE hInstance);
	~JTAppInternal();

	int CheckInstall(APP_INSTALL_MODE mode);
	int CheckMd5();
	void UnInstall();

	EXTRACT_RES InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo);

	LPWSTR *GetCommandLineArray() { return appArgList; };
	int GetCommandLineArraySize() { return appArgCount; };

	bool IsCommandExists(LPCWSTR cmd);
	int FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg);

	LPCWSTR MakeFromSourceArg(LPCWSTR arg);

	int Run(int nCmdShow);
	int GetResult() { return appResult; }
	void Exit(int code);

	void LoadDriver();

	HINSTANCE GetInstance() {
		return hInstance;
	}

	int GetAppShowCmd() { return appShowCmd; };
	bool GetAppIsHiddenMode() { return appIsHiddenMode; };

	LPCWSTR GetPartFullPath(int partId);
	LPCWSTR GetFullPath() { return fullPath.c_str(); }
	LPCWSTR GetCurrentDir() { return fullDir.c_str(); }
	LPCWSTR GetSourceInstallerPath() { return fullSourceInstallerPath.c_str(); }
	LPCWSTR GetStartupErr() { return appStartErr.c_str(); }

	Logger* GetLogger() { return appLogger; };
	SettingHlp* GetSettings() { return appSetting; };
	bool GetSelfProtect() { return !appForceNoSelfProtect; }
	TrainerWorker* GetTrainerWorker() { return appWorker; };
	void* GetUtils(int utilsId);

	LPVOID RunOperation(AppOperation op);
private:

	std::wstring fullPath;
	std::wstring fullDir;
	std::wstring fullSourceInstallerPath;
	std::wstring fullIniPath;
	std::wstring fullArgBuffer;

	std::wstring parts[PART_COUNT] = {
		std::wstring(L"JiYuTrainer.exe"),
		std::wstring(L"JiYuTrainer.bat"),
		std::wstring(L"JiYuTrainerUInstall.bat"),
		std::wstring(L"JiYuTrainerUI.dll"),
		std::wstring(L"JiYuTrainerHooks.dll"), 
		std::wstring(L"JiYuTrainerDriver.sys"),
		std::wstring(L"JiYuTrainerUpdater.dll"),
		std::wstring(L"sciter.dll"),
	};
	int partsResId[PART_COUNT] = {
		0,
		0,
		0,
		IDR_DLL_UI,
		IDR_DLL_HOOKS,
		IDR_DLL_DRIVER,
		IDR_DLL_UPDATER,
		IDR_DLL_SCITER,
	};
	LPCWSTR partsMd5Checks[PART_COUNT] = {
		L"",
		L"",
		L"",
		PART_MD5_UI,
		PART_MD5_HOOKS,
		PART_MD5_DRIVER,
		PART_MD5_UPDATER,
		PART_MD5_SCITER
	};
	LPCWSTR partsMd5CheckNames[PART_COUNT] = {
		L"",
		L"",
		L"",
		L"PART_MD5_UI",
		L"PART_MD5_HOOKS",
		L"PART_MD5_DRIVER",
		L"PART_MD5_UPDATER",
		L"PART_MD5_SCITER"
	};
	LPVOID utilsPointer[16];

	enum AppStartType {
		AppStartTypeNormal,
		AppStartTypeInTemp,
		AppStartTypeUpdater,
		AppStartTypeConfig,
	};

	int appResult = 0;
	static HINSTANCE hInstance;
	int appStartType = AppStartTypeNormal;
	std::wstring appStartErr;

	bool appArgeementArgeed = false;
	bool appForceNoSelfProtect = false;
	bool appForceInstallInCurrentDir = false;
	bool appForceNoDriver = false;
	bool appArgForceNoInstall = false;
	bool appArgForceCheckFileMd5 = false;
	bool appArgForceTemp = false;
	bool appArgInstallMode = false;
	bool appArgRemoveUpdater = false;
	bool appArgBreak = false;
	bool appNeedInstallIniTemple = false;
	bool appIsRecover = false;
	bool appIsMd5CalcMode = false;
	bool appIsConfigMode = false;
	bool appIsHiddenMode = false;

	int appShowCmd = 0;
	std::wstring updaterPath;

	LPWSTR *appArgList = nullptr;
	int appArgCount = 0;

	Logger *appLogger = nullptr;
	SettingHlp *appSetting = nullptr;
	TrainerWorker *appWorker = nullptr;
	SysHlp *appSysHlp = nullptr;
	MD5Utils* appMD5Utils = nullptr;

	void MergePathString(LPCWSTR path);
	void InitPath();
	void InitCommandLine();
	void InitArgs();
	void InitLogger();
	void InitPrivileges();
	void InitSettings();
	void InitUtils();

	int RunCheckRunningApp();
	bool RunArgeementDialog();
	bool RunMd5ShowDialog();

	int RunInternal();
	bool ExitInternal();
	void ExitClear();

	static HFONT hFontRed;

	static INT_PTR CALLBACK ArgeementWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK Md5ShowWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
	static LONG GenerateMiniDump(PEXCEPTION_POINTERS pExInfo);
};

#endif