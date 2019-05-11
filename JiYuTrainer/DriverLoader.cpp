#include "DriverLoader.h"
#include "DriverLoader.h"
#include "stdafx.h"
#include "DriverLoader.h"
#include "JiYuTrainer.h"
#include "KernelUtils.h"
#include "SysHlp.h"
#include "AppPublic.h"
#include <shlwapi.h>

extern JTApp * currentApp;

HANDLE hKDrv = NULL;

//删除注册表键以及子键
BOOL MREG_DeleteKey(HKEY hRootKey, LPWSTR path) {

	DWORD lastErr = SHDeleteKey(hRootKey, path);
	if (lastErr == ERROR_SUCCESS || lastErr == ERROR_FILE_NOT_FOUND)
		return TRUE;
	else
	{
		SetLastError(lastErr);
		return 0;
	}
}
BOOL MREG_ForceDeleteServiceRegkey(LPWSTR lpszDriverName)
{
	BOOL rs = FALSE;
	wchar_t regPath[MAX_PATH];
	wsprintf(regPath, L"SYSTEM\\CurrentControlSet\\services\\%s", lpszDriverName);
	rs = MREG_DeleteKey(HKEY_LOCAL_MACHINE, regPath);

	if (!rs)JTLogWarn(L"RegDeleteTree failed : %d in delete key HKEY_LOCAL_MACHINE\\%s", GetLastError(), regPath);
	else JTLogInfo(L"Service Key deleted : HKEY_LOCAL_MACHINE\\%s", regPath);

	wchar_t regName[MAX_PATH];
	wcscpy_s(regName, lpszDriverName);
	_wcsupr_s(regName);
	wsprintf(regPath, L"SYSTEM\\CurrentControlSet\\Enum\\Root\\LEGACY_%s", regName);
	rs = MREG_DeleteKey(HKEY_LOCAL_MACHINE, regPath);

	if (!rs) {
		JTLogWarn(L"RegDeleteTree failed : %d in delete key HKEY_LOCAL_MACHINE\\%s", GetLastError(), regPath);
		rs = TRUE;
	}
	else JTLogInfo(L"Service Key deleted : HKEY_LOCAL_MACHINE\\%s", regPath);

	return rs;
}
//加载驱动
//    lpszDriverName：驱动的服务名
//    driverPath：驱动的完整路径
//    lpszDisplayName：nullptr
BOOL LoadKernelDriver(const wchar_t* lpszDriverName, const wchar_t* driverPath, const wchar_t* lpszDisplayName)
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
		JTLogError(L"Load driver error in OpenSCManager : %d", GetLastError());
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
			JTLogError(L"Load driver error in CreateService : ERROR_SERVICE_MARKED_FOR_DELETE");
			if (!recreatee) {
				recreatee = true;
				if (hServiceDDK) CloseServiceHandle(hServiceDDK);
				if (hServiceMgr) CloseServiceHandle(hServiceMgr);
				if (MREG_ForceDeleteServiceRegkey(sDriverName)) goto RECREATE;
			}
		}
		if (dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS)
		{
			JTLogError(L"Load driver error in CreateService : %d", dwRtn);
			bRet = FALSE;
			goto BeforeLeave;
		}
		hServiceDDK = OpenService(hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS);
		if (hServiceDDK == NULL)
		{
			dwRtn = GetLastError();
			JTLogError(L"Load driver error in OpenService : %d", dwRtn);
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
			JTLogError(L"Load driver error in StartService : %d", dwRtn);
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
BOOL UnLoadKernelDriver(const wchar_t* szSvrName)
{
	if (hKDrv && wcscmp(szSvrName, L"JiYuKillerDriver") == 0) {
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
		JTLogError(L"UnLoad driver error in OpenSCManager : %d", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	//打开驱动所对应的服务
	hServiceDDK = OpenService(hServiceMgr, szSvrName, SERVICE_ALL_ACCESS);
	if (hServiceDDK == NULL)
	{
		if (GetLastError() == ERROR_SERVICE_DOES_NOT_EXIST)
			JTLogWarn(L"UnLoad driver error because driver not load.");
		else JTLogError(L"UnLoad driver error in OpenService : %d", GetLastError());
		bRet = FALSE;
		goto BeforeLeave;
	}
	//停止驱动程序，如果停止失败，只有重新启动才能，再动态加载。 
	if (!ControlService(hServiceDDK, SERVICE_CONTROL_STOP, &SvrSta)) {
		JTLogError(L"UnLoad driver error in ControlService : %d", GetLastError());
	}
	//动态卸载驱动程序。 
	if (!DeleteService(hServiceDDK)) {
		JTLogError(L"UnLoad driver error in DeleteService : %d", GetLastError());
		bRet = FALSE;
	}
	else bDeleted = TRUE;

BeforeLeave:
	//离开前关闭打开的句柄
	if (hServiceDDK) CloseServiceHandle(hServiceDDK);
	if (hServiceMgr) CloseServiceHandle(hServiceMgr);

	if (bDeleted) bRet = MREG_ForceDeleteServiceRegkey((LPWSTR)szSvrName);

	return bRet;
}
//打开驱动
BOOL OpenDriver()
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
		JTLogError(L"Get Kernel driver handle (CreateFile) failed : %d . ", GetLastError());
		return FALSE;
	}
	return TRUE;
}
BOOL DriverLoaded() {
	return hKDrv != NULL;
}

BOOL XinitSelfProtect()
{
	return KFInstallSelfProtect();
}
BOOL XLoadDriver() {

	bool isWin7 = SysHlp::GetSystemVersion() == SystemVersionWindows7OrLater;
	bool isXp = SysHlp::GetSystemVersion() == SystemVersionWindowsXP;

	if (!SysHlp::Is64BitOS() && (isXp || SysHlp::IsRunasAdmin()))
	{
		if (LoadKernelDriver(L"JiYuTrainerDriver", currentApp->GetPartFullPath(PART_DRIVER), NULL))
			if (OpenDriver()) {
				JTLogInfo(L"驱动加载成功");
				KFSendDriverinitParam(isXp, isWin7);
				return TRUE;
			}
			else JTLogWarn(L"驱动加载成功，但打开驱动失败");
	}
	return FALSE;
}
BOOL XCloseDriverHandle() {
	if (hKDrv) {
		CloseHandle(hKDrv);
		hKDrv = nullptr;
		return TRUE;
	}
	return FALSE;
}
BOOL XUnLoadDriver() {
	if (DriverLoaded())
		return UnLoadKernelDriver(L"JiYuTrainerDriver");
	return TRUE;
}