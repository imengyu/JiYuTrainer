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
