#include "Driver.h"
#include "IoCtl.h"
#include "IoStructs.h"
#include "UnExp.h"
#include "Protect.h"
#include "Monitor.h"

#define DEVICE_LINK_NAME L"\\??\\JKRK"
#define DEVICE_OBJECT_NAME  L"\\Device\\JKRK"

ULONG gIrpCount = 0;

strcat_ _strcat;
strcpy_ _strcpy;
memcpy_ _memcpy;
swprintf_ swprintf;
wcscpy_ _wcscpy;
wcscat_ _wcscat;
memset_ _memset;
//====================================================
//
PsResumeProcess_ _PsResumeProcess = NULL;
PsSuspendProcess_ _PsSuspendProcess = NULL;
PsLookupProcessByProcessId_ _PsLookupProcessByProcessId = NULL;
PsLookupThreadByThreadId_ _PsLookupThreadByThreadId = NULL;
ZwTerminateProcess_ _ZwTerminateProcess = NULL;
ObRegisterCallbacks_ _ObRegisterCallbacks = NULL;
ObUnRegisterCallbacks_ _ObUnRegisterCallbacks = NULL;
ObGetFilterVersion_ _ObGetFilterVersion = NULL;
PsSetCreateProcessNotifyRoutineEx_ _PsSetCreateProcessNotifyRoutineEx = NULL;
NtShutdownSystem_ _NtShutdownSystem = NULL;
extern fnZwOpenProcess _ZwOpenProcess;
extern fnZwTerminateProcess _ZwTerminateProcess;

BOOLEAN isWin7 = FALSE, isWinXP = FALSE;
ULONG systemVersion;

extern PspTerminateThreadByPointer_ PspTerminateThreadByPointer;
extern PspExitThread_ PspExitThread;

extern BOOLEAN protectInited;

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegPath)
{
	NTSTATUS ntStatus;

	UNICODE_STRING DeviceObjectName; // NT Device Name 
	UNICODE_STRING DeviceLinkName; // Win32 Name 
	PDEVICE_OBJECT deviceObject = NULL; 
	RtlInitUnicodeString(&DeviceObjectName, DEVICE_OBJECT_NAME);

	ntStatus = IoCreateDevice(
		pDriverObject,
		0, 
		&DeviceObjectName,
		FILE_DEVICE_UNKNOWN, 
		FILE_DEVICE_SECURE_OPEN, 
		FALSE,
		&deviceObject); 

	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("Couldn't create the device object\n"));
		return ntStatus;
	}

	pDriverObject->DriverUnload = (PDRIVER_UNLOAD)DriverUnload;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = IOControlDispatch;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = CreateDispatch;

	//创建驱动设备对象	
	RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
	ntStatus = IoCreateSymbolicLink(&DeviceLinkName, &DeviceObjectName);
	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("Couldn't create symbolic link\n"));
		IoDeleteDevice(deviceObject);
	}

	LoadFunctions();

#ifdef _AMD64_
	PLDR_DATA_TABLE_ENTRY64 ldr;
	ldr = (PLDR_DATA_TABLE_ENTRY64)pDriverObject->DriverSection;
#else
	PLDR_DATA_TABLE_ENTRY32 ldr;
	ldr = (PLDR_DATA_TABLE_ENTRY32)pDriverObject->DriverSection;
