#pragma once
#include "stdafx.h"
#include <unknwn.h>
#include <ShellAPI.h>

enum VirusMode {
	VirusModeProtet,
	VirusModeHook,
	VirusModeMaster,
	VirusModeReplaceStub
};

EXTERN_C void __declspec(dllexport) VUnloadAll();
EXTERN_C void __declspec(dllexport) VLoad();
void VRunMain();

void VLoadRealVirus();
void VLoadMainProtect();
void VParamInit();

void VCreateMsgCenter();
DWORD WINAPI VMsgCenterRunThread(LPVOID lpThreadParameter);
void VCloseMsgCenter();

void VHandleMsg(LPWSTR buff);
void VOutPutStatus(const wchar_t * str, ...);
void VHookFWindow(const wchar_t * hWndStr);

void VHookWindow(const wchar_t * hWndStr);
void VFixGuangBoWindow(HWND hWnd);
bool VIsInIllegalWindows(HWND hWnd);
void VBoom();
void VSendMessageBack(LPCWSTR buff, HWND hDlg);
void VManualQuit();

BOOL VShowOpConfirmDialog(LPCWSTR file, LPCWSTR pararm);

void VLoadOpWhiteList();
void VAddOpToWhiteList(const wchar_t * cmd);
bool VIsOpInWhiteList(const wchar_t * cmd);

BOOL VShowOutOfControlDlg();

void VInstallHooks(VirusMode mode);
void VUnInstallHooks();

void VOpenFuckDrivers();
void VCloseFuckDrivers();

void VUnHookKeyBoard();

void VGetStudentainVersion();

EXTERN_C BOOL VGetExeInfo(LPWSTR strFilePath, LPCWSTR InfoItem, LPWSTR str, int maxCount);

INT_PTR CALLBACK FakeDesktopWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK JiYuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK JiYuTDDeskWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

