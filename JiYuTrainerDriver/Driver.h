#pragma once
#include <ntifs.h>

NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject, IN PUNICODE_STRING pRegPath);
VOID DriverUnload(_In_ struct _DRIVER_OBJECT *DriverObject);

NTSTATUS IOControlDispatch(IN PDEVICE_OBJECT pDeviceObject, IN PIRP Irp);
NTSTATUS CreateDispatch(IN PDEVICE_OBJECT DeviceObject, IN PIRP Irp);

VOID LoadFunctions();
VOID LoadUnExpFunctions();

NTSTATUS ZeroKill(ULONG_PTR PID);
NTSTATUS KillProcess(PEPROCESS pEThread);

VOID CompuleReBoot(void);
NTSTATUS KxTerminateThreadApc(PETHREAD Thread);
VOID CompuleShutdown(void);

//Defs

typedef struct _LDR_DATA_TABLE_ENTRY64
{
	LIST_ENTRY64    InLoadOrderLinks;
	LIST_ENTRY64    InMemoryOrderLinks;
	LIST_ENTRY64    InInitializationOrderLinks;
	PVOID            DllBase;
	PVOID            EntryPoint;
	ULONG            SizeOfImage;
	UNICODE_STRING    FullDllName;
	UNICODE_STRING     BaseDllName;
	ULONG            Flags;
	USHORT            LoadCount;
	USHORT            TlsIndex;
	PVOID            SectionPointer;
	ULONG            CheckSum;
	PVOID            LoadedImports;
	PVOID            EntryPointActivationContext;
	PVOID            PatchInformation;
	LIST_ENTRY64    ForwarderLinks;
	LIST_ENTRY64    ServiceTagLinks;
	LIST_ENTRY64    StaticLinks;
	PVOID            ContextInformation;
	ULONG64            OriginalBase;
	LARGE_INTEGER    LoadTime;
} LDR_DATA_TABLE_ENTRY64, *PLDR_DATA_TABLE_ENTRY64;

typedef struct _LDR_DATA_TABLE_ENTRY32 {
	LIST_ENTRY32 InLoadOrderLinks;
	LIST_ENTRY32 InMemoryOrderLinks;
	LIST_ENTRY32 InInitializationOrderLinks;
	ULONG DllBase;
	ULONG EntryPoint;
	ULONG SizeOfImage;
	UNICODE_STRING32 FullDllName;
	UNICODE_STRING32 BaseDllName;
	ULONG Flags;
	USHORT LoadCount;
	USHORT TlsIndex;
	LIST_ENTRY32 HashLinks;
	ULONG SectionPointer;
	ULONG CheckSum;
	ULONG TimeDateStamp;
	ULONG LoadedImports;
	ULONG EntryPointActivationContext;
	ULONG PatchInformation;
	LIST_ENTRY32 ForwarderLinks;
	LIST_ENTRY32 ServiceTagLinks;
	LIST_ENTRY32 StaticLinks;
	ULONG ContextInformation;
	ULONG OriginalBase;
	LARGE_INTEGER LoadTime;
} LDR_DATA_TABLE_ENTRY32, *PLDR_DATA_TABLE_ENTRY32;

typedef enum _SHUTDOWN_ACTION {
	ShutdownNoReboot,
	ShutdownReboot,
	ShutdownPowerOff
} SHUTDOWN_ACTION, *PSHUTDOWN_ACTION;

typedef enum _KAPC_ENVIRONMENT
{
	OriginalApcEnvironment,
	AttachedApcEnvironment,
	CurrentApcEnvironment,
	InsertApcEnvironment
} KAPC_ENVIRONMENT;

#define NUMBER_SERVICE_TABLES 4

typedef struct _KSERVICE_TABLE_DESCRIPTOR {
	PULONG_PTR Base;
	PULONG Count;
	ULONG Limit;
	PUCHAR Number;
} KSERVICE_TABLE_DESCRIPTOR, *PKSERVICE_TABLE_DESCRIPTOR;


extern __declspec(dllimport) KSERVICE_TABLE_DESCRIPTOR KeServiceDescriptorTable[NUMBER_SERVICE_TABLES];


//根据 ZwServiceFunction 获取 ZwServiceFunction 在 SSDT 中所对应的服务的索引号 
#define SYSCALL_INDEX(ServiceFunction) (*(PULONG)((PUCHAR)ServiceFunction + 1))

//根据ZwServiceFunction 来获得服务在 SSDT 中的索引号，然后再通过该索引号来获取ntServiceFunction的地址 
#define SYSCALL_FUNCTION(ServiceFunction) KeServiceDescriptorTable->ntoskrnl.ServiceTableBase[SYSCALL_INDEX(ServiceFunction)]