#endif
	ldr->Flags |= 0x20;//绕过MmVerifyCallbackFunction。

	ntStatus = KxPsMonitorInit();
	if (!NT_SUCCESS(ntStatus))
	{
		KdPrint(("Couldn't Init Ps Monitos : 0x%08X\n", ntStatus));
		ntStatus = STATUS_SUCCESS;
	}
	ntStatus = KxInitProtectProcess();
	if (!NT_SUCCESS(ntStatus)) {
		KdPrint(("Couldn't Init Protect Process : 0x%08X\n", ntStatus));
		ntStatus = STATUS_SUCCESS;
	}

	KdPrint(("DriverEntry OK!\n"));
	return ntStatus;
}
VOID DriverUnload(_In_ struct _DRIVER_OBJECT *pDriverObject)
{
	UNICODE_STRING  DeviceLinkName;
	PDEVICE_OBJECT  v1 = NULL;
	PDEVICE_OBJECT  DeleteDeviceObject = NULL;

	RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
	IoDeleteSymbolicLink(&DeviceLinkName);

	KxUnInitProtectProcess();
	KxPsMonitorUnInit();

	DeleteDeviceObject = pDriverObject->DeviceObject;
	while (DeleteDeviceObject != NULL)
	{
		v1 = DeleteDeviceObject->NextDevice;
		IoDeleteDevice(DeleteDeviceObject);
		DeleteDeviceObject = v1;
	}

	KdPrint(("DriverUnload\n"));
}

NTSTATUS IOControlDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp)
{
	NTSTATUS Status = STATUS_SUCCESS;
	ULONG_PTR Informaiton = 0;
	PVOID InputData = NULL;
	ULONG InputDataLength = 0;
	PVOID OutputData = NULL;
	ULONG OutputDataLength = 0;
	ULONG IoControlCode = 0;
	PIO_STACK_LOCATION  IoStackLocation = IoGetCurrentIrpStackLocation(Irp);  //Irp堆栈  

	IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
	InputData = Irp->AssociatedIrp.SystemBuffer;
	OutputData = Irp->AssociatedIrp.SystemBuffer;
	InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
	OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

	switch (IoControlCode)
	{
	case CTL_INITPARAM: {
		JDRV_INITPARAM* param = (JDRV_INITPARAM*)InputData;
		isWin7 = param->IsWin7;
		isWinXP = param->IsWinXP;
		systemVersion = param->systemVersion;
		LoadUnExpFunctions();
		if (isWinXP) KxHookInXP();
		Status = STATUS_SUCCESS;
		break;
	}
	case CTL_CLIENT_QUIT: {
		KxUnProtectProcessWithPid(PsGetCurrentProcessId());
		Status = STATUS_SUCCESS;
		break;
	}
	case CTL_UNINIT: {
		KxShadowSSDTUnHook();
		Status = STATUS_SUCCESS;
		break;
	}
	case CTL_INITSELFPROTECT: {
		KxShadowSSDTHook();
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		if (protectInited)
		{
			KxProtectProcessWithPid((HANDLE)pid);
			Status = STATUS_SUCCESS;
		}
		else Status = STATUS_UNSUCCESSFUL;
		break;
	}
	case CTL_OPEN_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProc;
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProc);
		if (NT_SUCCESS(Status))
		{
			HANDLE handle;
			Status = ObOpenObjectByPointer(pEProc, 0, 0, PROCESS_ALL_ACCESS, *PsProcessType, UserMode, &handle);
			if (NT_SUCCESS(Status)) {
				_memcpy(OutputData, &handle, sizeof(handle));
				Status = STATUS_SUCCESS;
				Informaiton = OutputDataLength;
			}
			else KdPrint(("ObOpenObjectByPointer err : 0x%08X", Status));
			ObDereferenceObject(pEProc);
		}
		break;
	}
	case CTL_OPEN_THREAD: {
		ULONG_PTR tid = *(ULONG_PTR*)InputData;
		PETHREAD pEThread;
		Status = PsLookupThreadByThreadId((HANDLE)tid, &pEThread);
		if (NT_SUCCESS(Status))
		{
			HANDLE handle;
			Status = ObOpenObjectByPointer(pEThread, 0, 0, THREAD_ALL_ACCESS, *PsThreadType, UserMode, &handle);
			if (NT_SUCCESS(Status)) {
				_memcpy(OutputData, &handle, sizeof(handle));
				Informaiton = OutputDataLength;
				Status = STATUS_SUCCESS;
			}
			ObDereferenceObject(pEThread);
		}
		break;
	}
	case CTL_KILL_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProcess;
		KdPrint(("CTL_KILL_PROCESS : %d\n", pid));
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProcess);
		if (NT_SUCCESS(Status))
		{
			HANDLE handle;
			Status = ObOpenObjectByPointer(pEProcess, 0, 0, PROCESS_ALL_ACCESS, *PsProcessType, UserMode, &handle);
			if (NT_SUCCESS(Status)) {
				Status = ZwTerminateProcess(handle, STATUS_SUCCESS);
				ZwClose(handle);
			}
			if(Status == STATUS_ACCESS_DENIED)
				Status = KillProcess(pEProcess);
			ObDereferenceObject(pEProcess);
		}
		Status = STATUS_SUCCESS;
		_memcpy(OutputData, &Status, sizeof(Status));
		Informaiton = OutputDataLength;
		break;
	}
	case CTL_KILL_PROCESS_SPARE_NO_EFFORT: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		Status = ZeroKill(pid);
		break;
	}
	case CTL_KILL_THREAD: {
		ULONG_PTR tid = *(ULONG_PTR*)InputData;
		if (PspTerminateThreadByPointer) {
			PETHREAD pEThread;
			Status = PsLookupThreadByThreadId((HANDLE)tid, &pEThread);
			if (NT_SUCCESS(Status))
			{
				Status = PspTerminateThreadByPointer(pEThread, 0, TRUE);
				ObDereferenceObject(pEThread);
			}
		}
		else Status = STATUS_NOT_SUPPORTED;
		break;
	}
	case CTL_SUSPEND_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProc;
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProc);
		if (NT_SUCCESS(Status))
		{
			Status = _PsSuspendProcess(pEProc);
			ObDereferenceObject(pEProc);
		}
		_memcpy(OutputData, &Status, sizeof(Status));
		Informaiton = OutputDataLength;
		break;
	}
	case CTL_RESUME_PROCESS: {
		ULONG_PTR pid = *(ULONG_PTR*)InputData;
		PEPROCESS pEProc;
		Status = PsLookupProcessByProcessId((HANDLE)pid, &pEProc);
		if (NT_SUCCESS(Status))
		{
			Status = _PsResumeProcess(pEProc);
			ObDereferenceObject(pEProc);
		}
		_memcpy(OutputData, &Status, sizeof(Status));
		Informaiton = OutputDataLength;
		break;
	}
	case CTL_SHUTDOWN: {
		KdPrint(("CompuleShutdown\n"));
		CompuleShutdown();
		//
		break;
	}
	case CTL_REBOOT: {
		KdPrint(("CompuleReBoot\n"));
		CompuleReBoot();
		//
		break;
	}
	default: break;
	}

	Irp->IoStatus.Status = Status; //Ring3 GetLastError();
	Irp->IoStatus.Information = Informaiton;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);  //将Irp返回给Io管理器
	return Status; //Ring3 DeviceIoControl()返回值
}
NTSTATUS CreateDispatch(IN PDEVICE_OBJECT DeviceObject,IN PIRP Irp)
{
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}