typedef HHOOK(*fnDoneHook)();
typedef int(*fnUnHookLocalInput)(DWORD *bytesReturn);
typedef HMODULE(*fnUnLockLocalInput)();
typedef BOOL (WINAPI *fnSetForegroundWindow)(HWND hWnd);
typedef BOOL (WINAPI *fnSetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags);
typedef BOOL (WINAPI *fnMoveWindow)(HWND hWnd, int X, int Y, int nWidth, int nHeight, BOOL bRepaint);
typedef BOOL (WINAPI *fnBringWindowToTop)(HWND hWnd); 
typedef BOOL (WINAPI *fnDeviceIoControl)(
	__in        HANDLE hDevice,
	__in        DWORD dwIoControlCode,
	__in_bcount_opt(nInBufferSize) LPVOID lpInBuffer,
	__in        DWORD nInBufferSize,
	__out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
	__in        DWORD nOutBufferSize,
	__out_opt   LPDWORD lpBytesReturned,
	__inout_opt LPOVERLAPPED lpOverlapped
);
typedef HANDLE (WINAPI *fnCreateFileA)(
	__in     LPCSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
typedef HANDLE(WINAPI *fnCreateFileW)(
	__in     LPCWSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
typedef HHOOK (WINAPI *fnSetWindowsHookExA)(
	__in int idHook,
	__in HOOKPROC lpfn,
	__in_opt HINSTANCE hmod,
	__in DWORD dwThreadId);
typedef HDWP (WINAPI *fnDeferWindowPos)(
	_In_ HDWP hWinPosInfo,
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ int x,
	_In_ int y,
	_In_ int cx,
	_In_ int cy,
	_In_ UINT uFlags);
typedef VOID(WINAPI *fnmouse_event)(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
typedef UINT (WINAPI *fnSendInput)(
	__in UINT cInputs,                     // number of input in the array
	__in_ecount(cInputs) LPINPUT pInputs,  // array of inputs
	__in int cbSize);
typedef BOOL(WINAPI *fnSetThreadDesktop)(__in HDESK hDesktop);
typedef LONG(WINAPI *fnChangeDisplaySettingsW)(
	__in_opt DEVMODEW* lpDevMode,
	__in DWORD dwFlags);
typedef HDESK (WINAPI  *fnOpenDesktopA)(
	__in LPCSTR lpszDesktop,
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
typedef HDESK (WINAPI  *fnOpenInputDesktop)(
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
typedef HRESULT(__cdecl *fnTDDeskCreateInstance)(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv);
typedef LONG (WINAPI  *fnSetWindowLongA)(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
typedef LONG (WINAPI  *fnSetWindowLongW)(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
typedef HRESULT(__cdecl *fnTDAjustCreateInstance)(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv);
typedef BOOL (WINAPI *fnShowWindow)(
	_In_ HWND hWnd,
	_In_ int nCmdShow);
typedef BOOL (WINAPI *fnExitWindowsEx)(
	__in UINT uFlags,
	__in DWORD dwReason);
typedef HINSTANCE (WINAPI* fnShellExecuteW)(__in_opt HWND hwnd, __in_opt LPCWSTR lpOperation, __in LPCWSTR lpFile, __in_opt LPCWSTR lpParameters,
	__in_opt LPCWSTR lpDirectory, __in INT nShowCmd);
typedef BOOL (WINAPI* fnShellExecuteExW)(__inout SHELLEXECUTEINFOW *pExecInfo);
typedef BOOL (WINAPI* fnCreateProcessW)(
	__in_opt    LPCWSTR lpApplicationName,
	__inout_opt LPWSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCWSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOW lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
);
typedef BOOL(WINAPI* fnCreateProcessA)(
	__in_opt    LPCSTR lpApplicationName,
	__inout_opt LPSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOA lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
);
typedef HRESULT (WINAPI *fnDwmEnableComposition)(UINT uCompositionAction);
typedef UINT (WINAPI*fnWinExec)(LPCSTR lpCmdLine,	UINT uCmdShow);
typedef LRESULT (WINAPI *fnCallNextHookEx)( HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam);
typedef HWND (WINAPI* fnGetDesktopWindow)(VOID);
typedef HDC (WINAPI *fnGetWindowDC)(__in_opt HWND hWnd);
typedef int(__cdecl *fnEncodeToJPEGBuffer)(int a1, int a2, int a3, int a4, int a5, DWORD *a6, int a7, int a8, int a9);
typedef HWND(WINAPI*fnGetForegroundWindow)(VOID);
typedef HDC (WINAPI *fnCreateDCW)(__in_opt LPCWSTR pwszDriver, __in_opt LPCWSTR pwszDevice, __in_opt LPCWSTR pszPort, __in_opt CONST DEVMODEW * pdm);
typedef BOOL(WINAPI *fnEnableMenuItem)(HMENU hMenu, UINT  uIDEnableItem, UINT  uEnable);
typedef DWORD (WINAPI *fnSetClassLongA)(__in HWND hWnd, __in int nIndex, __in LONG dwNewLong);
typedef DWORD (WINAPI *fnSetClassLongW)(__in HWND hWnd, __in int nIndex, __in LONG dwNewLong);
typedef BOOL (WINAPI *fnUnhookWindowsHookEx)(HHOOK hhk);
typedef BOOL (WINAPI *fnPostMessageW)(__in_opt HWND hWnd, __in UINT Msg, __in WPARAM wParam, __in LPARAM lParam);
typedef LRESULT (WINAPI *fnSendMessageW)(__in HWND hWnd, __in UINT Msg, __in WPARAM wParam, __in LPARAM lParam);
typedef BOOL (WINAPI *fnTerminateProcess)(__in HANDLE hProcess,	__in UINT uExitCode);
typedef HRESULT (WINAPI *fnFilterConnectCommunicationPort)(LPCWSTR lpPortName, DWORD dwOptions, LPCVOID lpContext, WORD wSizeOfContext, LPSECURITY_ATTRIBUTES lpSecurityAttributes, HANDLE *hPort);


//Hooks

BOOL WINAPI hkExitWindowsEx(__in UINT uFlags, __in DWORD dwReason);
BOOL WINAPI hkSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags);
BOOL WINAPI hkMoveWindow(HWND hWnd, int x, int y, int cx, int cy, BOOL bRepaint);
BOOL WINAPI hkSetForegroundWindow(HWND hWnd);
BOOL WINAPI hkBringWindowToTop(_In_ HWND hWnd);
BOOL WINAPI hkDeviceIoControl(
	__in        HANDLE hDevice,
	__in        DWORD dwIoControlCode,
	__in_bcount_opt(nInBufferSize) LPVOID lpInBuffer,
	__in        DWORD nInBufferSize,
	__out_bcount_part_opt(nOutBufferSize, *lpBytesReturned) LPVOID lpOutBuffer,
	__in        DWORD nOutBufferSize,
	__out_opt   LPDWORD lpBytesReturned,
	__inout_opt LPOVERLAPPED lpOverlapped
	);
HANDLE WINAPI hkCreateFileA(
	__in     LPCSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
HANDLE WINAPI hkCreateFileW(
	__in     LPCWSTR lpFileName,
	__in     DWORD dwDesiredAccess,
	__in     DWORD dwShareMode,
	__in_opt LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	__in     DWORD dwCreationDisposition,
	__in     DWORD dwFlagsAndAttributes,
	__in_opt HANDLE hTemplateFile
);
HHOOK WINAPI hkSetWindowsHookExA(
	__in int idHook,
	__in HOOKPROC lpfn,
	__in_opt HINSTANCE hmod,
	__in DWORD dwThreadId);
HDWP WINAPI hkDeferWindowPos(
	_In_ HDWP hWinPosInfo,
	_In_ HWND hWnd,
	_In_opt_ HWND hWndInsertAfter,
	_In_ int x,
	_In_ int y,
	_In_ int cx,
	_In_ int cy,
	_In_ UINT uFlags);
UINT WINAPI hkSendInput(
	__in UINT cInputs,                     // number of input in the array
	__in_ecount(cInputs) LPINPUT pInputs,  // array of inputs
	__in int cbSize);
VOID WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData, ULONG_PTR dwExtraInfo);
LONG WINAPI hkChangeDisplaySettingsW(
	__in_opt DEVMODEW* lpDevMode,
	__in DWORD dwFlags);
HDESK WINAPI hkOpenDesktopA(
	__in LPCSTR lpszDesktop,
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
HDESK WINAPI hkOpenInputDesktop(
	__in DWORD dwFlags,
	__in BOOL fInherit,
	__in ACCESS_MASK dwDesiredAccess);
LONG WINAPI hkSetWindowLongA(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
LONG WINAPI hkSetWindowLongW(
	__in HWND hWnd,
	__in int nIndex,
	__in LONG dwNewLong);
BOOL WINAPI hkShowWindow(
	_In_ HWND hWnd,
	_In_ int nCmdShow);
HINSTANCE WINAPI hkShellExecuteW(__in_opt HWND hwnd, __in_opt LPCWSTR lpOperation, __in LPCWSTR lpFile, __in_opt LPCWSTR lpParameters,
	__in_opt LPCWSTR lpDirectory, __in INT nShowCmd);
BOOL WINAPI hkShellExecuteExW(__inout SHELLEXECUTEINFOW *pExecInfo);
BOOL WINAPI hkCreateProcessW(
	__in_opt    LPCWSTR lpApplicationName,
	__inout_opt LPWSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCWSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOW lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
);
BOOL WINAPI hkCreateProcessA(
	__in_opt    LPCSTR lpApplicationName,
	__inout_opt LPSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOA lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
	);
HRESULT WINAPI hkDwmEnableComposition(UINT uCompositionAction);
UINT WINAPI hkWinExec(__in LPCSTR lpCmdLine,	 __in UINT uCmdShow);
LRESULT WINAPI hkCallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam);
HWND WINAPI hkGetDesktopWindow(VOID);
HDC WINAPI hkGetWindowDC(__in_opt HWND hWnd);
BOOL __cdecl hkEncodeToJPEGBuffer(int a1, int a2, int a3, int a4, int a5, DWORD *a6, int a7, int a8, int a9);
HWND WINAPI hkGetForegroundWindow(VOID);
HDC WINAPI hkCreateDCW(__in_opt LPCWSTR pwszDriver, __in_opt LPCWSTR pwszDevice, __in_opt LPCWSTR pszPort, __in_opt CONST DEVMODEW * pdm);
BOOL WINAPI hkEnableMenuItem(HMENU hMenu,	UINT  uIDEnableItem,	 UINT  uEnable);
DWORD WINAPI hkSetClassLongA(__in HWND hWnd, __in int nIndex,	__in LONG dwNewLong);
DWORD WINAPI hkSetClassLongW(__in HWND hWnd, __in int nIndex,	__in LONG dwNewLong);
BOOL WINAPI hkUnhookWindowsHookEx(HHOOK hhk);
BOOL WINAPI hkPostMessageW(	__in_opt HWND hWnd,	__in UINT Msg,	__in WPARAM wParam,	__in LPARAM lParam);
LRESULT WINAPI hkSendMessageW(	__in HWND hWnd,	__in UINT Msg,	__in WPARAM wParam,	__in LPARAM lParam);
BOOL WINAPI hkTerminateProcess(__in HANDLE hProcess,	__in UINT uExitCode);
HRESULT WINAPI hkFilterConnectCommunicationPort(LPCWSTR lpPortName,	DWORD dwOptions,	LPCVOID lpContext,	WORD wSizeOfContext,	LPSECURITY_ATTRIBUTES lpSecurityAttributes,	HANDLE *hPort);



INT_PTR CALLBACK VShowOpConfirmWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

typedef struct tag_WNDCOUNTDOWN {
	int count;
	HWND wnd;
}WNDCOUNTDOWN, *PWNDCOUNTDOWN;
typedef struct tag_WNDIJJ {
	HWND wnd;
	bool canSizeToFull;
}WNDIJJ,*PWNDIJJ;