typedef VOID(*PKNORMAL_ROUTINE) (IN PVOID NormalContext, IN PVOID SystemArgument1, IN PVOID SystemArgument2);
typedef VOID(*PKKERNEL_ROUTINE) (IN struct _KAPC *Apc, IN OUT PKNORMAL_ROUTINE *NormalRoutine, IN OUT PVOID *NormalContext, IN OUT PVOID *SystemArgument1, IN OUT PVOID *SystemArgument2);
typedef VOID(*PKRUNDOWN_ROUTINE) (IN struct _KAPC *Apc);

typedef void* (__cdecl *memset_)(void*  _Dst, int _Val, size_t _Size);
typedef int(_stdcall *memcpy_)(void *Dst, const void *Src, size_t MaxCount);
typedef int(_stdcall *strcpy_)(char* _Destination, char const* _Source);
typedef int(_stdcall *strcat_)(char* _Destination, char const* _Source);
typedef int(_stdcall *swprintf_)(wchar_t* _Buffer, wchar_t const* _Format, ...);
typedef int(_stdcall *wcscpy_)(wchar_t* _Destination, _In_z_ wchar_t const* _Source);
typedef int(_stdcall *wcscat_)(wchar_t* _Destination, _In_z_ wchar_t const* _Source);

typedef NTSTATUS(_stdcall *PspTerminateThreadByPointer_)(IN PETHREAD Thread, IN NTSTATUS ExitStatus, IN BOOLEAN DirectTerminate);
typedef NTSTATUS(_stdcall *PspExitThread_)(IN NTSTATUS ExitStatus);
typedef NTSTATUS(_stdcall *PsResumeProcess_)(PEPROCESS Process);
typedef NTSTATUS(_stdcall *PsSuspendProcess_)(PEPROCESS Process);
typedef NTSTATUS(_stdcall *PsLookupProcessByProcessId_)(HANDLE ProcessId, PEPROCESS *Process);
typedef NTSTATUS(_stdcall *PsLookupThreadByThreadId_)(HANDLE ThreadId, PETHREAD *Thread);
typedef PETHREAD(_stdcall *PsGetNextProcessThread_)(IN PEPROCESS Process, IN PETHREAD Thread);
typedef NTSTATUS(_stdcall *PsTerminateProcess_)(PEPROCESS Process, NTSTATUS ExitStatus);
typedef PEPROCESS(_stdcall *PsGetNextProcess_)(PEPROCESS Process);
typedef ULONG(_stdcall *KeForceResumeThread_)(__inout PKTHREAD Thread);
typedef NTSTATUS(_stdcall *ZwTerminateProcess_)(_In_opt_ HANDLE ProcessHandle,_In_ NTSTATUS ExitStatus);
typedef NTSTATUS(_stdcall *ObRegisterCallbacks_)(_In_ POB_CALLBACK_REGISTRATION CallbackRegistration,	_Outptr_ PVOID *RegistrationHandle);
typedef VOID(_stdcall *ObUnRegisterCallbacks_)(	_In_ PVOID RegistrationHandle);
typedef USHORT(_stdcall *ObGetFilterVersion_)();
typedef NTSTATUS(_stdcall *PsSetCreateProcessNotifyRoutineEx_)(
	_In_ PCREATE_PROCESS_NOTIFY_ROUTINE_EX NotifyRoutine,
	_In_ BOOLEAN Remove
);
typedef NTSTATUS (NTAPI *NtShutdownSystem_)(IN SHUTDOWN_ACTION Action);

extern VOID NTAPI KeInitializeApc(__in PKAPC Apc, __in PKTHREAD Thread, __in KAPC_ENVIRONMENT   TargetEnvironment, __in PKKERNEL_ROUTINE KernelRoutine, __in_opt PKRUNDOWN_ROUTINE RundownRoutine, __in PKNORMAL_ROUTINE NormalRoutine, __in KPROCESSOR_MODE Mode, __in PVOID Context);
extern BOOLEAN NTAPI KeInsertQueueApc(IN PKAPC Apc, IN PVOID SystemArgument1, IN PVOID SystemArgument2, IN KPRIORITY PriorityBoost);
extern NTSTATUS NTAPI ZwQueryInformationProcess(
	_In_      HANDLE           ProcessHandle,
	_In_      PROCESSINFOCLASS ProcessInformationClass,
	_Out_     PVOID            ProcessInformation,
	_In_      ULONG            ProcessInformationLength,
	_Out_opt_ PULONG           ReturnLength
);





#define SYS_BULID_VERSION_XP 2600
#define SYS_BULID_VERSION_VISTA 6000

#define SYS_BULID_VERSION_7 7600
#define SYS_BULID_VERSION_7_SP1 7601