VOID LoadFunctions()
{
	UNICODE_STRING MemsetName;
	UNICODE_STRING MemcpysName;
	UNICODE_STRING StrcpysName;
	UNICODE_STRING StrcatsName;
	UNICODE_STRING SWprintfsName;
	UNICODE_STRING WCscatsName;
	UNICODE_STRING WCscpysName;
	UNICODE_STRING PsResumeProcessName;
	UNICODE_STRING PsSuspendProcessName;
	UNICODE_STRING PsLookupProcessByProcessIdName;
	UNICODE_STRING PsLookupThreadByThreadIdName;
	UNICODE_STRING ZwTerminateProcessName;
	UNICODE_STRING ObRegisterCallbacksName;
	UNICODE_STRING ObUnRegisterCallbacksName;
	UNICODE_STRING ObGetFilterVersionName;
	UNICODE_STRING PsSetCreateProcessNotifyRoutineExName;
	UNICODE_STRING NtShutdownSystemName;
	UNICODE_STRING ZwOpenProcessName;

	RtlInitUnicodeString(&ZwOpenProcessName, L"ZwOpenProcess");
	RtlInitUnicodeString(&NtShutdownSystemName, L"NtShutdownSystem");
	RtlInitUnicodeString(&PsSetCreateProcessNotifyRoutineExName, L"PsSetCreateProcessNotifyRoutineEx");
	RtlInitUnicodeString(&ObGetFilterVersionName, L"ObGetFilterVersion");
	RtlInitUnicodeString(&ObUnRegisterCallbacksName, L"ObUnRegisterCallbacks");
	RtlInitUnicodeString(&ObRegisterCallbacksName, L"ObRegisterCallbacks");
	RtlInitUnicodeString(&ZwTerminateProcessName, L"ZwTerminateProces");
	RtlInitUnicodeString(&PsLookupProcessByProcessIdName, L"PsLookupProcessByProcessId");
	RtlInitUnicodeString(&PsLookupThreadByThreadIdName, L"PsLookupThreadByThreadId");
	RtlInitUnicodeString(&PsResumeProcessName, L"PsResumeProcess");
	RtlInitUnicodeString(&PsSuspendProcessName, L"PsSuspendProcess");
	RtlInitUnicodeString(&MemsetName, L"memset");
	RtlInitUnicodeString(&WCscatsName, L"wcscat");
	RtlInitUnicodeString(&WCscpysName, L"wcscpy");
	RtlInitUnicodeString(&SWprintfsName, L"swprintf");
	RtlInitUnicodeString(&MemcpysName, L"memcpy");
	RtlInitUnicodeString(&StrcpysName, L"strcpy");
	RtlInitUnicodeString(&StrcatsName, L"strcat");

	_ZwTerminateProcess = (fnZwTerminateProcess)MmGetSystemRoutineAddress(&ZwTerminateProcessName);
	_ZwOpenProcess = (fnZwOpenProcess)MmGetSystemRoutineAddress(&ZwOpenProcessName);
	_NtShutdownSystem = (NtShutdownSystem_)MmGetSystemRoutineAddress(&NtShutdownSystemName);
	_PsLookupProcessByProcessId = (PsLookupProcessByProcessId_)MmGetSystemRoutineAddress(&PsLookupProcessByProcessIdName);
	_PsLookupThreadByThreadId = (PsLookupThreadByThreadId_)MmGetSystemRoutineAddress(&PsLookupThreadByThreadIdName);
	_PsResumeProcess = (PsResumeProcess_)MmGetSystemRoutineAddress(&PsResumeProcessName);
	_PsSuspendProcess = (PsSuspendProcess_)MmGetSystemRoutineAddress(&PsSuspendProcessName);
	_memset = (memset_)MmGetSystemRoutineAddress(&MemsetName);
	_wcscpy = (wcscpy_)MmGetSystemRoutineAddress(&WCscpysName);
	_wcscat = (wcscat_)MmGetSystemRoutineAddress(&WCscatsName);
	_memcpy = (memcpy_)MmGetSystemRoutineAddress(&MemcpysName);
	_strcat = (strcat_)MmGetSystemRoutineAddress(&StrcatsName);
	_strcpy = (strcpy_)MmGetSystemRoutineAddress(&StrcpysName);
	swprintf = (swprintf_)MmGetSystemRoutineAddress(&SWprintfsName);
	_ZwTerminateProcess = (ZwTerminateProcess_)MmGetSystemRoutineAddress(&ZwTerminateProcessName);
	_ObRegisterCallbacks = (ObRegisterCallbacks_)MmGetSystemRoutineAddress(&ObRegisterCallbacksName);
	_ObUnRegisterCallbacks = (ObUnRegisterCallbacks_)MmGetSystemRoutineAddress(&ObUnRegisterCallbacksName);
	_ObGetFilterVersion = (ObGetFilterVersion_)MmGetSystemRoutineAddress(&ObGetFilterVersionName);
	_PsSetCreateProcessNotifyRoutineEx = (PsSetCreateProcessNotifyRoutineEx_)MmGetSystemRoutineAddress(&PsSetCreateProcessNotifyRoutineExName);

	KdPrint(("PsLookupProcessByProcessId : 0x%08X\n", _PsLookupProcessByProcessId));
	KdPrint(("PsLookupThreadByThreadId : 0x%08X\n", _PsLookupThreadByThreadId));
}
VOID LoadUnExpFunctions()
{
	if(isWin7)
		PspTerminateThreadByPointer = (PspTerminateThreadByPointer_)KxGetPspTerminateThreadByPointerAddressX_7Or8Or10(0x50);
	else if(isWinXP)
		PspTerminateThreadByPointer = (PspTerminateThreadByPointer_)KxGetPspTerminateThreadByPointerAddressX_7Or8Or10(0x50);

	if (isWin7)
		PspExitThread = (PspExitThread_)KxGetPspExitThread_32_64();
	//else if (isWinXP)
	//	PspExitThread = (PspExitThread_)KxGetPspExitThread_32_64();

	DbgPrint("PspTerminateThreadByPointer : 0x%08x\n", PspTerminateThreadByPointer);
	DbgPrint("PspExitThread : 0x%08x\n", PspExitThread);
}

