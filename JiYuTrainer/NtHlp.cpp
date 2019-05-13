#include "stdafx.h"
#include "nthlp.h"
#include "StringHlp.h"
#include "JiYuTrainer.h"
#include "Logger.h"
#include <ShellAPI.h>

extern LoggerInternal * currentLogger;

NtSuspendThreadFun NtSuspendThread;
NtResumeThreadFun NtResumeThread;
NtTerminateThreadFun NtTerminateThread;
NtTerminateProcessFun NtTerminateProcess;
NtOpenThreadFun NtOpenThread;
NtSuspendProcessFun NtSuspendProcess;
NtResumeProcessFun NtResumeProcess;
NtOpenProcessFun NtOpenProcess;

NtQueryInformationThreadFun NtQueryInformationThread;
NtQuerySystemInformationFun NtQuerySystemInformation;
NtQueryInformationProcessFun NtQueryInformationProcess;
NtCloseFun NtClose;

HMODULE hNtdll = NULL;
HICON HIconDef = NULL;

void MLoadNt()
{
	hNtdll = GetModuleHandle(L"ntdll.dll");
	NtSuspendThread = (NtSuspendThreadFun)GetProcAddress(hNtdll, "NtSuspendThread");
	NtResumeThread = (NtResumeThreadFun)GetProcAddress(hNtdll, "NtResumeThread");
	NtTerminateThread = (NtTerminateThreadFun)GetProcAddress(hNtdll, "NtTerminateThread");
	NtTerminateProcess = (NtTerminateProcessFun)GetProcAddress(hNtdll, "NtTerminateProcess");
	NtOpenThread = (NtOpenThreadFun)GetProcAddress(hNtdll, "NtOpenThread");
	NtSuspendProcess = (NtSuspendProcessFun)GetProcAddress(hNtdll, "NtSuspendProcess");
	NtResumeProcess = (NtResumeProcessFun)GetProcAddress(hNtdll, "NtResumeProcess");
	NtOpenProcess = (NtOpenProcessFun)GetProcAddress(hNtdll, "NtOpenProcess");
	NtQueryInformationThread = (NtQueryInformationThreadFun)GetProcAddress(hNtdll, "NtQueryInformationThread");
	NtQuerySystemInformation = (NtQuerySystemInformationFun)GetProcAddress(hNtdll, "NtQuerySystemInformation");
	NtQueryInformationProcess = (NtQueryInformationProcessFun)GetProcAddress(hNtdll, "NtQueryInformationProcess");
	NtClose = (NtCloseFun)GetProcAddress(hNtdll, "NtClose");
}

