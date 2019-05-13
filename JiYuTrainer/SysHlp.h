#pragma once
#include "stdafx.h"

enum SystemVersion {
	SystemVersionUnknow,
	SystemVersionWindowsXP,
	SystemVersionWindowsVista,
	SystemVersionWindows7OrLater,
	SystemVersionNotSupport,
};

class EXPORT SysHlp
{
public:
	static SystemVersion GetSystemVersion();

	static LPCWSTR ConvertErrorCodeToString(DWORD ErrorCode);

	static bool CheckIsPortabilityDevice(LPCWSTR path);
	static bool CheckIsDesktop(LPCWSTR path);

	static bool EnableDebugPriv(const wchar_t * name);

	static BOOL IsRunasAdmin();
	static BOOL Is64BitOS();

	static bool RunApplication(LPCWSTR path, LPCWSTR cmd);
	static bool RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd);

};