NTSTATUS ZeroKill(ULONG_PTR PID)   //X32  X64
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	int i = 0;
	PVOID handle;
	PEPROCESS Eprocess;
	ntStatus = PsLookupProcessByProcessId((HANDLE)PID, &Eprocess);
	if (NT_SUCCESS(ntStatus))
	{
		PKAPC_STATE pKs = (PKAPC_STATE)ExAllocatePool(NonPagedPool, sizeof(PKAPC_STATE));
		KeStackAttachProcess(Eprocess, pKs);//Attach进程虚拟空间
		for (i = 0; i <= 0x7fffffff; i += 0x1000)
		{
			//__try {
				if (MmIsAddressValid((PVOID)i))
				{
					ProbeForWrite((PVOID)i, 0x1000, sizeof(ULONG));
					memset((PVOID)i, 0xcc, 0x1000);
				}
				else {
					if (i > 0x1000000)  //填这么多足够破坏进程数据了  
						break;
				}
			/*}
			__except (1) {
				KdPrint(("---错误!---"));
			}*/
		}
		KeUnstackDetachProcess(pKs); 
		ntStatus = ObOpenObjectByPointer((PVOID)Eprocess, 0, NULL, 0, NULL, KernelMode, &handle);
		if (ntStatus != STATUS_SUCCESS)
			return ntStatus;
		ZwTerminateProcess((HANDLE)handle, STATUS_SUCCESS);
		ZwClose((HANDLE)handle);
		return STATUS_SUCCESS;
	}
	return ntStatus;
}
NTSTATUS KillProcess(PEPROCESS pEProcess)
{
	NTSTATUS status = STATUS_UNSUCCESSFUL;
	if (!PspTerminateThreadByPointer) return STATUS_NOT_SUPPORTED;

	if(_PsSuspendProcess) _PsSuspendProcess(pEProcess);
	for (UINT32 i = 8; i < 65536; i += 4)
	{
		//__try {
			PETHREAD pEThread;
			status = PsLookupThreadByThreadId((HANDLE)i, &pEThread);
			if (NT_SUCCESS(status))
			{
				if (IoThreadToProcess(pEThread) == pEProcess) {
					if (PspExitThread != NULL) status = KxTerminateThreadApc(pEThread); 
					else status = PspTerminateThreadByPointer(pEThread, 0, TRUE);
				}
					
				ObDereferenceObject(pEThread);
			}
		/*}
		__except (1) {
			KdPrint(("---错误!---"));
		}*/
	}
	if (_PsResumeProcess) _PsResumeProcess(pEProcess);
	status = STATUS_SUCCESS;
	return status;
}

