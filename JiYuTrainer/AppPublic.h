#pragma once

#include "Logger.h"
#include "SettingHlp.h"
#include "TrainerWorker.h" 
#include "JyUdpAttack.h" 

#define CURRENT_VERSION "1.7.6.0515" 

#define FAST_STR_BINDER(str, fstr, size, ...) WCHAR str[size]; swprintf_s(str, fstr, __VA_ARGS__)

#define PART_INI -1
#define PART_MAIN 0
#define PART_LOG 1
#define PART_HOOKER 3
#define PART_DRIVER 4
#define PART_SCITER 5


enum EXTRACT_RES {
	ExtractUnknow,
	ExtractCreateFileError,
	ExtractWriteFileError,
	ExtractReadResError,
	ExtractSuccess
};
enum AppOperation {
	AppOperation1,
	AppOperation2,
	AppOperation3,
	AppOperationKShutdown,
	AppOperationKReboot,
	AppOperationUnLoadDriver,
	AppOperationForceLoadDriver,
};

class JTApp
{
public:

	/*
		卸载
	*/
	virtual void UnInstall() {}

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

	virtual LPCWSTR MakeFromSourceArg(LPCWSTR arg) { return nullptr; }



	virtual void LoadDriver() {}

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
	virtual int Run(int nCmdShow) { return 0; }
	virtual int GetResult() { return 0; }
	virtual void Exit(int code) {  }

	virtual LPVOID RunOperation(AppOperation op) { return nullptr; }

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
	
	virtual int GetAppShowCmd() { return 0; };
	virtual bool GetAppIsHiddenMode() { return false; };

	virtual LPCWSTR GetStartupErr() { return nullptr; }

	virtual JyUdpAttack* GetJyUdpAttack() { return nullptr; };
	virtual Logger* GetLogger() { return nullptr; };
	virtual SettingHlp* GetSettings() { return nullptr; };
	virtual bool GetSelfProtect() { return false; }
	virtual TrainerWorker* GetTrainerWorker() { return nullptr; };
	virtual void*GetSciterAPI() { return nullptr; }
};