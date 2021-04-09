#include "stdafx.h"
#include "SysHlp.h"
#include "NtHlp.h"
#include "VersionHelpers.h"
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <WinIoCtl.h>
#include <CommDlg.h>
#include <Commctrl.h>
#include <string>

SHSTDAPI_(BOOL) SHGetSpecialFolderPathW(__reserved HWND hwnd, __out_ecount(MAX_PATH) LPWSTR pszPath, __in int csidl, __in BOOL fCreate);

BOOL _Is64BitOS = -1;
BOOL _IsRunasAdmin = -1;
SystemVersion currentVersion = SystemVersionUnknow;
DWORD currentWindowsMajor = 0;
WCHAR szTempPath[MAX_PATH];
std::wstring szErrorCodeBuffer;

UINT  SysHlp::GetWindowsBulidVersion() {

	if (currentWindowsMajor == 0) {
		HKEY hKey;
		DWORD err;

		if (Is64BitOS()) err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &hKey);
		else err = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_WOW64_64KEY | KEY_READ, &hKey);

		if (err == ERROR_SUCCESS)
		{
			TCHAR bulidver[MAX_PATH] = { 0 };
			DWORD bulidverLength = MAX_PATH;
			DWORD bulidverType = REG_SZ;
			err = RegQueryValueEx(hKey, L"CurrentBuildNumber", 0, &bulidverType, (LPBYTE)&bulidver, &bulidverLength);
			if (err == ERROR_SUCCESS)
				currentWindowsMajor = static_cast<DWORD>(_wtoll(bulidver));
		}
	}
	return currentWindowsMajor;
}
SystemVersion SysHlp::GetSystemVersion()
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

bool SysHlp::RunApplication(LPCWSTR path, LPCWSTR cmd)
{
	return (INT)ShellExecute(NULL, L"open", path, cmd, NULL, SW_NORMAL) > 32;
}
bool SysHlp::RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd)
{
	return (INT)ShellExecute(NULL, (currentVersion == SystemVersionWindowsXP ?  L"open" : L"runas"), path, cmd, NULL, SW_NORMAL) > 32;
}

LPCWSTR SysHlp::ConvertErrorCodeToString(DWORD ErrorCode)
{
	HLOCAL LocalAddress = NULL;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, ErrorCode, 0, (LPWSTR)&LocalAddress, 0, NULL);
	szErrorCodeBuffer = (LPCWSTR)LocalAddress;
	if (szErrorCodeBuffer.size() >= 2 && szErrorCodeBuffer.substr(szErrorCodeBuffer.size() - 2, 2) == L"\r\n")
		szErrorCodeBuffer = szErrorCodeBuffer.substr(0, szErrorCodeBuffer.size() - 2);
	LocalFree(LocalAddress);
	return szErrorCodeBuffer.c_str();
}
LPCWSTR SysHlp::ConvertLasyErrorToString()
{
	return ConvertErrorCodeToString(GetLastError());
}
bool  SysHlp::CheckIsPortabilityDevice(LPCWSTR path) {
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
bool SysHlp::CheckIsDesktop(LPCWSTR path)
{
	wchar_t desktopPath[MAX_PATH];
	if (!SHGetSpecialFolderPathW(0, desktopPath, 0x0010, 0))
		return FALSE;
	return wcscmp(desktopPath, path) == 0;
}

LPCWSTR SysHlp::GetTempPath()
{
	::GetTempPath(MAX_PATH, szTempPath);
	return szTempPath;
}

bool SysHlp::EnableDebugPriv(const wchar_t * name)
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
bool SysHlp::IsRunasAdmin()
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
bool SysHlp::Is64BitOS()
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
bool SysHlp::CopyToClipBoard(LPCWSTR str)
{
	if (OpenClipboard(NULL))
	{
		int len = (wcslen(str) + 1) * sizeof(wchar_t);
		HGLOBAL hmem = GlobalAlloc(GHND, len);
		void* pmem = GlobalLock(hmem);
		EmptyClipboard();
		memcpy_s(pmem, len, str, len);
		SetClipboardData(CF_UNICODETEXT, hmem);
		CloseClipboard();
		GlobalFree(hmem);
		return true;
	}
	return false;
}
void SysHlp::OpenUrl(LPCWSTR str)
{
	ShellExecute(NULL, L"open", str, NULL, NULL, SW_SHOW);
}
bool SysHlp::ChooseFileSingal(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt, LPCWSTR strrs, size_t bufsize)
{
	if (strrs) {
		OPENFILENAME ofn;
		TCHAR szFile[MAX_PATH];
		if (fileName != 0 && wcslen(fileName) != 0)
			wcscpy_s(szFile, fileName);
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = hWnd;
		ofn.lpstrFile = szFile;
		ofn.lpstrFile[0] = '\0';
		ofn.nMaxFile = sizeof(szFile);
		ofn.nFilterIndex = 1;
		ofn.lpstrFilter = fileFilter;
		ofn.lpstrDefExt = defExt;
		ofn.lpstrFileTitle = (LPWSTR)title;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = startDir;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if (GetOpenFileName(&ofn))
		{
			//显示选择的文件。 szFile
			wcscpy_s((LPWSTR)strrs, bufsize, szFile);
			return TRUE;
		}
	}
	return 0;
}
bool SysHlp::HotKeyCtlToKeyCode(int code, UINT *outModf, UINT* outVk) {
	WORD loword = LOWORD(code);
	BYTE mode = HIBYTE(loword);

	if ((mode & HOTKEYF_ALT) == HOTKEYF_ALT)
		*outModf |= MOD_ALT;
	if ((mode & HOTKEYF_CONTROL) == HOTKEYF_CONTROL)
		*outModf |= MOD_CONTROL;
	if ((mode & HOTKEYF_SHIFT) == HOTKEYF_SHIFT)
		*outModf |= MOD_SHIFT;

	*outVk = LOBYTE(code);
	return true;
}