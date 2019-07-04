#pragma once
#include "stdafx.h"

enum SystemVersion {
	SystemVersionUnknow,
	SystemVersionWindowsXP,
	SystemVersionWindowsVista,
	SystemVersionWindows7OrLater,
	SystemVersionNotSupport,
};
class SysHlp
{
public:
	virtual SystemVersion GetSystemVersion() { return SystemVersion::SystemVersionUnknow; }

	virtual LPCWSTR ConvertErrorCodeToString(DWORD ErrorCode) { return false; }

	virtual bool CheckIsPortabilityDevice(LPCWSTR path) { return false; }
	virtual bool CheckIsDesktop(LPCWSTR path) { return false; }

	virtual bool EnableDebugPriv(const wchar_t * name) { return false; }

	virtual BOOL IsRunasAdmin() { return false; }
	virtual BOOL Is64BitOS() { return false; }

	virtual bool RunApplication(LPCWSTR path, LPCWSTR cmd) { return false; }
	virtual bool RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd) { return false; }

};
class SysHlpInternal : public SysHlp
{
public:
	SystemVersion GetSystemVersion();
 LPCWSTR ConvertErrorCodeToString(DWORD ErrorCode);

	bool CheckIsPortabilityDevice(LPCWSTR path);
	bool CheckIsDesktop(LPCWSTR path);

	bool EnableDebugPriv(const wchar_t * name);

	BOOL IsRunasAdmin();
	BOOL Is64BitOS();

	bool RunApplication(LPCWSTR path, LPCWSTR cmd);
	bool RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd);
};

