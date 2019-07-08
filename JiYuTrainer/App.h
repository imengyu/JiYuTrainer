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
	void*GetSciterAPI() {	return pSciterAPI;}

	LPVOID RunOperation(AppOperation op);
private:

	std::wstring fullPath;
	std::wstring fullDir;
	std::wstring fullSourceInstallerPath;
	std::wstring fullIniPath;
	std::wstring fullLogPath;
	std::wstring fullArgBuffer;

	std::wstring fullDriverPath;
	std::wstring fullHookerPath;

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

	int appShowCmd = 0;
	std::wstring updaterPath;

	LPWSTR *appArgList = nullptr;
	int appArgCount = 0;

	Logger *appLogger = nullptr;
	SettingHlp *appSetting = nullptr;
	TrainerWorker *appWorker = nullptr;

	void MergePathString();
	void InitPath();
	void InitCommandLine();
	void InitArgs();
	void InitLogger();
	void InitPrivileges();
	void InitSettings();

	int RunCheckRunningApp();
	bool RunArgeementDialog();

	int RunInternal();
	bool ExitInternal();
	void ExitClear();

	static HFONT hFontRed;

	static INT_PTR CALLBACK ArgeementWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

	static LPTOP_LEVEL_EXCEPTION_FILTER __stdcall MyDummySetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter);
	static BOOL PreventSetUnhandledExceptionFilter();
	static LONG WINAPI UnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
	static LONG GenerateMiniDump(PEXCEPTION_POINTERS pExInfo);
	static BOOL GenerateCrashInfo(PEXCEPTION_POINTERS pExInfo, LPCWSTR info_file_name, LPCWSTR file_name, SYSTEMTIME tm, LPCWSTR);
};


#endif