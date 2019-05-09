#include "Monitor.h"
#include "Protect.h"

extern ULONG_PTR CurrentPCMgrProcess;
extern ULONG_PTR CurrentDbgViewProcess;

extern PsSetCreateProcessNotifyRoutineEx_ _PsSetCreateProcessNotifyRoutineEx;

BOOLEAN KxPsMonitorInited = FALSE;

NTSTATUS KxPsMonitorInit() 
{
	if (!KxPsMonitorInited) {
		if (!_PsSetCreateProcessNotifyRoutineEx)
			return STATUS_NOT_SUPPORTED;
		KxPsMonitorInited = TRUE;
		NTSTATUS status = _PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)KxCreateProcessNotifyEx, FALSE);
		return status;
	}
	return STATUS_SUCCESS;
}
VOID KxPsMonitorUnInit() {
	if(KxPsMonitorInited)
		_PsSetCreateProcessNotifyRoutineEx((PCREATE_PROCESS_NOTIFY_ROUTINE_EX)KxCreateProcessNotifyEx, TRUE);
}
VOID KxCreateProcessNotifyEx(__inout PEPROCESS Process,__in HANDLE ProcessId,__in_opt PPS_CREATE_NOTIFY_INFO CreateInfo)
{
	if (CreateInfo != NULL)	//进程创建事件
	{
		//KdPrint(("进程创建 %ld", ProcessId));
	}
	else
	{
		//KdPrint(("进程退出 %ld", ProcessId));
		if (KxIsProcessProtect(ProcessId))
			KxUnProtectProcessWithPid(ProcessId);
	}
}
