#include "stdafx.h"
#include "SysHlp.h"
#include "VersionHelpers.h"
#include <Shlwapi.h>
#include <ShellAPI.h>

BOOL _Is64BitOS = false;
BOOL _IsRunasAdmin = false;
SystemVersion currentVersion = SystemVersionUnknow;

SystemVersion SysHlp::GetSystemVersion()
{
	if (currentVersion == SystemVersionUnknow) {
		if (IsWindows7OrGreater())
			currentVersion = SystemVersionWindows7OrLater;
		else if (IsWindowsVistaOrGreater())
			currentVersion = SystemVersionWindows7OrLater;
		else if (IsWindowsXPSP2OrGreater())
			currentVersion = SystemVersionWindows7OrLater;
		else currentVersion = SystemVersionNotSupport;
	}
	return currentVersion;
}

BOOL SysHlp::RunApplication(LPCWSTR path, LPCWSTR cmd)
{
	return (INT)ShellExecute(NULL, L"open", path, cmd, NULL, SW_NORMAL) > 32;
}
BOOL SysHlp::RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd)
{
	return (INT)ShellExecute(NULL, (currentVersion == SystemVersionWindowsXP ?  L"open" : L"runas"), path, cmd, NULL, SW_NORMAL) > 32;
}

LPCWSTR SysHlp::ConvertErrorCodeToString(DWORD ErrorCode)
{
	HLOCAL LocalAddress = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ErrorCode, 0, (LPWSTR)&LocalAddress, 0, NULL);
	return (LPCWSTR)LocalAddress;
}

BOOL SysHlp::EnableDebugPriv(const wchar_t * name)
{
	HANDLE hToken;
	TOKEN_PRIVILEGES tp;
	LUID luid;
	//打开进程令牌环
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
	//获得进程本地唯一ID
	LookupPrivilegeValue(NULL, name, &luid);

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	tp.Privileges[0].Luid = luid;
	//调整权限
	return AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL);
}
BOOL SysHlp::IsRunasAdmin()
{
	if (_IsRunasAdmin = -1)
	{
		BOOL bElevated = FALSE;
		HANDLE hToken = NULL;

		// Get current process token
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
			return FALSE;

		TOKEN_ELEVATION tokenEle;
		DWORD dwRetLen = 0;

		// Retrieve token elevation information
		if (GetTokenInformation(hToken, TokenElevation, &tokenEle, sizeof(tokenEle), &dwRetLen))
		{
			if (dwRetLen == sizeof(tokenEle))
			{
				bElevated = tokenEle.TokenIsElevated;
			}
		}

		CloseHandle(hToken);
		_IsRunasAdmin = bElevated;
	}
	return _IsRunasAdmin;
}
BOOL SysHlp::Is64BitOS()
{
	if (_Is64BitOS == -1)
	{
		BOOL bRetVal = FALSE;
		SYSTEM_INFO si = { 0 };

		GetNativeSystemInfo(&si);
		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ||
			si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64)
			bRetVal = TRUE;
		_Is64BitOS = bRetVal;
	}
	return _Is64BitOS;
}