NTSTATUS MQueryProcessVariableSize(_In_ HANDLE ProcessHandle, _In_ PROCESSINFOCLASS ProcessInformationClass, _Out_ PVOID *Buffer)
{
	NTSTATUS status;
	PVOID buffer;
	ULONG returnLength = 0;

	status = NtQueryInformationProcess(
		ProcessHandle,
		ProcessInformationClass,
		NULL,
		0,
		&returnLength
	);

	if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL && status != STATUS_INFO_LENGTH_MISMATCH)
		return status;

	buffer = malloc(returnLength);
	status = NtQueryInformationProcess(
		ProcessHandle,
		ProcessInformationClass,
		buffer,
		returnLength,
		&returnLength
	);

	if (NT_SUCCESS(status))
	{
		*Buffer = buffer;
	}
	else
	{
		free(buffer);
	}

	return status;
}
BOOL MGetProcessFullPathEx(HANDLE hProcess, LPWSTR outPath)
{
	TCHAR szImagePath[MAX_PATH];
	if (!hProcess) return FALSE;
	if (!NT_SUCCESS(MGetProcessImageFileNameWin32(hProcess, szImagePath, MAX_PATH)))
		return FALSE;
	wcscpy_s(outPath, 260, szImagePath);
	return TRUE;
}
NTSTATUS MGetProcessImageFileNameWin32(HANDLE ProcessHandle, LPWSTR FileNameBuffer, ULONG FileNameBufferSize)
{
	NTSTATUS status;
	PUNICODE_STRING fileName;

	status = MQueryProcessVariableSize(ProcessHandle, ProcessImageFileNameWin32, (PVOID*)&fileName);

	if (!NT_SUCCESS(status))
		return status;

	wcscpy_s(FileNameBuffer, FileNameBufferSize, fileName->Buffer);

	free(fileName);

	return status;
}
NTSTATUS MSuspendProcessNt(DWORD dwPId, HANDLE handle)
{
	if (dwPId != 0 && dwPId != 4 && dwPId > 0) {
		HANDLE hProcess;
		NTSTATUS rs = MOpenProcessNt(dwPId, &hProcess);
		if (rs == STATUS_SUCCESS) {
			if (hProcess) {
				rs = NtSuspendProcess(hProcess);
				NtClose(hProcess);
				if (rs == STATUS_SUCCESS)
					return STATUS_SUCCESS;
				else {
					currentLogger->LogError(L"SuspendProcess failed (PID : %d) NTSTATUS : 0x%08X", dwPId, rs);
					return rs;
				}
			}
		}
		else currentLogger->LogError(L"SuspendProcess failed in OpenProcess : (PID : %d) NTSTATUS : 0x%08X", dwPId, rs);
		return rs;
	}
	else if (handle)
	{
		NTSTATUS  rs = NtSuspendProcess(handle);
		if (rs == 0) return STATUS_SUCCESS;
		else {
			currentLogger->LogError(L"SuspendProcess failed NTSTATUS : 0x%08X", rs);
			return rs;
		}
	}
	return STATUS_UNSUCCESSFUL;
}
NTSTATUS MResumeProcessNt(DWORD dwPId, HANDLE handle)
{
	if (dwPId != 0 && dwPId != 4 && dwPId > 0) {
		HANDLE hProcess;
		NTSTATUS rs = MOpenProcessNt(dwPId, &hProcess);
		if (rs == STATUS_SUCCESS) {
			if (hProcess)
			{
				rs = NtResumeProcess(hProcess);
				NtClose(hProcess);
				if (rs == STATUS_SUCCESS) return STATUS_SUCCESS;
				else {
					currentLogger->LogError(L"RusemeProcess failed (PID : %d) NTSTATUS : 0x%08X", dwPId, rs);
					return rs;
				}
			}
		}
		else currentLogger->LogError(L"RusemeProcess failed in OpenProcess : (PID : %d) NTSTATUS : 0x%08X", dwPId, rs);
		return rs;
	}
	else if (handle)
	{
		NTSTATUS rs = NtResumeProcess(handle);
		if (rs == 0)return STATUS_SUCCESS;
		else
		{
			currentLogger->LogError(L"RusemeProcess failed NTSTATUS : 0x%08X", rs);
			return rs;
		}
	}
	return STATUS_UNSUCCESSFUL;
}
NTSTATUS MOpenProcessNt(DWORD dwId, PHANDLE pLandle)
{
	HANDLE hProcess;
	OBJECT_ATTRIBUTES ObjectAttributes;
	_CLIENT_ID ClientId;

	ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
	ObjectAttributes.RootDirectory = NULL;
	ObjectAttributes.ObjectName = NULL;
	ObjectAttributes.Attributes = NULL;
	ObjectAttributes.SecurityDescriptor = NULL;
	ObjectAttributes.SecurityQualityOfService = NULL;

	ClientId.UniqueThread = 0;
	ClientId.UniqueProcess = (HANDLE)(long long)dwId;

	NTSTATUS NtStatus = NtOpenProcess(
		&hProcess,
		PROCESS_ALL_ACCESS,
		&ObjectAttributes,
		&ClientId);

	if (NtStatus == STATUS_SUCCESS) {
		*pLandle = hProcess;
		return STATUS_SUCCESS;
	}
	else return NtStatus;
}
NTSTATUS MTerminateProcessNt(DWORD dwId, HANDLE handle)
{
	if (handle) {
		NTSTATUS rs = NtTerminateProcess(handle, 0);
		if (rs == 0) return STATUS_SUCCESS;
		else {
			currentLogger->LogError(L"TerminateProcess failed NTSTATUS : 0x%08X", rs);
			return rs;
		}
	}
	else
	{
		if (dwId != 0 && dwId != 4 && dwId > 0) {
			HANDLE hProcess;
			NTSTATUS rs = MOpenProcessNt(dwId, &hProcess);
			if (hProcess)
			{
				rs = NtTerminateProcess(hProcess, 0);
				NtClose(hProcess);
				if (rs == 0) return STATUS_SUCCESS;
				else {
					currentLogger->LogError(L"TerminateProcess failed : (PID : %d) NTSTATUS : 0x%08X", dwId, rs);
					return rs;
				}
			}
			return rs;
		}
		else return STATUS_ACCESS_DENIED;
	}
}
NTSTATUS MOpenThreadNt(DWORD dwId, PHANDLE pLandle, DWORD dwPId)
{
	HANDLE hThread;
	OBJECT_ATTRIBUTES ObjectAttributes;
	CLIENT_ID ClientId;

	ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
	ObjectAttributes.RootDirectory = NULL;
	ObjectAttributes.ObjectName = NULL;
	ObjectAttributes.Attributes = OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE;
	ObjectAttributes.SecurityDescriptor = NULL;
	ObjectAttributes.SecurityQualityOfService = NULL;

	ClientId.UniqueThread = ((PVOID)(ULONG_PTR)dwId);
	ClientId.UniqueProcess = ((PVOID)(ULONG_PTR)dwPId);

	DWORD NtStatus = NtOpenThread(
		&hThread,
		THREAD_ALL_ACCESS,
		&ObjectAttributes,
		&ClientId);

	if (NtStatus == 0) {
		*pLandle = hThread;
		return 0;
	}
	else {
		return 0;
	}
}
NTSTATUS MTerminateThreadNt(DWORD dwTid, HANDLE handle, DWORD dwPid)
{
	if(handle) return NtTerminateThread(handle, 0);
	else if (dwTid) {
		HANDLE hThread = NULL;
		NTSTATUS ntStatus = 0;
		ntStatus = MOpenThreadNt(dwTid, &hThread, dwPid);
		if (NT_SUCCESS(ntStatus))
			return NtTerminateThread(handle, 0);
		return ntStatus;
	}
	return 0;
}
NTSTATUS MResumeThreadNt(HANDLE handle)
{
	ULONG count = 0;
	return NtResumeThread(handle, &count);
}
NTSTATUS MSuspendThreadNt(HANDLE handle)
{
	ULONG count = 0;
	return NtSuspendThread(handle, &count);
}

