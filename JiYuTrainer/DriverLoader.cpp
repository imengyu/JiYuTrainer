#include "DriverLoader.h"
#include "stdafx.h"
#include "DriverLoader.h"
#include "JiYuTrainer.h"
#include "KernelUtils.h"
#include "SysHlp.h"
#include "AppPublic.h"
#include "RegHlp.h"
#include "Logger.h"

extern JTApp * currentApp;
extern LoggerInternal * currentLogger;

HANDLE hKDrv = NULL;

//加载驱动
//    lpszDriverName：驱动的服务名
//    driverPath：驱动的完整路径
//    lpszDisplayName：nullptr
BOOL MLoadKernelDriver(const wchar_t* lpszDriverName, const wchar_t* driverPath, const wchar_t* lpszDisplayName)
{
	//MessageBox(0, driverPath, L"driverPath", 0);
	wchar_t sDriverName[32];
	wcscpy_s(sDriverName, lpszDriverName);

	bool recreatee = false;

RECREATE:
	DWORD dwRtn = 0;
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		currentLogger->LogError2(L"Load driver error in OpenSCManager : %d", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}

	hServiceDDK = CreateService(hServiceMgr, lpszDriverName, lpszDisplayName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
		SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, driverPath, NULL, NULL, NULL, NULL, NULL);


	if (hServiceDDK == NULL)
	{
		dwRtn = GetLastError();
		if (dwRtn == ERROR_SERVICE_MARKED_FOR_DELETE)
		{
			currentLogger->LogError2(L"Load driver error in CreateService : ERROR_SERVICE_MARKED_FOR_DELETE");
			if (!recreatee) {
				recreatee = true;
				if (hServiceDDK) CloseServiceHandle(hServiceDDK);
				if (hServiceMgr) CloseServiceHandle(hServiceMgr);
				if (MRegForceDeleteServiceRegkey(sDriverName)) goto RECREATE;
			}
		}
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			currentLogger->LogError2(L"Load driver error in CreateService : %d", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			dwRtn = GetLastError();
			currentLogger->LogError2(L"Load driver error in OpenService : %d", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
	}
	bRet = StartService(hServiceDDK, NULL, NULL);
	if (!bRet)
	{
		DWORD dwRtn = GetLastError();
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING)
		{
			currentLogger->LogError2(L"Load driver error in StartService : %d", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else
		{
			if (dwRtn == ERROR_IO_PENDING)
			{
				bRet = FALSE;
				goto BeforeLeave;
			}
			else
			{
				bRet = TRUE;
				goto BeforeLeave;
			}
		}
	}
	bRet = TRUE;
	//离开前关闭句柄
BeforeLeave:
	if (hServiceDDK) CloseServiceHandle(hServiceDDK);
	if (hServiceMgr) CloseServiceHandle(hServiceMgr);
	return bRet;
}
//卸载驱动
//    szSvrName：服务名
BOOL MUnLoadKernelDriver(const wchar_t* szSvrName)
{
	if (hKDrv && (wcscmp(szSvrName, L"JiYuTrainerDriver") == 0)) {
		CloseHandle(hKDrv);
		hKDrv = NULL;
	}

	BOOL bDeleted = FALSE;
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;
	SERVICE_STATUS SvrSta;
	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		currentLogger->LogError2(L"UnLoad driver error in OpenSCManager : %d", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	//打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);
	if (hServiceDDK == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
			currentLogger->LogWarn(L"UnLoad driver error because driver not load.");
		else currentLogger->LogError2(L"UnLoad driver error in OpenService : %d", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta)) {
		currentLogger->LogError2(L"UnLoad driver error in ControlService : %d", GetLastError());
	}
	//动态卸载驱动程序。 
	if (!DeleteService(hServiceDDK)) {
		currentLogger->LogError2(L"UnLoad driver error in DeleteService : %d", GetLastError());
		bRet = FALSE;
	}
	else bDeleted = TRUE;

BeforeLeave:
	//离开前关闭打开的句柄
	if (hServiceDDK) CloseServiceHandle(hServiceDDK);
	if (hServiceMgr) CloseServiceHandle(hServiceMgr);

	if (bDeleted) bRet = MRegForceDeleteServiceRegkey((LPWSTR)szSvrName);

	return bRet;
}
BOOL MUnLoadDriverServiceWithMessage(const wchar_t* szSvrName)
{
	BOOL bDeleted = FALSE;
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr = NULL;
	SC_HANDLE hServiceDDK = NULL;
	SERVICE_STATUS SvrSta;
	DWORD lastErr = 0;

	hServiceMgr = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (hServiceMgr == NULL)
	{
		lastErr = GetLastError();
		FAST_STR_BINDER(str, L"卸载驱动错误，打开驱动管理错误：%s\n请尝试以管理员身份运行软件。", 128, SysHlp::ConvertErrorCodeToString(lastErr));
		MessageBox(NULL, str, L"JiYuTrainer - 错误", MB_ICONERROR);
		bRet = FALSE;
		goto BeforeLeave;
	}
	//打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);
	if (hServiceDDK == NULL)
	{
		lastErr = GetLastError();
		if (lastErr == ERROR_SERVICE_DOES_NOT_EXIST) 
			MessageBox(NULL, L"驱动已卸载并删除，请不要重复操作", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION);
		else if ( lastErr == ERROR_SERVICE_MARKED_FOR_DELETE) 
			MessageBox(NULL, L"没有在这台计算机上找到找到驱动，可能是驱动已经被卸载了", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION);
		else {
			FAST_STR_BINDER(str, L"卸载驱动错误，打开驱动错误：%s", 128, SysHlp::ConvertErrorCodeToString(lastErr));
			MessageBox(NULL, str, L"JiYuTrainer - 错误", MB_ICONERROR);
		}
		bRet = FALSE;
		goto BeforeLeave;
	}
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta)) {
		lastErr = GetLastError();
		if (lastErr == ERROR_SERVICE_MARKED_FOR_DELETE) {
			MessageBox(NULL, L"驱动已卸载并删除，请不要重复操作", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION);
			bRet = FALSE;
			goto BeforeLeave;
		}
		else {
			FAST_STR_BINDER(str, L"卸载驱动错误，停止驱动失败：%s", 128, SysHlp::ConvertErrorCodeToString(lastErr));
			MessageBox(NULL, str, L"JiYuTrainer - 错误", MB_ICONERROR);
		}
	}
	//动态卸载驱动程序。 
	if (!DeleteService(hServiceDDK)) {
		lastErr = GetLastError();
		if (lastErr == ERROR_SERVICE_MARKED_FOR_DELETE) 
			MessageBox(NULL, L"驱动已卸载并删除，请不要重复操作", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION);
		else {
			FAST_STR_BINDER(str, L"卸载驱动错误，删除驱动错误：%s", 128, SysHlp::ConvertErrorCodeToString(lastErr));
			MessageBox(NULL, str, L"JiYuTrainer - 错误", MB_ICONERROR);
			bRet = FALSE;
		}
	}
	else bDeleted = TRUE;
BeforeLeave:
	//离开前关闭打开的句柄
	if (hServiceDDK) CloseServiceHandle(hServiceDDK);
	if (hServiceMgr) CloseServiceHandle(hServiceMgr);

	if (bDeleted) bRet = MRegForceDeleteServiceRegkey((LPWSTR)szSvrName);

	return bRet;
}
//打开驱动
BOOL XOpenDriver()
{
	hKDrv = CreateFile(L"\\\\.\\JKRK",
		GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (!hKDrv || hKDrv == INVALID_HANDLE_VALUE)
	{
		currentLogger->LogError(L"Get Kernel driver handle (CreateFile) failed : %d . ", GetLastError());
		return FALSE;
	}
	return TRUE;
}
BOOL XDriverLoaded() {
	return hKDrv != NULL;
}

BOOL XTestDriverCanUse() {

	bool isWin7 = SysHlp::GetSystemVersion() == SystemVersionWindows7OrLater;
	bool isXp = SysHlp::GetSystemVersion() == SystemVersionWindowsXP;

	if (SysHlp::Is64BitOS()) {
		currentLogger->LogWarn(L"驱动不支持 64位 系统");
		return FALSE;
	}
	if (!SysHlp::IsRunasAdmin() && !isXp) {
		currentLogger->LogWarn(L"要加载驱动，请以管理员身份运行本程序");
		return FALSE;
	}

	return TRUE;
}
BOOL XInitSelfProtect()
{
	return KFInstallSelfProtect();
}
BOOL XLoadDriver() {

	if(!XTestDriverCanUse()) return FALSE;
	if (MLoadKernelDriver(L"JiYuTrainerDriver", currentApp->GetPartFullPath(PART_DRIVER), NULL))
	{
		if (XOpenDriver()) {

			bool isWin7 = SysHlp::GetSystemVersion() == SystemVersionWindows7OrLater;
			bool isXp = SysHlp::GetSystemVersion() == SystemVersionWindowsXP;
			ULONG sysBulidVersion = SysHlp::GetWindowsBulidVersion();

			currentLogger->Log(L"Windows Bulid version %d", sysBulidVersion);
			KFSendDriverinitParam(isXp, isWin7, sysBulidVersion);
			
			currentLogger->LogInfo(L"驱动加载成功");
			return TRUE;
		}
		else currentLogger->LogWarn2(L"驱动加载成功，但打开驱动失败");
	}

	return FALSE;
}
BOOL XCloseDriverHandle() {
	if (hKDrv) {
		KFUnInstallSelfProtect();
		CloseHandle(hKDrv);
		hKDrv = nullptr;
		return TRUE;
	}
	return FALSE;
}
BOOL XUnLoadDriver()
{
	if (XDriverLoaded()) 
	{
		KFBeforeUnInitDriver();
		return MUnLoadKernelDriver(L"JiYuTrainerDriver");
	}
	return TRUE;
}