#pragma once
#include "stdafx.h"
#include <string>
#include "Logger.h"
#include "SettingHlp.h"
#include "TrainerWorker.h"

#define PART_INI -1
#define PART_HOOKER 2
#define PART_DRIVER 3
#define PART_COUNT 6

#define FAST_STR_BINDER(str, fstr, size, ...) WCHAR str[size]; swprintf_s(str, fstr, __VA_ARGS__)

#define APP_TITLE L"JiYuTrainer"

enum EXTRACT_RES {
	ExtractUnknow,
	ExtractCreateFileError,
	ExtractWriteFileError,
	ExtractReadResError,
	ExtractSuccess
};

class JTApp
{
public:

	/*
		检查程序完整性并开始安装
	*/
	virtual int CheckInstall() { return 0; }
	/*
		检查指定路径是否是USB设备路径
	*/
	virtual bool CheckIsPortabilityDevice(LPCWSTR path) { return false; }

	/*
		释放模块资源到文件
		[resModule] 资源所在模块
		[resId] 资源id
		[resType] 资源类型
		[extractTo] 文件路径
	*/
	virtual EXTRACT_RES InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo) { return EXTRACT_RES::ExtractUnknow; }

	//检测命令行参数是否存在某个参数
	virtual bool IsCommandExists(LPCWSTR cmd) { return false; }

	//获取命令行参数数组
	virtual LPWSTR *GetCommandLineArray() { return nullptr; }
	//获取命令行参数数组大小
	virtual int GetCommandLineArraySize() { return 0; }
	/*
		查找命令行参数在数组中的位置
		[szArgList] 命令行参数数组
		[argCount] 命令行参数数组大小
		[arg] 要查找命令行参数
		[返回] 如果找到，返回索引，否则返回-1
	*/
	virtual int FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg) { return 0; }

	/*
		运行程序
	*/
	virtual int Run() { return 0; }
	virtual int GetResult() { return 0; }
	virtual void Exit(int code) {  }

	/*
		获取当前程序 HINSTANCE
	*/
	virtual HINSTANCE GetInstance() { return nullptr; }

	/*
		获取部件完整位置
		[partId] 部件索引
	*/
	virtual LPCWSTR GetPartFullPath(int partId) { return nullptr; }

	//获取当前程序完整路径
	virtual LPCWSTR GetFullPath() { return nullptr; }
	//获取当前程序目录
	virtual LPCWSTR GetCurrentDir() { return nullptr; }
	virtual LPCWSTR GetSourceInstallerPath() { return nullptr; }

	virtual Logger* GetLogger() { return nullptr; };
	virtual SettingHlp* GetSettings() { return nullptr; };
	virtual bool GetSelfProtect() { return false; }
	virtual TrainerWorker* GetTrainerWorker() { return nullptr; };
};
class JTAppInternal : public JTApp
{
public:


	JTAppInternal(HINSTANCE hInstance);
	~JTAppInternal();

	/*
		检查程序完整性并开始安装
	*/
	int CheckInstall();
	/*
		检查指定路径是否是USB设备路径
	*/
	bool CheckIsPortabilityDevice(LPCWSTR path);

	/*
		释放模块资源到文件
		[resModule] 资源所在模块
		[resId] 资源id
		[resType] 资源类型
		[extractTo] 文件路径
	*/
	EXTRACT_RES InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo);

	//获取命令行参数数组
	LPWSTR *GetCommandLineArray() { return appArgList; };
	//获取命令行参数数组大小
	int GetCommandLineArraySize() { return appArgCount; };

	bool IsCommandExists(LPCWSTR cmd);

	int FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg);

	/*
		运行程序
	*/
	int Run();
	int GetResult() { return appResult; }
	void Exit(int code);

	/*
		获取当前程序 HINSTANCE
	*/
	HINSTANCE GetInstance() {
		return hInstance;
	}

	/*
		获取部件完整位置
		[partId] 部件索引
	*/
	LPCWSTR GetPartFullPath(int partId);

	//获取当前程序完整路径
	LPCWSTR GetFullPath() { return fullPath.c_str(); }
	//获取当前程序目录
	LPCWSTR GetCurrentDir() { return fullDir.c_str(); }
	LPCWSTR GetSourceInstallerPath() { return fullSourceInstallerPath.c_str(); }

	Logger* GetLogger() { return appLogger; };
	SettingHlp* GetSettings() { return appSetting; };
	bool GetSelfProtect() { return appForceNoSelfProtect; }
	TrainerWorker* GetTrainerWorker() { return appWorker; };

private:

	std::wstring fullPath;
	std::wstring fullDir;
	std::wstring fullSourceInstallerPath;
	std::wstring fullIniPath;

	std::wstring parts[PART_COUNT] = {
		std::wstring(L"JiYuTrainer.exe"),
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
		0,
		0,
		0,
	};

	enum AppStartType {
		AppStartTypeNormal,
		AppStartTypeInTemp,
		AppStartTypeUpdater,
	};

	int appResult = 0;
	static HINSTANCE hInstance;
	int appStartType = AppStartTypeNormal;

	bool appArgeementArgeed = false;
	bool appForceNoSelfProtect = false;
	bool appForceNoDriver = false;
	bool appArgForceNoInstall = false;
	bool appArgForceTemp = false;

	LPWSTR *appArgList = nullptr;
	int appArgCount = 0;

	Logger *appLogger = nullptr;
	SettingHlp *appSetting = nullptr;
	TrainerWorker *appWorker = nullptr;

	void InitPath();
	void InitCommandLine();
	void InitArgs();
	void InitLogger();
	void InitPrivileges();
	void InitSettings();
	void InitPP();

	int RunCheckRunningApp();
	bool RunArgeeMentDialog();

	int RunInternal();
	bool ExitInternal();
	void ExitClear();

	static HFONT hFontRed;

	static INT_PTR CALLBACK ArgeementWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
};


