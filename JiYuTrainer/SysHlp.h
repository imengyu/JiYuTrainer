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

	static BOOL EnableDebugPriv(const wchar_t * name);

	static BOOL IsRunasAdmin();
	static BOOL Is64BitOS();

	static BOOL RunApplication(LPCWSTR path, LPCWSTR cmd);
	static BOOL RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd);

};

