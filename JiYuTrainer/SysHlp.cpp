#include "stdafx.h"
#include "SysHlp.h"
#include "NtHlp.h"
#include "VersionHelpers.h"
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <WinIoCtl.h>

SHSTDAPI_(BOOL) SHGetSpecialFolderPathW(__reserved HWND hwnd, __out_ecount(MAX_PATH) LPWSTR pszPath, __in int csidl, __in BOOL fCreate);

BOOL _Is64BitOS = -1;
BOOL _IsRunasAdmin = -1;
SystemVersion currentVersion = SystemVersionUnknow;

SystemVersion SysHlpInternal::GetSystemVersion()
{
	if (currentVersion == SystemVersionUnknow) {
		if (IsWindows7OrGreater())
			currentVersion = SystemVersionWindows7OrLater;
		else if (IsWindowsVistaOrGreater())
			currentVersion = SystemVersionWindows7OrLater;
		else if (IsWindowsXPSP2OrGreater())
			currentVersion = SystemVersionWindowsXP;
		else currentVersion = SystemVersionNotSupport;
	}
	return currentVersion;
}

bool SysHlpInternal::RunApplication(LPCWSTR path, LPCWSTR cmd)
{
	return (INT)ShellExecute(NULL, L"open", path, cmd, NULL, SW_NORMAL) > 32;
}
bool SysHlpInternal::RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd)
{
	return (INT)ShellExecute(NULL, (currentVersion == SystemVersionWindowsXP ?  L"open" : L"runas"), path, cmd, NULL, SW_NORMAL) > 32;
}

LPCWSTR SysHlpInternal::ConvertErrorCodeToString(DWORD ErrorCode)
{
	HLOCAL LocalAddress = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ErrorCode, 0, (LPWSTR)&LocalAddress, 0, NULL);
	return (LPCWSTR)LocalAddress;
}
bool  SysHlpInternal::CheckIsPortabilityDevice(LPCWSTR path) {
	//
//path: "\\\\?\\F:"
#define IOCTL_STORAGE_QUERY_PROPERTY   CTL_CODE(IOCTL_STORAGE_BASE, 0x0500, METHOD_BUFFERED, FILE_ANY_ACCESS)
	typedef  struct _STORAGE_DEVICE_DESCRIPTOR
	{
		DWORD Version;                DWORD Size;
		BYTE  DeviceType;             BYTE  DeviceTypeModifier;
		BOOLEAN RemovableMedia;       BOOLEAN CommandQueueing;
		DWORD VendorIdOffset;         DWORD ProductIdOffset;
		DWORD ProductRevisionOffset;  DWORD SerialNumberOffset;
		STORAGE_BUS_TYPE BusType;     DWORD RawPropertiesLength;
		BYTE  RawDeviceProperties[1];
	} STORAGE_DEVICE_DESCRIPTOR;

	HANDLE hDisk;
	STORAGE_DEVICE_DESCRIPTOR devDesc;
	DWORD query[3] = { 0,0,1588180 };

	DWORD cbBytesReturned;

	TCHAR szBuf[300];
	wsprintf(szBuf, L"\\\\?\\%C:", path[0]);
	hDisk = CreateFile(szBuf, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hDisk == INVALID_HANDLE_VALUE)
		return false;

	if (DeviceIoControl(hDisk, IOCTL_STORAGE_QUERY_PROPERTY, &query, sizeof(query),
		&devDesc, sizeof(devDesc), &cbBytesReturned, NULL))
	{
		if (devDesc.BusType == BusTypeUsb)
		{
			CloseHandle(hDisk);
			return true;
		}
	}
	return false;
}
bool SysHlpInternal::CheckIsDesktop(LPCWSTR path)
{
	wchar_t desktopPath[MAX_PATH];
	if (!SHGetSpecialFolderPathW(0, desktopPath, 0x0010, 0))
		return FALSE;
	return wcscmp(desktopPath, path) == 0;
}

bool SysHlpInternal::EnableDebugPriv(const wchar_t * name)
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
BOOL SysHlpInternal::IsRunasAdmin()
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
BOOL SysHlpInternal::Is64BitOS()
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

