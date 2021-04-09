#pragma once
#include "stdafx.h"

enum SystemVersion {
	SystemVersionUnknow,
	SystemVersionWindowsXP,
	SystemVersionWindowsVista,
	SystemVersionWindows7OrLater,
	SystemVersionNotSupport,
};

#define PRINT_LAST_ERROR_STR SysHlp::ConvertLasyErrorToString(), GetLastError()

class SysHlp
{
public:
	static SystemVersion GetSystemVersion();
	static LPCWSTR ConvertErrorCodeToString(DWORD ErrorCode);
	static LPCWSTR ConvertLasyErrorToString();

	static bool CheckIsPortabilityDevice(LPCWSTR path);
	static bool CheckIsDesktop(LPCWSTR path);

	static LPCWSTR GetTempPath();
	static bool EnableDebugPriv(const wchar_t * name);
	static UINT GetWindowsBulidVersion();

	static bool IsRunasAdmin();
	static bool Is64BitOS();
	static bool CopyToClipBoard(LPCWSTR str);
	static void OpenUrl(LPCWSTR str);
	static bool RunApplication(LPCWSTR path, LPCWSTR cmd);
	static bool RunApplicationPriviledge(LPCWSTR path, LPCWSTR cmd);

	static bool ChooseFileSingal(HWND hWnd, LPCWSTR startDir, LPCWSTR title, LPCWSTR fileFilter, LPCWSTR fileName, LPCWSTR defExt, LPCWSTR strrs, size_t bufsize);
	static bool HotKeyCtlToKeyCode(int code, UINT* outModf, UINT* outVk);
};

