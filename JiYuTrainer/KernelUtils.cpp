#include "stdafx.h"
#include "JiYuTrainer.h"
#include "DriverLoader.h"
#include "KernelUtils.h"
#include "Logger.h"
#include "NtHlp.h"
#include <winioctl.h>
#include "../JiYuTrainerDriver/IoCtl.h"
#include "../JiYuTrainerDriver/IoStructs.h"

extern HANDLE hKDrv;
extern LoggerInternal * currentLogger;

BOOL KFShutdown()
{
	if (DriverLoaded())
	{
		DWORD ReturnLength = 0;
		BOOL rs = DeviceIoControl(hKDrv, CTL_SHUTDOWN, NULL, 0, NULL, 0, &ReturnLength, NULL);
		currentLogger->LogInfo(L"DeviceIoControl CTL_SHUTDOWN %s", rs ? L"TRUE" : L"FALSE");
		if (!rs) currentLogger->LogError(L"DeviceIoControl CTL_SHUTDOWN %d", GetLastError());

		return rs;
	}
	else currentLogger->LogWarn(L"Çý¶¯Î´¼ÓÔØ£¡");
	return false;
}
BOOL KFReboot()
{
	if (DriverLoaded())
	{
		DWORD ReturnLength = 0;
		BOOL rs = DeviceIoControl(hKDrv, CTL_REBOOT, NULL, 0, NULL, 0, &ReturnLength, NULL);
		currentLogger->LogInfo(L"DeviceIoControl CTL_REBOOT %s", rs ? L"TRUE" : L"FALSE");
		if(!rs) currentLogger->LogError(L"DeviceIoControl CTL_REBOOT %d",GetLastError());

		return rs;
	}
	else currentLogger->LogWarn(L"Çý¶¯Î´¼ÓÔØ£¡");
	return false;
}
BOOL KForceKill(DWORD pid, NTSTATUS *pStatus)
{
	if (DriverLoaded())
	{
		NTSTATUS status = STATUS_UNSUCCESSFUL;
		DWORD ReturnLength = 0;
		ULONG_PTR pidb = pid;
		if (DeviceIoControl(hKDrv, CTL_KILL_PROCESS, &pidb, sizeof(ULONG_PTR), &status, sizeof(status), &ReturnLength, NULL))
		{
			if (status == STATUS_SUCCESS)
				return TRUE;
			else currentLogger->LogError(L"CTL_KILL_PROCESS ´íÎó£º0x08%X", status);
		}
		else currentLogger->LogError(L"DeviceIoControl CTL_KILL_PROCESS ´íÎó£º%d", GetLastError());
		if (pStatus)*pStatus = status;
	}
	else currentLogger->LogWarn(L"Çý¶¯Î´¼ÓÔØ£¡");
	return false;
}
BOOL KFSendDriverinitParam(bool isXp, bool isWin7) {
	if (DriverLoaded())
	{
		DWORD ReturnLength = 0;

		JDRV_INITPARAM pidb = { 0 };
		pidb.IsWin7 = isWin7;
		pidb.IsWinXP = isXp;
		if (DeviceIoControl(hKDrv, CTL_INITPARAM, &pidb, sizeof(pidb), NULL, 0, &ReturnLength, NULL))
			return TRUE;
		else currentLogger->LogError(L"DeviceIoControl CTL_INITPARAM ´íÎó£º%d", GetLastError());
	}
	return false;
}
BOOL KFInstallSelfProtect()
{
	if (DriverLoaded())
	{
		DWORD ReturnLength = 0;
		ULONG_PTR pidb = GetCurrentProcessId();
		if (DeviceIoControl(hKDrv, CTL_INITSELFPROTECT, &pidb, sizeof(pidb), NULL, NULL, &ReturnLength, NULL))
				return TRUE;
		else currentLogger->LogError(L"DeviceIoControl CTL_INITSELFPROTECT ´íÎó£º%d", GetLastError());
	}
	return false;
}
BOOL KFInjectDll(DWORD pid, LPWSTR dllPath) {
	if (DriverLoaded())
	{

	}
	else currentLogger->LogWarn(L"Çý¶¯Î´¼ÓÔØ£¡");
	return false;
}

