#pragma once
#include "Driver.h"

#define PROCESS_TERMINATE         0x0001  
#define PROCESS_VM_OPERATION      0x0008  
#define PROCESS_VM_READ           0x0010  
#define PROCESS_VM_WRITE          0x0020 

typedef struct tag_PROTECT_PROC_STORAGE
{
	HANDLE ProcessId;
	void * Next;
}PROTECT_PROC_STORAGE, *PPROTECT_PROC_STORAGE;

BOOLEAN KxIsProcessProtect(HANDLE pid);

VOID KxProtectProcessWithPid(HANDLE pid);
VOID KxUnProtectProcessWithPid(HANDLE pid);

NTSTATUS KxInitProtectProcess();
VOID KxUnInitProtectProcess();

OB_PREOP_CALLBACK_STATUS KxObPreCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation);

VOID KxHookInXP();
VOID KxUnHookInXP();

NTSTATUS InstallSysServiceHook(ULONG oldService, ULONG newService);
NTSTATUS UnInstallSysServiceHook(ULONG oldService);

VOID BackupSysServicesTable();

VOID KxShadowSSDTHook();
VOID KxShadowSSDTUnHook();

VOID DisableWriteProtect(ULONG oldAttr);
VOID EnableWriteProtect(PULONG pOldAttr);

typedef NTSTATUS(NTAPI *fnZwOpenProcess)(
	_Out_ PHANDLE ProcessHandle,
	_In_ ACCESS_MASK DesiredAccess,
	_In_ POBJECT_ATTRIBUTES ObjectAttributes,
	_In_opt_ PCLIENT_ID ClientId);
typedef NTSTATUS(NTAPI*fnZwTerminateProcess)(
	__in_opt HANDLE ProcessHandle,
	__in NTSTATUS ExitStatus
	);