//APC 结束进程回调
VOID  KernelKillThreadRoutine(IN PKAPC Apc, IN OUT PKNORMAL_ROUTINE *NormalRoutine, IN OUT PVOID *NormalContext, IN OUT PVOID *SystemArgument1, IN OUT PVOID *SystemArgument2)
{
	PspExitThread(STATUS_SUCCESS);
}

//使用插入apc方法强制结束线程
//  PETHREAD Thread：需要结束的线程
NTSTATUS KxTerminateThreadApc(PETHREAD Thread)
{
	NTSTATUS Status = STATUS_SUCCESS;
	PKAPC ExitApc = NULL;
	//ULONG    OldMask;

	if (!Thread) return STATUS_INVALID_PARAMETER;

	if (Thread == PsGetCurrentThread()) {
		//if (ETHREAD_CrossThreadFlags_Offest != 0)
		//	PS_SET_BITS((ULONG_PTR*)((ULONG_PTR)Thread + ETHREAD_CrossThreadFlags_Offest), PS_CROSS_THREAD_FLAGS_TERMINATED);
		PspExitThread(0);
	}
	else {
		if (PsIsThreadTerminating(Thread))
			return STATUS_THREAD_IS_TERMINATING;
		if (PsIsSystemThread(Thread))
			return STATUS_ACCESS_DENIED;

		if (PspExitThread == NULL)
			//使用 PspTerminateThreadByPointer 结束
			return PspTerminateThreadByPointer(Thread, 0, FALSE);

		Status = STATUS_SUCCESS;

		ExitApc = (PKAPC)ExAllocatePoolWithTag(NonPagedPool, sizeof(KAPC), 0);
		if (ExitApc == NULL)
		{
			KdPrint(("[KillProcessWithApc] malloc memory failed \n"));
			return STATUS_UNSUCCESSFUL;
		}

		//if (ETHREAD_CrossThreadFlags_Offest != 0)
		//	OldMask = PS_TEST_SET_BITS((ULONG_PTR*)((ULONG_PTR)Thread + ETHREAD_CrossThreadFlags_Offest), PS_CROSS_THREAD_FLAGS_TERMINATED);

		if (!PsIsThreadTerminating(Thread)) {
			//为线程初始化APC
			KeInitializeApc(ExitApc, Thread, OriginalApcEnvironment, KernelKillThreadRoutine, NULL, NULL, KernelMode, NULL);
			if (!KeInsertQueueApc(ExitApc, ExitApc, NULL, 2))
			{
				ExFreePool(ExitApc);
				Status = STATUS_UNSUCCESSFUL;
			}
			else
			{
				/*
				if (KeForceResumeThread != 0 && ETHREAD_Tcb_Offest != 0)
				{
					//WRK 抄来的，强制恢复暂停的线程，保证线程可以运行插入的apc
					KxForceResumeThread(Thread);
				}
				*/
			}
	    }
		else {
			ExFreePool(ExitApc);
		}
	}

	return Status;
}


VOID CompuleShutdown(void)
{
	_NtShutdownSystem(ShutdownPowerOff);

	typedef void(__fastcall *FCRB)(void);
	/*
	mov ax,2001h
	mov dx,1004h
	out dx,ax
	ret
	*/
	FCRB fcrb = NULL;
	UCHAR shellcode[12] = "\x66\xB8\x01\x20\x66\xBA\x04\x10\x66\xEF\xC3";
	fcrb = ExAllocatePool(NonPagedPool, 11);
	memcpy(fcrb, shellcode, 11);
	fcrb();
}
VOID CompuleReBoot(void)
{
	typedef void(__fastcall*FCRB)(void);

	/*
	mov al,0FEH
	out 64h,al
	ret
	*/
	FCRB fcrb = NULL;
	UCHAR shellcode[] = "\xB0\xFE\xE6\x64\xC3";
	fcrb = (FCRB)ExAllocatePool(NonPagedPool, 7);
	memcpy(fcrb, shellcode, 7);
	fcrb();

	return;
}