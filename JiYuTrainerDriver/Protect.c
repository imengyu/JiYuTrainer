#include "protect.h"

PPROTECT_PROC_STORAGE protectProcsStart = NULL;
PPROTECT_PROC_STORAGE protectProcsEnd = NULL;
PVOID obHandle = NULL;
BOOLEAN protectInited = FALSE;

extern ObRegisterCallbacks_ _ObRegisterCallbacks;
extern ObUnRegisterCallbacks_ _ObUnRegisterCallbacks;
extern ObGetFilterVersion_ _ObGetFilterVersion;
extern memset_ _memset;

//存放保护进程的一个链表
VOID KxInitProtectProcessList()
{
	protectProcsStart = ExAllocatePool(NonPagedPool, sizeof(PROTECT_PROC_STORAGE));
	protectProcsStart->Next = NULL;
	protectProcsStart->ProcessId = 0;
	protectProcsEnd = protectProcsStart;
}
VOID KxDestroyProtectProcessList()
{
	PPROTECT_PROC_STORAGE ptr = protectProcsStart;
	if (ptr->Next != NULL) {
		do {
			PPROTECT_PROC_STORAGE ptr_next = ptr->Next;
			ExFreePool(ptr);
			ptr = ptr_next;
		} while (ptr != NULL);
	}
	else ExFreePool(ptr);
}
BOOLEAN KxIsProcessProtect(HANDLE pid)
{
	//检测pid是否被保护
	if (pid == 0)return FALSE;
	PPROTECT_PROC_STORAGE ptr = protectProcsStart;
	if (ptr->Next != NULL) {
		do {
			if (ptr->ProcessId == pid)
				return TRUE;
			ptr = ptr->Next;
		} while (ptr != NULL);
	}
	return FALSE;
}
VOID KxProtectProcessWithPid(HANDLE pid)
{
	if (pid == 0) return;
	if (!KxIsProcessProtect(pid))
	{
		if (protectProcsEnd)
		{
			//添加到链表末尾
			protectProcsEnd->Next = ExAllocatePool(NonPagedPool, sizeof(PROTECT_PROC_STORAGE));
			_memset(protectProcsEnd->Next, 0, sizeof(PROTECT_PROC_STORAGE));
			((PPROTECT_PROC_STORAGE)protectProcsEnd->Next)->ProcessId = pid;
			protectProcsEnd = protectProcsEnd->Next;

			KdPrint(("Protect Process : %d", pid));
		}
	}
	else KdPrint(("Protect Process : %d alreday protected", pid));
}
VOID KxUnProtectProcessWithPid(HANDLE pid)
{
	if (pid == 0)return;
	PPROTECT_PROC_STORAGE ptr = protectProcsStart;
	PPROTECT_PROC_STORAGE ptr_for = protectProcsStart;
	if (ptr->Next != NULL) {
		do {
			//链表删除项目
			if (ptr->ProcessId == pid) {
				if (ptr_for) {
					if (ptr->Next == NULL) {
						ptr_for->Next = NULL;
						protectProcsEnd = ptr_for;
					}
					else ptr_for->Next = ptr->Next;
				}
				KdPrint(("UnProtect Process : %d", pid));
				ExFreePool(ptr);
				return;
			}
			else {
				ptr_for = ptr;
				ptr = ptr->Next;
			}
		} while (ptr != NULL);
	}
}

//初始化和退出
NTSTATUS KxInitProtectProcess()
{
	if (!protectInited) {

		if (!_ObRegisterCallbacks || !_ObGetFilterVersion)
			return STATUS_NOT_SUPPORTED;

		KxInitProtectProcessList();

		protectInited = TRUE;

		OB_CALLBACK_REGISTRATION obReg;
		OB_OPERATION_REGISTRATION opReg;

		memset(&obReg, 0, sizeof(obReg));
		obReg.Version = _ObGetFilterVersion();
		obReg.OperationRegistrationCount = 1;
		obReg.RegistrationContext = NULL;
		RtlInitUnicodeString(&obReg.Altitude, L"321000");

		memset(&opReg, 0, sizeof(opReg)); //初始化				  
		opReg.ObjectType = PsProcessType;//进程
		opReg.Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
		opReg.PreOperation = (POB_PRE_OPERATION_CALLBACK)&KxObPreCall; //在这里注册一个回调函数指针
		obReg.OperationRegistration = &opReg; //注意这一条语句
		return _ObRegisterCallbacks(&obReg, &obHandle); //在这里注册回调函数
	}
	return TRUE;
}
VOID KxUnInitProtectProcess()
{
	if(obHandle) _ObUnRegisterCallbacks(obHandle); //取消注册回调
	if (protectInited) {
		KxDestroyProtectProcessList();
		protectInited = FALSE;
	}
}

//回调函数
OB_PREOP_CALLBACK_STATUS KxObPreCall(PVOID RegistrationContext, POB_PRE_OPERATION_INFORMATION pOperationInformation)
{
	if (pOperationInformation->KernelHandle || pOperationInformation->ObjectType != *PsProcessType)
		return OB_PREOP_SUCCESS;
	HANDLE pid = PsGetProcessId((PEPROCESS)pOperationInformation->Object);
	UNREFERENCED_PARAMETER(RegistrationContext);
	if (KxIsProcessProtect(pid))
	{
		if (pOperationInformation->Operation == OB_OPERATION_HANDLE_CREATE || pOperationInformation->Operation == OB_OPERATION_HANDLE_DUPLICATE)
		{
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_TERMINATE) == PROCESS_TERMINATE)
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_TERMINATE;
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_OPERATION) == PROCESS_VM_OPERATION)
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_OPERATION;
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_READ) == PROCESS_VM_READ)
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_READ;
			if ((pOperationInformation->Parameters->CreateHandleInformation.OriginalDesiredAccess & PROCESS_VM_WRITE) == PROCESS_VM_WRITE)
				pOperationInformation->Parameters->CreateHandleInformation.DesiredAccess &= ~PROCESS_VM_WRITE;
		}
	}
	return OB_PREOP_SUCCESS;
}