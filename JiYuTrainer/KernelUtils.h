#pragma once
#include "stdafx.h"
#include "NtHlp.h"

EXPORT_CFUNC(BOOL) KFShutdown();
EXPORT_CFUNC(BOOL) KFReboot();
EXPORT_CFUNC(BOOL) KForceKill(DWORD pid, NTSTATUS * pStatus);

EXPORT_CFUNC(BOOL) KFSendDriverinitParam(bool isXp, bool isWin7);
EXPORT_CFUNC(BOOL) KFInstallSelfProtect();

EXPORT_CFUNC(BOOL) KFInjectDll(DWORD pid, LPWSTR dllPath);
