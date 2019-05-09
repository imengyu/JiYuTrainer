#pragma once
#include "Driver.h"

NTSTATUS KxPsMonitorInit();

VOID KxPsMonitorUnInit();

VOID KxLmageNotifyRoutine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo);

VOID KxCreateProcessNotifyEx(PEPROCESS Process, HANDLE ProcessId, PPS_CREATE_NOTIFY_INFO CreateInfo);

VOID KxCreateThreadNotify(IN HANDLE ProcessId, IN HANDLE ThreadId, IN BOOLEAN Create);

NTSTATUS KxRefuseCreateThread(IN HANDLE ProcessId, IN HANDLE ThreadId);
