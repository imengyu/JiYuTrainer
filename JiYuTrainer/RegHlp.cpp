#include "stdafx.h"
#include "RegHlp.h"
#include "AppPublic.h"
#include "PathHelper.h"
#include "StringHlp.h"
#include <shlwapi.h>

extern JTApp * currentApp;
extern LoggerInternal * currentLogger;

//删除注册表键以及子键
BOOL MRegDeleteKey(HKEY hRootKey, LPWSTR path) {

	DWORD lastErr = SHDeleteKey(hRootKey, path);
	if (lastErr == ERROR_SUCCESS || lastErr == ERROR_FILE_NOT_FOUND)
		return TRUE;
	else
	{
		SetLastError(lastErr);
		return 0;
	}
}
BOOL MRegForceDeleteServiceRegkey(LPWSTR lpszDriverName)
{
	BOOL rs = FALSE;
	wchar_t regPath[MAX_PATH];
	wsprintf(regPath, L"SYSTEM\\CurrentControlSet\\services\\%s", lpszDriverName);
	rs = MRegDeleteKey(HKEY_LOCAL_MACHINE, regPath);

	if (!rs) currentLogger->LogWarn2(L"RegDeleteTree failed : %d in delete key HKEY_LOCAL_MACHINE\\%s", GetLastError(), regPath);
	else currentLogger->LogInfo(L"Service Key deleted : HKEY_LOCAL_MACHINE\\%s", regPath);

	wchar_t regName[MAX_PATH];
	wcscpy_s(regName, lpszDriverName);
	_wcsupr_s(regName);
	wsprintf(regPath, L"SYSTEM\\CurrentControlSet\\Enum\\Root\\LEGACY_%s", regName);
	rs = MRegDeleteKey(HKEY_LOCAL_MACHINE, regPath);

	if (!rs) {
		currentLogger->LogWarn2(L"RegDeleteTree failed : %d in delete key HKEY_LOCAL_MACHINE\\%s", GetLastError(), regPath);
		rs = TRUE;
	}
	else currentLogger->LogInfo(L"Service Key deleted : HKEY_LOCAL_MACHINE\\%s", regPath);

	return rs;
}
BOOL MRegReadKeyString64And32(HKEY hRootKey, LPCWSTR path32, LPCWSTR path64, LPCWSTR keyName, LPWSTR buffer, SIZE_T count) {
	BOOL rs = FALSE;
	HKEY hKey;
	LRESULT lastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SysHlp::Is64BitOS() ? path64 : path32, 0, KEY_WOW64_64KEY | KEY_READ, &hKey);
	if (lastError == ERROR_SUCCESS) {
		DWORD dwType = REG_SZ;
		WCHAR * Data = new WCHAR[count];
		DWORD cbData = count;
		lastError = RegQueryValueEx(hKey, keyName, 0, &dwType, (LPBYTE)Data, &cbData);		
		if (lastError == ERROR_SUCCESS) {
			wcscpy_s(buffer, count, Data);
			rs = TRUE;
		}
		delete Data;
		RegCloseKey(hKey);
	}
	SetLastError(lastError);
	return rs;
}
BOOL MRegReadKeyString(HKEY hRootKey, LPCWSTR path, LPCWSTR keyName, LPWSTR buffer, SIZE_T count) {
	BOOL rs = FALSE;
	HKEY hKey;
	LRESULT lastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, path, 0, KEY_READ, &hKey);
	if (lastError == ERROR_SUCCESS) {
		DWORD dwType = REG_SZ;
		WCHAR * Data = new WCHAR[count];
		DWORD cbData = count;
		lastError = RegQueryValueEx(hKey, keyName, 0, &dwType, (LPBYTE)Data, &cbData);
		if (lastError == ERROR_SUCCESS) {
			wcscpy_s(buffer, count, Data);
			rs = TRUE;
		}
		delete Data;
		RegCloseKey(hKey);
	}
	SetLastError(lastError);
	return rs;
}
/*
BOOL MRegCheckUninstallItemExists(LPCWSTR keyName) {
	WCHAR Data[MAX_PATH];
	if (MRegReadKeyString64And32(HKEY_LOCAL_MACHINE,
		FormatString(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s", keyName).c_str(),
		FormatString(L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\%s", keyName).c_str(),
		L"DisplayIcon", Data, MAX_PATH)) {
		return TRUE;
	}
	else currentLogger->LogWarn(L"MRegReadKeyString64And32 failed : %s (%d) ", PRINT_LAST_ERROR_STR);

	return FALSE;
}
*/