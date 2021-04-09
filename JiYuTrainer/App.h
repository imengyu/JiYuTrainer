#pragma once
#include "stdafx.h"
#ifdef JTEXPORT
#include "resource.h"
#include <string>
#include "AppPublic.h"
#include "SysHlp.h"
#include "MD5Utils.h"
#include "MemoryModule.h"

extern "C" int JiYuTrainerUICommonEntry(int i);

#endif

#define APP_TITLE L"JiYuTrainer"
#define APP_FAIL_SYSTEM_NOT_SUPPORT -2
#define APP_FAIL_PIRACY_VERSION -3
#define APP_FAIL_INSTALL -6
#define APP_FAIL_ALEDAY_RUN -5

typedef INT_PTR(WINAPI *fnDialogBoxParamW)(HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam);
extern "C" int WINAPI MessageBoxTimeoutW(IN HWND hWnd, IN LPCWSTR lpText, IN LPCWSTR lpCaption, IN UINT uType, IN WORD wLanguageId, IN DWORD dwMilliseconds);

#ifdef JTEXPORT

class JTAppInternal : public JTApp
{
public:

	JTAppInternal(HINSTANCE hInstance);
	~JTAppInternal();

	int CheckAndInstall();
	void UnInstall();

	BOOL InstallSciter();

	EXTRACT_RES InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo);

	LPWSTR *GetCommandLineArray() { return appArgList; };
	int GetCommandLineArraySize() { return appArgCount; };

	bool IsCommandExists(LPCWSTR cmd);
	int FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg);

	LPCWSTR MakeFromSourceArg(LPCWSTR arg);

	void CloseSourceDir();
	bool CheckAntiVirusSoftware(bool showTip);

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
	JyUdpAttack* GetJyUdpAttack() { return appJyUdpAttack; };
	void*GetSciterAPI() {	return pSciterAPI;}

	LPVOID RunOperation(AppOperation op);

private:

	std::wstring fullPath;
	std::wstring fullDir;
	std::wstring fullSourceInstallerPath;
	std::wstring fullSourceInstallerDir;
	std::wstring fullIniPath;
	std::wstring fullLogPath;
	std::wstring fullArgBuffer;

	std::wstring fullDriverPath;
	std::wstring fullHookerPath;
	std::wstring fullSciterPath;

	std::wstring existsAntiVirus;

	enum AppStartType {
		AppStartTypeNormal,
		AppStartTypeInTemp,
		AppStartTypeUpdater,
		AppStartTypeConfig,
		AppStartTypeBugReport,
	};

	int appResult = 0;
	static HINSTANCE hInstance;
	int appStartType = AppStartTypeNormal;
	std::wstring appStartErr;
	HMEMORYMODULE pMemSciterdll = NULL;
	PVOID pSciterAPI = NULL;
	fnDialogBoxParamW _DialogBoxParamW = NULL;

	bool appFirstUse = false;
	bool appShowAvTest = false;
	bool appArgeementArgeed = false;
	bool appForceNoSelfProtect = false;
	bool appArgInstallMode = false;
	bool appForceNoDriver = false;
	bool appForceIntallInCurrentDir = false;
	bool appArgForceCheckFileMd5 = false;
	bool appArgRemoveUpdater = false;
	bool appArgBreak = false;
	bool appNeedInstallIniTemple = false;
	bool appIsInstaller = false;
	bool appIsRecover = false;
	bool appIsConfigMode = false;
	bool appIsHiddenMode = false;
	bool appIsBugReportMode = false;
	bool appCrashTestMode = false;
	bool appCmdHelpMode = false;
	bool appKillStMode = false;

	int appShowCmd = 0;
	std::wstring updaterPath;

	LPWSTR *appArgList = nullptr;
	int appArgCount = 0;

	Logger *appLogger = nullptr;
	SettingHlp *appSetting = nullptr;
	TrainerWorker *appWorker = nullptr;
	JyUdpAttack* appJyUdpAttack = nullptr;
	MD5Utils * appMd5Utils = nullptr;

	void MergePathString();
	void InitPath();
	void InitCommandLine();
	void InitArgs();
	void InitLogger();
	void InitPrivileges();
	void InitSettings();

	void EnableVisualStyles();

	bool CheckAppCorrectness();
	int RunCheckRunningApp();
	bool RunArgeementDialog();

	int RunInternal();
	bool ExitInternal();
	void ExitClear();

	static HFONT hFontRed;

	void copyStrFromIntArr(wchar_t*buffer, int *arr, size_t len);

	static INT_PTR CALLBACK AVTipWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK ArgeementWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};


#endif