#pragma once
#include "stdafx.h"
#include "NtHlp.h"

BOOL KFShutdown();
BOOL KFReboot();
BOOL KForceKill(DWORD pid, NTSTATUS * pStatus);

BOOL KFSendDriverinitParam(bool isXp, bool isWin7);
BOOL KFInstallSelfProtect();

BOOL KFInjectDll(DWORD pid, LPWSTR dllPath);
