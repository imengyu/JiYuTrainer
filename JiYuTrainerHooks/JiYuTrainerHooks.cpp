
// JiYuKillerVirus.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "JiYuTrainerHooks.h"
#include "resource.h"
#include "StringHlp.h"
#include "StringSplit.h"
#include "mhook-lib/mhook.h"
#include <list>
#include <string>
#include <time.h>
#include <stdio.h>
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <CommCtrl.h>
#include <windowsx.h>
#include <dwmapi.h>
#include <TlHelp32.h>

#define TIMER_WATCH_DOG_SRV 10011
#define TIMER_AUTO_HIDE 10012
#define TIMER_COUNTDOWN_TICK 10013
#define TIMER_LIGNT_DELAY1 10014
#define TIMER_LIGNT_DELAY2 10015

#define DirectorySeparatorChar L'\\'
#define AltDirectorySeparatorChar  L'/'
#define VolumeSeparatorChar  L':'

#define IDC_SW_STATUS_FAKEFULL 20001
#define IDC_SW_STATUS_LOCKED 20002
#define IDC_SW_STATUS_MAIN 20003
#define IDC_SW_STATUS_MAIN_OUT 20004
#define IDC_SW_STATUS_MAIN_FLASH 20005
#define IDC_SW_STATUS_RB_FLASH 20006
#define IDC_SW_STATUS_RB 20007
#define IDC_SW_STATUS_RB_FAIL 20008
#define IDC_SW_STATUS_RB_FAIL_FLASH 20009
#define IDC_SW_STATUS_RB_FLASH_FAST 20010

#define IDM_TOPMOST 25600
#define IDM_FULL 25601
#define IDM_HELP_GB 25602
#define IDM_CLOSE_GB 25603
#define IDM_HELP_GB2 25604

#define INVALID_HHOOK_MOUSE 0x0001
#define INVALID_HHOOK_KEYBOARD 0x0002

using namespace std;

extern HINSTANCE hInst;

WNDPROC jiYuWndProc;
WNDPROC jiYuTDDeskWndProc;
list<HWND>  jiYuWnds;
list<HWND>  jiYuWndCanSize;
HWND jiYuGBWnd = NULL;
HWND jiYuGBDeskRdWnd = NULL;
HWND jiYuGBToolWnd = NULL;
int jiYuGBToolHeight = 0;

enum jiYuVersions {
	jiYuVersionsAuto,
	jiYuVersions40,
	jiYuVersions402016HH,
};
jiYuVersions jiYuVersion = jiYuVersionsAuto;

HWND hWndMsgCenter = NULL;
HWND hListBoxStatus = NULL;

HWND desktopWindow, fakeDesktopWindow;
HWND mainWindow;

HMENU hMenuGb = NULL;
HMENU hMenuGbP;
HANDLE hThreadMain = NULL;
LRESULT hWndOpConformRs = 0;
LRESULT hWndOutOfControlConformRs = 0;
INT screenWidth, screenHeight;
bool outlineEndJiy = false;
HWND hWndOpConformNoBtn = NULL;
bool bandAllRunOp = false, allowNextRunOp = false, allowAllRunOp  = false, ProhibitKillProcess = false, ProhibitCloseWindow = false;
bool allowMonitor = false, allowControl = false, allowGbTop = false, fakeFull = false, gbFullManual = false,
doNotShowVirusWindow = true, forceDisableWatchDog = false;
bool isLocked = false, gbCurrentIsTop = false, isGbFounded = false;
std::list<std::wstring> runOPWhiteList;
bool forceKill = false;
int wdCount = 0;
WCHAR mainIniPath[MAX_PATH];
WCHAR mainFullPath[MAX_PATH];
WCHAR mainDir[MAX_PATH];
WCHAR currOpCfPath[MAX_PATH];
WCHAR currOpCfPararm[MAX_PATH];
HWND gbWindow = NULL;
DWORD currentPid = 0;

fnTDAjustCreateInstance faTDAjustCreateInstance = NULL;

fnUnLockLocalInput UnLockLocalInput = NULL;
fnSetWindowPos raSetWindowPos = NULL;
fnMoveWindow raMoveWindow = NULL;
fnSetForegroundWindow raSetForegroundWindow = NULL;
fnBringWindowToTop faBringWindowToTop = NULL;
fnDeviceIoControl raDeviceIoControl = NULL;
fnCreateFileA faCreateFileA = NULL;
fnCreateFileW faCreateFileW = NULL;
fnSetWindowsHookExA faSetWindowsHookExA = NULL;
fnDeferWindowPos faDeferWindowPos = NULL;
fnSendInput faSendInput = NULL;
fnmouse_event famouse_event = NULL;
fnChangeDisplaySettingsW faChangeDisplaySettingsW = NULL;
fnTDDeskCreateInstance faTDDeskCreateInstance = NULL;
fnSetWindowLongA faSetWindowLongA = NULL;
fnSetWindowLongW faSetWindowLongW = NULL;
fnShowWindow faShowWindow = NULL;
fnExitWindowsEx faExitWindowsEx = NULL;
fnShellExecuteW faShellExecuteW = NULL;
fnShellExecuteExW faShellExecuteExW = NULL;
fnCreateProcessA faCreateProcessA = NULL;
fnCreateProcessW faCreateProcessW = NULL;
fnDwmEnableComposition faDwmEnableComposition = NULL;
fnWinExec faWinExec = NULL;
fnCallNextHookEx faCallNextHookEx = NULL;
fnGetDesktopWindow faGetDesktopWindow = NULL;
fnGetWindowDC faGetWindowDC = NULL;
fnEncodeToJPEGBuffer faEncodeToJPEGBuffer = NULL;
fnGetForegroundWindow faGetForegroundWindow = NULL;
fnCreateDCW faCreateDCW = NULL;
fnEnableMenuItem faEnableMenuItem = NULL;
fnSetClassLongA faSetClassLongA = NULL;
fnSetClassLongW faSetClassLongW = NULL;
fnUnhookWindowsHookEx faUnhookWindowsHookEx = NULL;
fnPostMessageW  faPostMessageW = NULL;
fnSendMessageW faSendMessageW = NULL;
fnTerminateProcess faTerminateProcess = NULL;
fnFilterConnectCommunicationPort faFilterConnectCommunicationPort = NULL;


bool loaded = false;

HHOOK g_hhook = NULL;

HMODULE hLibTDMaster;

BOOL hk1 = 0, hk2 = 0, hk3 = 0, hk4 = 0,
hk5 = 0, hk6 = 0, hk7 = 0, hk8 = 0,
hk9 = 0, hk10 = 0, hk11 = 0, hk12 = 0,
hk13 = 0, hk14 = 0, hk15 = 0, hk16 = 0,
hk17 = 0, hk18 = 0, hk19 = 0, hk20 = 0,
hk21 = 0, hk22 = 0, hk23 = 0, hk24 = 0,
hk25 = 0, hk26 = 0, hk27 = 0, hk28 = 0,
hk29 = 0, hk30 = 0, hk31 = 0, hk32 = 0,
hk33 = 0, hk34 = 0, hk35 = 0, hk36 = 0,
hk37 = 0, hk38 = 0, hk39 = 0, hk40 = 0;

void VUnloadAll() {

	if (loaded)
	{
		VCloseMsgCenter();
		VCloseFuckDrivers();
		jiYuWnds.clear();
		jiYuWndCanSize.clear();
		runOPWhiteList.clear();
		if (hMenuGb) DestroyMenu(hMenuGb);
		VUnInstallHooks();
		loaded = false;
		FreeLibrary(hInst);
	}
}
void VLoad()
{
	VParamInit();

	//Get main mod name
	WCHAR mainModName[MAX_PATH];
	GetModuleFileName(NULL, mainModName, MAX_PATH);

	std::wstring path(mainModName);

	int lastQ = path.find_last_of(L'\\');
	std::wstring name = path.substr(lastQ + 1, path.length() - lastQ - 1);

	if (name == L"StudentMain.exe") {

		hLibTDMaster = GetModuleHandle(L"LibTDMaster.dll");
		if (hLibTDMaster == NULL) {
			//这不是极域, 混入了什么奇怪的东西？

			WCHAR buf[32];
			swprintf_s(buf, L"hkb:wtf:%d", GetCurrentProcessId());
			VSendMessageBack(buf, hWndMsgCenter);
			VUnloadAll();
			FreeLibrary(hInst);//卸载本体
		}

		//This is target, run virus

		GetModuleFileName(hInst, mainModName, MAX_PATH);
		path = std::wstring(mainModName);
		lastQ = path.find_last_of(L'\\');
		name = path.substr(lastQ + 1, path.length() - lastQ - 1);

		if (name == L"LibTDAjust.dll") { //Current is virus stub dll , load real and alloc
			VLoadRealVirus();
			VRunMain();
		}
		else if (name == L"JiYuTrainerHooks.dll") {//Current is virus main dll
			VRunMain();
		}
	}
	else if (name == L"JiYuTrainer.exe") {
		VLoadMainProtect();
	}
	else if (name == L"MasterHelper.exe") {
		//MasterHelper.exe 搞一些事情
		VInstallHooks(VirusModeMaster);
	}
	else if (name == L"ProcHelper64.exe") {
		//ProcHelper64.exe 搞一些事情
		VInstallHooks(VirusModeMaster);
	}

	loaded = true;
}
void VRunMain() {
	desktopWindow = GetDesktopWindow();

	VCreateMsgCenter();
	VOpenFuckDrivers();
	VInstallHooks(VirusModeHook);
	VSendMessageBack(L"hkb:succ", hWndMsgCenter);
}
void VLoadRealVirus() {
	if (_waccess_s(L"LibTDAjust.dll.bak.dll", 0) == 0)
	{
		HMODULE hrealTDAjust = LoadLibrary(L"LibTDAjust.dll.bak.dll");
		if (!hrealTDAjust) {
			MessageBox(0, L"!hrealTDAjust ", L"ERROR!", MB_ICONERROR);
			ExitProcess(0);
		}
		faTDAjustCreateInstance = (fnTDAjustCreateInstance)GetProcAddress(hrealTDAjust, "TDAjustCreateInstance");
		if (!faTDAjustCreateInstance) {
			MessageBox(0, L"!faTDAjustCreateInstance", L"ERROR!", MB_ICONERROR);
			ExitProcess(0);
		}
	}
	else {
		MessageBox(0, L"!LibTDAjust.dll.bak.dll", L"ERROR!", MB_ICONERROR);
		ExitProcess(0);
		
	}
}
void VLoadMainProtect() {

}

DWORD WINAPI VMsgCenterRunThread(LPVOID lpThreadParameter) {

	fakeDesktopWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_FAKEDESKTOP), desktopWindow, FakeDesktopWndProc);
	hWndMsgCenter = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MSGCT), desktopWindow, MainWndProc);
	ShowWindow(hWndMsgCenter, SW_SHOW);
	UpdateWindow(hWndMsgCenter);
	ShowWindow(fakeDesktopWindow, SW_SHOW);
	UpdateWindow(fakeDesktopWindow);

	hListBoxStatus = GetDlgItem(hWndMsgCenter, IDC_STATUS_LIST);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	hThreadMain = NULL;
	return 0;
}

void VParamInit() {
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	currentPid = GetCurrentProcessId();

	//创建广播的右键菜单

	HBITMAP hIconExit = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_EXIT));
	HBITMAP hIconHelp = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_HELP));

	hMenuGb = CreatePopupMenu();
	AppendMenu(hMenuGb, MF_STRING , IDM_HELP_GB, L"JiYuTrainer 添加的菜单");
	AppendMenu(hMenuGb, MF_STRING, IDM_HELP_GB2, L"JiYuTrainer 帮助");
	AppendMenu(hMenuGb, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenuGb, MF_STRING, IDM_FULL, L"广播窗口全屏");
	AppendMenu(hMenuGb, MF_STRING, IDM_TOPMOST, L"广播窗口置顶");
	AppendMenu(hMenuGb, MF_SEPARATOR, 0, NULL);
	AppendMenu(hMenuGb, MF_STRING, IDM_CLOSE_GB, L"关闭广播窗口");

	SetMenuItemBitmaps(hMenuGb, IDM_CLOSE_GB, MF_BITMAP, hIconExit, hIconExit);
	SetMenuItemBitmaps(hMenuGb, IDM_HELP_GB2, MF_BITMAP, hIconHelp, hIconHelp);
}
void VInitSettings()
{
	WCHAR w[32];
	GetPrivateProfileString(L"JTSettings", L"AutoForceKill", L"FALSE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) forceKill = true;
	else forceKill = false;
	GetPrivateProfileString(L"JTSettings", L"AllowAllRunOp", L"FALSE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) allowAllRunOp = true;
	else allowAllRunOp = false;
	GetPrivateProfileString(L"JTSettings", L"BandAllRunOp", L"TRUE", w, 32, mainIniPath);
	if (!StrEqual(w, L"TRUE") && !StrEqual(w, L"true") && !StrEqual(w, L"1")) bandAllRunOp = false;
	else bandAllRunOp = true;
	GetPrivateProfileString(L"JTSettings", L"ProhibitKillProcess", L"TRUE", w, 32, mainIniPath);
	if (!StrEqual(w, L"TRUE") && !StrEqual(w, L"true") && !StrEqual(w, L"1")) ProhibitKillProcess = false;
	else ProhibitKillProcess = true;
	GetPrivateProfileString(L"JTSettings", L"ProhibitCloseWindow", L"TRUE", w, 32, mainIniPath);
	if (!StrEqual(w, L"TRUE") && !StrEqual(w, L"true") && !StrEqual(w, L"1")) ProhibitCloseWindow = false;
	else ProhibitCloseWindow = true;
	
	GetPrivateProfileString(L"JTSettings", L"DoNotShowVirusWindow", L"TRUE", w, 32, mainIniPath);
	if (!StrEqual(w, L"TRUE") && !StrEqual(w, L"true") && !StrEqual(w, L"1")) doNotShowVirusWindow = false;
	else doNotShowVirusWindow = true;

	GetPrivateProfileString(L"JTSettings", L"ForceDisableWatchDog", L"FALSE", w, 32, mainIniPath);
	if (!StrEqual(w, L"TRUE") && !StrEqual(w, L"true") && !StrEqual(w, L"1")) forceDisableWatchDog = false;
	else forceDisableWatchDog = true;

	GetPrivateProfileString(L"JTSettings", L"AllowGbTop", L"FALSE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) allowGbTop = true;
	else allowGbTop = false;

	GetPrivateProfileString(L"JTSettings", L"AllowMonitor", L"TRUE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) allowMonitor = true;
	else allowMonitor = false;
	GetPrivateProfileString(L"JTSettings", L"AllowControl", L"FALSE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) allowControl = true;
	else allowControl = false;
}
void VLaterInit()
{
	if (PathFileExists(mainIniPath)) 
	{
		VInitSettings();
		VLoadOpWhiteList();
	}
}
void VCreateMsgCenter() {
	hThreadMain = CreateThread(NULL, NULL, VMsgCenterRunThread, NULL, NULL, NULL);
}
void VCloseMsgCenter() {
	DestroyWindow(hWndMsgCenter);
	if (hThreadMain) {
		TerminateThread(hThreadMain, 0);
		hThreadMain = NULL;
	}
}
void VHandleMsg(LPWSTR buff) {
	wstring act(buff);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2) {
		if (arr[0] == L"hw")  VHookWindow(arr[1].c_str());
		else if (arr[0] == L"hwf") VHookFWindow(arr[1].c_str());
		else if (arr[0] == L"sh") {
			HWND hWnd = (HWND)_wtol(arr[1].c_str());
			if (IsWindow(hWnd)) mainWindow = hWnd;
			VOutPutStatus(L"[S] %s", buff);
		}
		else if (arr[0] == L"ss") VBoom();
		else if (arr[0] == L"ss2") {
			PostQuitMessage(0);
			ExitProcess(0);
		}
		else if (arr[0] == L"hk") {
			VOutPutStatus(L"[V] %s", buff);
			if (arr[1] == L"ckstat") {
				if(jiYuVersion == jiYuVersionsAuto) VGetStudentainVersion();
				VUnHookKeyBoard();
				VSendMessageBack(L"hkb:succ", hWndMsgCenter);
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_MAIN, NULL);
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_RB_FLASH, 0);
			}
			else if (arr[1] == L"ckend") VManualQuit();
			else if (arr[1] == L"path" && arr.size() >= 4) {
				wcscpy_s(mainFullPath, arr[2].c_str());
				wcscat_s(mainFullPath, L":");
				wcscat_s(mainFullPath, arr[3].c_str());
			}
			else if (arr[1] == L"inipath" && arr.size() >= 4) {
				wcscpy_s(mainIniPath, arr[2].c_str());
				wcscat_s(mainIniPath, L":");
				wcscat_s(mainIniPath, arr[3].c_str());
				VLaterInit();
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_RB_FLASH, 0);
			}
			else if (arr[1] == L"reset") {
				VInitSettings();
				VSendMessageBack(L"hkb:succ", hWndMsgCenter);
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_RB_FAIL_FLASH, 0);
			}
			else if (arr[1] == L"fkfull" && arr.size() >= 3) {
				fakeFull = arr[2] == L"true";
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_FAKEFULL, NULL);
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_RB_FLASH, 0);
			}
		}
		else if (arr[0] == L"ukt") {
			VOutPutStatus(L"[T] %s", buff);
			if (UnLockLocalInput) { UnLockLocalInput(); VSendMessageBack(L"ukt:succ", hWndMsgCenter); }
			else VSendMessageBack(L"ukt:fail", hWndMsgCenter);
		}
		else if (arr[0] == L"test") {
			VOutPutStatus(L"[T] %s", buff);
			VShowOpConfirmDialog(L"test", L"test");
			VSendMessageBack(L"vback:test virus", hWndMsgCenter);
		}
		else if (arr[0] == L"test2") {
			if (arr[1] == L"f") {
				VOutPutStatus(L"test2 > f");
				SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_MAIN_FLASH, NULL);
			}
			//WCHAR str[300]; swprintf_s(str, L"vback:hk29 %s  faavcodec_encode_video : 0x%08X", hk29 ? L"TRUE" : L"FALSE", faavcodec_encode_video);
			//VSendMessageBack(str, hWndMsgCenter);
		}
	}
}
void VOutPutStatus(const wchar_t* str, ...) {
	time_t time_log = time(NULL);
	struct tm tm_log;
	localtime_s(&tm_log, &time_log);
	va_list arg;
	va_start(arg, str);
	wstring format1 = FormatString(L"[%02d:%02d:%02d] %s", tm_log.tm_hour, tm_log.tm_min, tm_log.tm_sec, str);
	wstring out = FormatString(format1.c_str(), arg);
	SendMessage(hListBoxStatus, LB_ADDSTRING, 0, (LPARAM)out.c_str());
	SendMessage(hListBoxStatus, LB_SETTOPINDEX, ListBox_GetCount(hListBoxStatus) - 1, 0);
	va_end(arg);
}
bool VWindowTextIsGb(const wchar_t* text) {

	return StringHlp::StrContainsW(text, L"广播", nullptr) || StringHlp::StrContainsW(text, L"演示", nullptr)
		|| StringHlp::StrContainsW(text, L"共享", nullptr) || StringHlp::StrEqualW(text, L"屏幕演播室窗口") 
		|| StringHlp::StrContainsW(text, L"共享屏幕", nullptr);
}
void VHookFWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[50];
			GetWindowText(hWnd, text, 50);
			if (VWindowTextIsGb(text)) {
				VFixGuangBoWindow(hWnd);
				jiYuGBWnd = hWnd;
			}
		}
		if (!VIsInIllegalWindows(hWnd)) {
			jiYuWnds.push_back(hWnd);
		}
	}
}
void VHookWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[50];
			GetWindowText(hWnd, text, 50);
			if (VWindowTextIsGb(text)) {
				VFixGuangBoWindow(hWnd);
				jiYuGBWnd = hWnd;
			}
		}
		//
		if (!VIsInIllegalWindows(hWnd)) {
			jiYuWnds.push_back(hWnd);
			//jiYuWndCanSize.push_back(hWnd);
		}
	}
}
void VFixGuangBoWindow(HWND hWnd)
{
	if (gbWindow != hWnd) {
		gbWindow = hWnd;
		VOutPutStatus(L"[H] Guang Bo Window : %d", hWnd);
	}
	//WNDPROC 接管
	WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
	if (oldWndProc != (WNDPROC)JiYuWndProc) {
		jiYuWndProc = (WNDPROC)oldWndProc;
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)JiYuWndProc);
		VOutPutStatus(L"[J] Hooked hWnd %d (0x%08x) WNDPROC", hWnd, hWnd);
		SendMessage(hWnd, WM_SHOWWINDOW, TRUE, FALSE);
	}
	LONG style = GetWindowLong(hWnd, GWL_STYLE);
	if ((style & WS_OVERLAPPEDWINDOW) != WS_OVERLAPPEDWINDOW)
		style |= WS_OVERLAPPEDWINDOW;
	if ((style & WS_SYSMENU) != WS_SYSMENU)
		style |= WS_SYSMENU;
	SetWindowLong(hWnd, GWL_STYLE, style);
	//增加菜单
	GetSystemMenu(gbWindow, TRUE);
	HMENU hMenu = GetSystemMenu(gbWindow, FALSE);
	AppendMenu(hMenu, MF_SEPARATOR, MF_SEPARATOR, L"");
	AppendMenu(hMenu, MF_STRING, IDM_FULL, L"广播窗口全屏");
	AppendMenu(hMenu, MF_STRING, IDM_TOPMOST, L"广播窗口置顶");

	LONG oldLong = GetWindowLong(gbWindow, GWL_EXSTYLE);
	if ((oldLong & WS_EX_TOPMOST) == WS_EX_TOPMOST) {
		CheckMenuItem(hMenu, IDM_TOPMOST, MF_CHECKED);
		CheckMenuItem(hMenuGb, IDM_TOPMOST, MF_CHECKED);
		gbCurrentIsTop = true;
	}
	else {
		CheckMenuItem(hMenu, IDM_TOPMOST, MF_UNCHECKED);
		CheckMenuItem(hMenuGb, IDM_TOPMOST, MF_UNCHECKED);
		gbCurrentIsTop = false;
	}
	if (gbFullManual) {
		CheckMenuItem(hMenu, IDM_FULL, MF_CHECKED);
		CheckMenuItem(hMenuGb, IDM_FULL, MF_CHECKED);
	}
	else {
		CheckMenuItem(hMenu, IDM_FULL, MF_UNCHECKED);
		CheckMenuItem(hMenuGb, IDM_FULL, MF_UNCHECKED);
	}

	//类属性
	LONG oldWndClsLong = GetClassLong(gbWindow, GCL_STYLE);
	oldWndClsLong ^= CS_NOCLOSE;
	SetClassLong(gbWindow, GCL_STYLE, oldWndClsLong);

	isGbFounded = true;
}
bool VIsInIllegalWindows(HWND hWnd) {
	list<HWND>::iterator testiterator;
	for (testiterator = jiYuWnds.begin(); testiterator != jiYuWnds.end(); testiterator++)
	{
		if ((*testiterator) == hWnd)
			return true;
	}
	return false;
}
bool VIsInIllegalCanSizeWindows(HWND hWnd) {
	list<HWND>::iterator testiterator;
	for (testiterator = jiYuWndCanSize.begin(); testiterator != jiYuWndCanSize.end(); testiterator++)
	{
		if ((*testiterator) == hWnd)
			return true;
	}
	return false;
}
bool VIsInIllegalWindowMessage(UINT msg) {
	return msg == WM_CLOSE || msg == WM_DESTROY;
}
void VBoom() {

	PostQuitMessage(0);
	ExitProcess(0);

	/*
	CHAR*P = 0;
	*P = 0;
	*/
}
void VSendMessageBack(LPCWSTR buff, HWND hDlg) {
	HWND receiveWindow = FindWindow(NULL, L"JiYu Trainer Main Window");
	if (receiveWindow) {
		COPYDATASTRUCT copyData = { 0 };
		copyData.lpData = (PVOID)buff;
		copyData.cbData = sizeof(WCHAR) * (wcslen(buff) + 1);
		SendMessageTimeout(receiveWindow, WM_COPYDATA, (WPARAM)hDlg, (LPARAM)&copyData, SMTO_NORMAL, 500, 0);
		SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_RB_FLASH_FAST, 0);
	}
	else if(!outlineEndJiy) {
		SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_MAIN_OUT, NULL);
		SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_RB_FAIL, 0);
		outlineEndJiy = true;
		if (forceKill) {
			if (_waccess_s(mainFullPath, 0) == 0)
				ShellExecute(hWndMsgCenter, L"open", mainFullPath, L"-r2", NULL, SW_NORMAL);
			VBoom();
		}
		else {
			VShowOutOfControlDlg();
			if (hWndOutOfControlConformRs == IDC_KILL) {
				PostQuitMessage(0);
				ExitProcess(0);
			}
			else if (hWndOutOfControlConformRs != IDCANCEL)
			{
				outlineEndJiy = false;
				if (_waccess_s(mainFullPath, 0) == 0)
					ShellExecute(hWndMsgCenter, L"open", mainFullPath, L"-r3", NULL, SW_NORMAL);
			}
		}
	}
}
void VManualQuit()
{
	VCloseFuckDrivers();
	VCloseMsgCenter();
	jiYuWnds.clear();
	loaded = false;
}

bool VIsWindowGbOrHp(HWND hWnd) {
	WCHAR text[50];
	GetWindowText(hWnd, text, 50);
	return VWindowTextIsGb(text) || StrEqual(text, L"BlackScreen Window");
}
bool VIsWindowGb(HWND hWnd) {
	WCHAR text[50]; GetWindowText(hWnd, text, 50);
	return VWindowTextIsGb(text);
}
bool VIsWindowHp(HWND hWnd) {
	WCHAR text[50];
	GetWindowText(hWnd, text, 50);
	return StrEqual(text, L"BlackScreen Window");
}
void VSwitchLockState(bool l) {
	if (isLocked != l) {
		isLocked = l;
		SendMessage(hWndMsgCenter, WM_COMMAND, IDC_SW_STATUS_LOCKED, NULL);
		WCHAR str[32]; swprintf_s(str, L"hkb:jyk:%s", l ? L"1" : L"0");
		VSendMessageBack(str, hWndMsgCenter);
		if (jiYuGBDeskRdWnd) {
			RECT rc, rcTool; GetWindowRect(jiYuGBDeskRdWnd, &rc);
			if (jiYuGBToolWnd) {
				GetClientRect(jiYuGBToolWnd, &rcTool);
				jiYuGBToolHeight = rcTool.bottom - rcTool.top;
			}
			raMoveWindow(jiYuGBDeskRdWnd, 0, isLocked ? 0 : jiYuGBToolHeight,
				rc.right - rc.left, rc.bottom - rc.top - (isLocked ? 0 : jiYuGBToolHeight), TRUE);
		}
	}
}
void VSwitchLockState(HWND hWnd, bool l) {
	if (l) {
		if (!isLocked && VIsWindowGbOrHp(hWnd)) {
			if (VIsWindowGb(hWnd))
				isGbFounded = true;
			VSwitchLockState(l);
		}
	}
	else {
		if (isLocked && VIsWindowGbOrHp(hWnd)) {
			if (VIsWindowGb(hWnd))
				isGbFounded = false;
			VSwitchLockState(l);
		}
	}
}
void VUnHookKeyBoard() {
	HMODULE hTDMaster = GetModuleHandle(L"LibTDMaster.dll");
	if (hTDMaster) {
		fnDoneHook DoneHook = (fnDoneHook)GetProcAddress(hTDMaster, "DoneHook");
		UnLockLocalInput = (fnUnLockLocalInput)GetProcAddress(hTDMaster, "UnLockLocalInput");
		if (UnLockLocalInput) {
			UnLockLocalInput();
			VOutPutStatus(L"[V] Unlocked Local Input. ");
		}
		else if (DoneHook) {
			DoneHook();
			VOutPutStatus(L"[V] Forece call DoneHook .");
		}
	}
}
void VGetStudentainVersion() 
{
	//Get main mod name
	WCHAR mainModName[MAX_PATH];
	GetModuleFileName(NULL, mainModName, MAX_PATH);
	WCHAR mainModVersion[64];

	VOutPutStatus(L"[V] VGetExeInfo %s ", mainModName);
	//获取极域版本
	if (VGetExeInfo(mainModName, L"ProductVersion", mainModVersion, 64)) {

		VOutPutStatus(L"[V] JiYu Version : %s", mainModVersion);

		if (wcscmp(mainModVersion, L"5.01 Baseline") == 0) {
			jiYuVersion = jiYuVersions::jiYuVersions40;
			VOutPutStatus(L"[V] 当前是：极域 V4 2010 专业版");
		}
		else if (wcscmp(mainModVersion, L"2.07 CMPC") == 0) {
			jiYuVersion = jiYuVersions::jiYuVersions402016HH;
			VOutPutStatus(L"[V] 当前是：极域 V6.0 2016 豪华版");
		}

	}else VOutPutStatus(L"[V] VGetExeInfo err !");

}
BOOL VGetExeInfo(LPWSTR strFilePath, LPCWSTR InfoItem, LPWSTR str, int maxCount)
{
	/*
	CompanyName
	FileDescription
	FileVersion
	InternalName
	LegalCopyright
	OriginalFilename
	ProductName
	ProductVersion
	Comments
	LegalTrademarks
	PrivateBuild
	SpecialBuild
	*/

	TCHAR   szResult[256];
	TCHAR   szGetName[256];
	LPWSTR  lpVersion = { 0 };        // String pointer to Item text
	DWORD   dwVerInfoSize;    // Size of version information block
	DWORD   dwVerHnd = 0;        // An 'ignored' parameter, always '0'
	UINT    uVersionLen;
	BOOL    bRetCode;

	dwVerInfoSize = GetFileVersionInfoSize(strFilePath, &dwVerHnd);
	if (dwVerInfoSize) {
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo = (LPSTR)GlobalLock(hMem);
		GetFileVersionInfo(strFilePath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);
		lstrcpy(szGetName, L"\\VarFileInfo\\Translation");
		uVersionLen = 0;
		lpVersion = NULL;
		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
			szGetName,
			(void **)&lpVersion,
			(UINT *)&uVersionLen);
		if (bRetCode && uVersionLen && lpVersion)
			wsprintf(szResult, L"%04x%04x", (WORD)(*((DWORD *)lpVersion)),
			(WORD)(*((DWORD *)lpVersion) >> 16));
		else lstrcpy(szResult, L"041904b0");
		wsprintf(szGetName, L"\\StringFileInfo\\%s\\", szResult);
		lstrcat(szGetName, InfoItem);
		uVersionLen = 0;
		lpVersion = NULL;
		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo,
			szGetName,
			(void **)&lpVersion,
			(UINT *)&uVersionLen);
		if (bRetCode && uVersionLen && lpVersion) {
			if (str) {
				wcscpy_s(str, maxCount, lpVersion);
				return TRUE;
			}
		}
	}
	return FALSE;
}

INT_PTR CALLBACK VShowOpConfirmWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	switch (message)
	{
	case WM_INITDIALOG: {

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONWARN)));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONWARN)));

		SetDlgItemText(hDlg, IDC_EDIT_OP_PATH, currOpCfPath);
		SetDlgItemText(hDlg, IDC_EDIT_OP_PARARM, currOpCfPararm);

		SetWindowLong(GetDlgItem(hDlg, IDNO), GWL_USERDATA, (LPARAM)30);
		SetTimer(hDlg, TIMER_COUNTDOWN_TICK, 1000, NULL);

		lResult = TRUE;
		break;
	}
	case WM_TIMER: {
		if (wParam == TIMER_COUNTDOWN_TICK) {
			int w = GetWindowLong(GetDlgItem(hDlg, IDNO), GWL_USERDATA);
			if (w > 0) {
				WCHAR str[18]; swprintf_s(str, L"拒绝 (%d)", w); 
				SetWindowLong(GetDlgItem(hDlg, IDNO), GWL_USERDATA, (LPARAM)(w - 1));
				SetDlgItemText(hDlg, IDNO, str);
			}
			else SendMessage(hDlg, WM_COMMAND, IDNO, NULL);
		}
		break;
	}
	case WM_COMMAND: {
		if (wParam == IDYES || wParam == IDNO || wParam == IDCANCEL) { 
			hWndOpConformRs = wParam; 
			lResult = wParam;
			DestroyWindow(hDlg);
		} 
		if (wParam == IDC_CHECK_BANDALL) {
			bandAllRunOp = IsDlgButtonChecked(hDlg, IDC_CHECK_BANDALL);
		}
		if (wParam == IDC_CHECK_ALOW) {
			allowNextRunOp = IsDlgButtonChecked(hDlg, IDC_CHECK_ALOW);
		}
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	}
	return lResult;
}
INT_PTR CALLBACK VShowOutOfControlWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;
	switch (message)
	{
	case WM_INITDIALOG: {

		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONWARN)));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICONWARN)));

		SetDlgItemText(hDlg, IDC_EDIT_OP_PATH, currOpCfPath);
		SetDlgItemText(hDlg, IDC_EDIT_OP_PARARM, currOpCfPararm);

		SetWindowLong(GetDlgItem(hDlg, IDC_RESTART), GWL_USERDATA, (LPARAM)7);
		SetTimer(hDlg, TIMER_COUNTDOWN_TICK, 1000, NULL);

		lResult = TRUE;
		break;
	}
	case WM_TIMER: {
		if (wParam == TIMER_COUNTDOWN_TICK) {
			int w = GetWindowLong(GetDlgItem(hDlg, IDC_RESTART), GWL_USERDATA);
			if (w > 0) {
				WCHAR str[18]; swprintf_s(str, L"重启主程序 (%d)", w);
				SetWindowLong(GetDlgItem(hDlg, IDC_RESTART), GWL_USERDATA, (LPARAM)(w - 1));
				SetDlgItemText(hDlg, IDC_RESTART, str);
			}
			else SendMessage(hDlg, WM_COMMAND, IDC_RESTART, NULL);
		}
		break;
	}
	case WM_COMMAND: {
		if (wParam == IDC_RESTART || wParam == IDNO || wParam == IDC_KILL) {
			
			hWndOutOfControlConformRs = wParam;
			lResult = wParam; 
			DestroyWindow(hDlg);
		}
		break;
	}
	case WM_DESTROY: {
		PostQuitMessage(0);
		break;
	}
	}
	return lResult;
}
std::wstring VGetExtension(std::wstring path)
{
	if (path == L"")
		return nullptr;

	size_t length = path.size();
	size_t num = length;
	while (--num >= 0)
	{
		wchar_t c = (path)[num];
		if (c == L'.')
		{
			if (num != length - 1)
			{
				return std::wstring(path.substr(num, length - num));
			}
			return nullptr;
		}
		else if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
		{
			break;
		}
	}
	return nullptr;
}
BOOL VShowOutOfControlDlg() {

	hWndOutOfControlConformRs = 0;

	HWND hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_OUTCTL), NULL, VShowOutOfControlWndProc);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return hWndOutOfControlConformRs == IDYES;
}
BOOL VShowOpConfirmDialog(LPCWSTR file, LPCWSTR pararm) {
	if (file) wcsncpy_s(currOpCfPath, file, MAX_PATH); else wcscpy_s(currOpCfPath, L"");
	if (pararm) wcsncpy_s(currOpCfPararm, pararm, MAX_PATH); else wcscpy_s(currOpCfPararm, L"");

	allowNextRunOp = false;
	hWndOpConformRs = 0;

	HWND hWnd = CreateDialog(hInst, MAKEINTRESOURCE(IDD_OPASK), NULL, VShowOpConfirmWndProc);
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (allowNextRunOp) {

		LPCWSTR exePath = NULL;
		if (!StrEmepty(file) && _waccess_s(file, 0) == 0) exePath = file;
		else if (!StrEmepty(pararm) && _waccess_s(pararm, 0) == 0) exePath = pararm;
		else if (!StrEmepty(file)) exePath = file;
		else if (!StrEmepty(pararm)) exePath = pararm;

		VAddOpToWhiteList(exePath);
	}

	return hWndOpConformRs == IDYES;
}	

LRESULT CALLBACK VCBTProc(int nCode, WPARAM wParam, LPARAM lParam) {
	return TRUE;
}

void VLoadOpWhiteList() {
	WCHAR mainWhiteListPath[MAX_PATH];
	wcscpy_s(mainWhiteListPath, mainFullPath);
	PathRemoveFileSpec(mainWhiteListPath);
	wcscat_s(mainWhiteListPath, L"\\JiYuKillerOpWhiteList.txt");

	if (PathFileExists(mainWhiteListPath)) {
		FILE * fp;
		int linenum;
		WCHAR * p, buf[1024];

	   _wfopen_s(&fp, mainWhiteListPath, L"r");
		if (fp == NULL) {
			return;
		}
		
		for (linenum = 1; fgetws(buf, sizeof(buf), fp) != NULL; ++linenum) {
			if ((p = wcschr(buf, L'\n')) == NULL) {
				p = buf + wcslen(buf);
			}
			if (p > buf && p[-1] == L'\r') {
				--p;
			}
			*p = L'\0';
			for (p = buf; *p != L'\0' && isspace((int)*p); ++p) {
				;
			}
			if (*p == L'\0' || *p == L'#') {
				continue;
			}

			VAddOpToWhiteList(p);
		}
		fclose(fp);
	}
}
void VAddOpToWhiteList(const wchar_t* cmd) {
	wstring wcmd = wstring(cmd);
	wcmd = StringHlp::StrLoW(wcmd.data());
	if (!VIsOpInWhiteList(wcmd.c_str()))
		runOPWhiteList.push_back(wcmd);
}
bool VIsOpInWhiteList(const wchar_t* cmd) {
	wstring wcmd = wstring(cmd);
	wcmd = StringHlp::StrLoW(wcmd.data());
	std::list<std::wstring>::iterator iter;
	for (iter = runOPWhiteList.begin(); iter != runOPWhiteList.end(); iter++)
	{
		if ((*iter) == wcmd)
			return true;
	}
	return false;
}

void VInstallHooks(VirusMode mode) {

	//Mhook_SetHook
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
	HMODULE hShell32 = GetModuleHandle(L"shell32.dll");
	HMODULE hDwmApi = GetModuleHandle(L"dwmapi.dll");
	HMODULE hGdi32 = GetModuleHandle(L"gdi32.dll");
	HMODULE hLibJPEG20 = GetModuleHandle(L"LibJPEG20.dll");
	HMODULE hLibAVCodec52 = GetModuleHandle(L"LibAVCodec52.dll");
	HMODULE hFltLib = GetModuleHandle(L"FltLib.dll");


	raSetWindowPos = (fnSetWindowPos)GetProcAddress(hUser32, "SetWindowPos");
	raMoveWindow = (fnMoveWindow)GetProcAddress(hUser32, "MoveWindow");
	raSetForegroundWindow = (fnSetForegroundWindow)GetProcAddress(hUser32, "SetForegroundWindow");
	faSetWindowsHookExA = (fnSetWindowsHookExA)GetProcAddress(hUser32, "SetWindowsHookExA");
	faDeferWindowPos = (fnDeferWindowPos)GetProcAddress(hUser32, "DeferWindowPos");
	faBringWindowToTop = (fnBringWindowToTop)GetProcAddress(hUser32, "BringWindowToTop");
	faSendInput = (fnSendInput)GetProcAddress(hUser32, "SendInput");
	faChangeDisplaySettingsW = (fnChangeDisplaySettingsW)GetProcAddress(hUser32, "ChangeDisplaySettingsW");
	famouse_event = (fnmouse_event)GetProcAddress(hUser32, "mouse_event");
	faSetWindowLongA = (fnSetWindowLongA)GetProcAddress(hUser32, "SetWindowLongA");
	faSetWindowLongW = (fnSetWindowLongW)GetProcAddress(hUser32, "SetWindowLongW");
	faShowWindow = (fnShowWindow)GetProcAddress(hUser32, "ShowWindow");
	faCallNextHookEx = (fnCallNextHookEx)GetProcAddress(hUser32, "CallNextHookEx");
	faUnhookWindowsHookEx = (fnUnhookWindowsHookEx)GetProcAddress(hUser32, "UnhookWindowsHookEx");
	faGetDesktopWindow = (fnGetDesktopWindow)GetProcAddress(hUser32, "GetDesktopWindow");
	faGetWindowDC = (fnGetWindowDC)GetProcAddress(hUser32, "GetDesktopWindow");
	faGetForegroundWindow = (fnGetForegroundWindow)GetProcAddress(hUser32, "GetForegroundWindow");
	faEnableMenuItem = (fnEnableMenuItem)GetProcAddress(hUser32, "EnableMenuItem");
	faSetClassLongA = (fnSetClassLongA)GetProcAddress(hUser32, "SetClassLongA");
	faSetClassLongW = (fnSetClassLongW)GetProcAddress(hUser32, "SetClassLongW");

	faPostMessageW = (fnPostMessageW)GetProcAddress(hUser32, "PostMessageW");
	faSendMessageW = (fnSendMessageW)GetProcAddress(hUser32, "SendMessageW");
	faTerminateProcess = (fnTerminateProcess)GetProcAddress(hKernel32, "TerminateProcess");

	raDeviceIoControl = (fnDeviceIoControl)GetProcAddress(hKernel32, "DeviceIoControl");
	faCreateFileA = (fnCreateFileA)GetProcAddress(hKernel32, "CreateFileA");
	faCreateFileW = (fnCreateFileW)GetProcAddress(hKernel32, "CreateFileW");
	faCreateProcessW = (fnCreateProcessW)GetProcAddress(hKernel32, "CreateProcessW");
	faCreateProcessA = (fnCreateProcessA)GetProcAddress(hKernel32, "CreateProcessA");
	faWinExec = (fnWinExec)GetProcAddress(hKernel32, "WinExec");

	faExitWindowsEx = (fnExitWindowsEx)GetProcAddress(hUser32, "ExitWindowsEx");
	//faShellExecuteW = (fnShellExecuteW)GetProcAddress(hShell32, "ShellExecuteW");
	//faShellExecuteExW = (fnShellExecuteExW)GetProcAddress(hShell32, "ShellExecuteExW");

	faCreateDCW = (fnCreateDCW)GetProcAddress(hGdi32, "CreateDCW");

	if (hDwmApi) faDwmEnableComposition = (fnDwmEnableComposition)GetProcAddress(hDwmApi, "DwmEnableComposition");
	if (hLibJPEG20) faEncodeToJPEGBuffer = (fnEncodeToJPEGBuffer)GetProcAddress(hLibJPEG20, "EncodeToJPEGBuffer");
	if (hFltLib) faFilterConnectCommunicationPort = (fnFilterConnectCommunicationPort)GetProcAddress(hFltLib, "FilterConnectCommunicationPort");

	//HMODULE hTDDesk2 = GetModuleHandle(L"libtddesk2.dll");
	//if (hTDDesk2) {
	//	faTDDeskCreateInstance = (fnTDDeskCreateInstance)GetProcAddress(hTDDesk2, "TDDeskCreateInstance");
	//}

	if (mode == VirusModeHook) {

		//HMODULE hTDMaster = GetModuleHandle(L"libTDMaster.dll");
		//UnHookLocalInput = (fnUnHookLocalInput)GetProcAddress(hTDMaster, "UnHookLocalInput");
		//if (UnHookLocalInput) UnHookLocalInput();

		VOutPutStatus(L"[D] Install hook in mode VirusModeHook");

		hk1 = Mhook_SetHook((PVOID*)&raSetWindowPos, hkSetWindowPos);
		hk2 = Mhook_SetHook((PVOID*)&raMoveWindow, hkMoveWindow);
		hk3 = Mhook_SetHook((PVOID*)&raSetForegroundWindow, hkSetForegroundWindow);
		hk4 = Mhook_SetHook((PVOID*)&faBringWindowToTop, hkBringWindowToTop);
		hk5 = Mhook_SetHook((PVOID*)&raDeviceIoControl, hkDeviceIoControl);
		hk6 = Mhook_SetHook((PVOID*)&faCreateFileA, hkCreateFileA);
		hk7 = Mhook_SetHook((PVOID*)&faCreateFileW, hkCreateFileW);
		hk8 = Mhook_SetHook((PVOID*)&faSetWindowsHookExA, hkSetWindowsHookExA);
		hk9 = Mhook_SetHook((PVOID*)&faDeferWindowPos, hkDeferWindowPos);
		hk10 = Mhook_SetHook((PVOID*)&faSendInput, hkSendInput);
		hk11 = Mhook_SetHook((PVOID*)&famouse_event, hkmouse_event);

		//if(faTDDeskCreateInstance) 
		//	hk15 = Mhook_SetHook((PVOID*)&faTDDeskCreateInstance, hkTDDeskCreateInstance);

		hk16 = Mhook_SetHook((PVOID*)&faSetWindowLongA, hkSetWindowLongA);
		hk17 = Mhook_SetHook((PVOID*)&faSetWindowLongW, hkSetWindowLongW);
		hk18 = Mhook_SetHook((PVOID*)&faShowWindow, hkShowWindow);
		hk19 = Mhook_SetHook((PVOID*)&faExitWindowsEx, hkExitWindowsEx);

		//hk20 = Mhook_SetHook((PVOID*)&faShellExecuteW, hkShellExecuteW);
		//if(faShellExecuteExW) hk21 = Mhook_SetHook((PVOID*)&faShellExecuteExW, hkShellExecuteExW);

		hk22 = Mhook_SetHook((PVOID*)&faCreateProcessW, hkCreateProcessW);

		if (faDwmEnableComposition) hk23 = Mhook_SetHook((PVOID*)&faDwmEnableComposition, hkDwmEnableComposition);

		hk24 = Mhook_SetHook((PVOID*)&faWinExec, hkWinExec);
		hk25 = Mhook_SetHook((PVOID*)&faCallNextHookEx, hkCallNextHookEx);
		hk26 = Mhook_SetHook((PVOID*)&faGetWindowDC, hkGetWindowDC);
		hk27 = Mhook_SetHook((PVOID*)&faGetDesktopWindow, hkGetDesktopWindow);

		if (faEncodeToJPEGBuffer) hk28 = Mhook_SetHook((PVOID*)&faEncodeToJPEGBuffer, hkEncodeToJPEGBuffer);
		hk29 = Mhook_SetHook((PVOID*)&faUnhookWindowsHookEx, hkUnhookWindowsHookEx);

		hk30 = Mhook_SetHook((PVOID*)&faCreateProcessA, hkCreateProcessA);
		hk31 = Mhook_SetHook((PVOID*)&faGetForegroundWindow, hkGetForegroundWindow);

		hk33 = Mhook_SetHook((PVOID*)&faCreateDCW, hkCreateDCW);
		hk34 = Mhook_SetHook((PVOID*)&faEnableMenuItem, hkEnableMenuItem);

		hk35 = Mhook_SetHook((PVOID*)&faSetClassLongA, hkSetClassLongA);
		hk36 = Mhook_SetHook((PVOID*)&faSetClassLongW, hkSetClassLongW);

		hk37 = Mhook_SetHook((PVOID*)&faPostMessageW, hkPostMessageW);
		hk38 = Mhook_SetHook((PVOID*)&faSendMessageW, hkSendMessageW);
		hk39 = Mhook_SetHook((PVOID*)&faTerminateProcess, hkTerminateProcess);

		if(faFilterConnectCommunicationPort) hk40 = Mhook_SetHook((PVOID*)&faFilterConnectCommunicationPort, hkFilterConnectCommunicationPort);
	}
	if (mode == VirusModeMaster) {

		g_hhook = SetWindowsHookExA(WH_CBT, VCBTProc, hInst, GetCurrentThreadId());
		
		VOutPutStatus(L"[D] Install hook in mode VirusModeMaster");

		hk8 = Mhook_SetHook((PVOID*)&faSetWindowsHookExA, hkSetWindowsHookExA);
		hk19 = Mhook_SetHook((PVOID*)&faExitWindowsEx, hkExitWindowsEx);
		hk22 = Mhook_SetHook((PVOID*)&faCreateProcessW, hkCreateProcessW);
	}
}
void VUnInstallHooks() {

	if (hk1) Mhook_Unhook((PVOID*)&raSetWindowPos);
	if (hk2) Mhook_Unhook((PVOID*)&raMoveWindow);
	if (hk3) Mhook_Unhook((PVOID*)&raSetForegroundWindow);
	if (hk4) Mhook_Unhook((PVOID*)&faBringWindowToTop);
	if (hk5) Mhook_Unhook((PVOID*)&raDeviceIoControl);
	if (hk6) Mhook_Unhook((PVOID*)&faCreateFileA);
	if (hk7) Mhook_Unhook((PVOID*)&faCreateFileW);
	if (hk8) Mhook_Unhook((PVOID*)&faSetWindowsHookExA);
	if (hk9) Mhook_Unhook((PVOID*)&faDeferWindowPos);
	if (hk10) Mhook_Unhook((PVOID*)&faSendInput);
	if (hk11) Mhook_Unhook((PVOID*)&famouse_event);
	//if (hk12) Mhook_Unhook((PVOID*)&faChangeDisplaySettingsW);
	//if (hk13) Mhook_Unhook((PVOID*)&faOpenDesktopA);
	//if (hk14) Mhook_Unhook((PVOID*)&faOpenInputDesktop);
	//if (hk15) Mhook_Unhook((PVOID*)&faTDDeskCreateInstance);
	if (hk16) Mhook_Unhook((PVOID*)&faSetWindowLongA);
	if (hk17) Mhook_Unhook((PVOID*)&faSetWindowLongW);
	if (hk18) Mhook_Unhook((PVOID*)&faShowWindow);

	if (hk19) Mhook_Unhook((PVOID*)&faExitWindowsEx);
	if (hk20) Mhook_Unhook((PVOID*)&faShellExecuteW);
	if (hk21) Mhook_Unhook((PVOID*)&faShellExecuteExW);
	if (hk22) Mhook_Unhook((PVOID*)&faCreateProcessW);
	if (hk23) Mhook_Unhook((PVOID*)&faDwmEnableComposition);
	if (hk24) Mhook_Unhook((PVOID*)&faWinExec);
	if (hk25) Mhook_Unhook((PVOID*)&faCallNextHookEx);
	if (hk26) Mhook_Unhook((PVOID*)&faGetWindowDC);
	if (hk27) Mhook_Unhook((PVOID*)&faGetDesktopWindow);
	if (hk28) Mhook_Unhook((PVOID*)&faEncodeToJPEGBuffer);

	if (hk30) Mhook_Unhook((PVOID*)&faCreateProcessA);
	if (hk31) Mhook_Unhook((PVOID*)&faGetForegroundWindow);

	if (hk33) Mhook_Unhook((PVOID*)&faCreateDCW);
	if (hk34) Mhook_Unhook((PVOID*)&faEnableMenuItem);
	if (hk35) Mhook_Unhook((PVOID*)&faSetClassLongA);
	if (hk36) Mhook_Unhook((PVOID*)&faSetClassLongW);
	if (hk37) Mhook_Unhook((PVOID*)&faPostMessageW);
	if (hk38) Mhook_Unhook((PVOID*)&faSendMessageW);
	if (hk39) Mhook_Unhook((PVOID*)&faTerminateProcess);
	if (hk40) Mhook_Unhook((PVOID*)&faFilterConnectCommunicationPort);

	VOutPutStatus(L"[D] UnInstall all hook");

	if (g_hhook) {
		UnhookWindowsHookEx(g_hhook);
		g_hhook = NULL;
	}
}

//Fuck driver devices
HANDLE hDeviceTDKeybd = NULL;
HANDLE hDeviceTDProcHook = NULL;
HANDLE hDeviceTDNetFilter = NULL;

void VOpenFuckDrivers() {

	hDeviceTDNetFilter = CreateFileW(L"\\\\.\\TDNetFilter", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hDeviceTDKeybd = CreateFileW(L"\\\\.\\TDKeybd", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hDeviceTDProcHook = CreateFileW(L"\\\\.\\TDProcHook", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}
void VCloseFuckDrivers()
{
	if (hDeviceTDNetFilter) CloseHandle(hDeviceTDNetFilter);
	if (hDeviceTDProcHook) CloseHandle(hDeviceTDProcHook);
	if (hDeviceTDKeybd) CloseHandle(hDeviceTDKeybd);
}

//Fake hook for TDMaster


HHOOK hMouseHook;
HHOOK hKeyboardHook;

HOOKPROC lpMouseHookfn = NULL;
HOOKPROC lpKeyboardHookfn = NULL;

LRESULT WINAPI LowLevelMouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = faCallNextHookEx(hMouseHook, nCode, wParam, lParam);
	if (lpMouseHookfn)
		lpMouseHookfn(nCode, wParam, lParam);
	return result;
}
LRESULT WINAPI LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = faCallNextHookEx(hKeyboardHook, nCode, wParam, lParam);
	if (lpKeyboardHookfn)
		lpKeyboardHookfn(nCode, wParam, lParam);
	return result;
}

//Hook stubs

BOOL WINAPI hkSetForegroundWindow(HWND hWnd)
{
		return TRUE;
	return raSetForegroundWindow(hWnd);
}
BOOL WINAPI hkSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (loaded) 
	{
		if ((uFlags & SWP_NOSIZE) == 0)
		{
			if ((x == 0 && y == 0 && cx == screenWidth && cy == screenHeight))
			{
				VSwitchLockState(hWnd, TRUE);
				if ((fakeFull) && VIsWindowGbOrHp(hWnd))
					return raSetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
				if (VIsInIllegalCanSizeWindows(hWnd))
				{
					jiYuWndCanSize.remove(hWnd);
					raSetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags | SWP_NOZORDER);
					SendMessage(hWnd, WM_SIZE, NULL, NULL);
					return TRUE;
				}
				return TRUE;
			}
			else {
				VSwitchLockState(hWnd, FALSE);
				if (fakeFull)
					return raSetWindowPos(hWnd, HWND_NOTOPMOST, x, y, cx, cy, uFlags);
			}
		}

		if (VIsInIllegalWindows(hWnd)) 
			return TRUE;
	}
	return raSetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}
HDWP WINAPI hkDeferWindowPos(HDWP hWinPosInfo, HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (loaded)
	{
		if ((uFlags & SWP_NOSIZE) == 0)
		{
			if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) {
				VSwitchLockState(hWnd, TRUE);
				if (fakeFull && VIsWindowGbOrHp(hWnd))
					return faDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
				if (VIsInIllegalCanSizeWindows(hWnd)) {
					jiYuWndCanSize.remove(hWnd);
					HDWP rs = faDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags | SWP_NOZORDER);
					SendMessage(hWnd, WM_SIZE, NULL, NULL);
					return rs;
				}
				return NULL;
			}
			else {
				VSwitchLockState(hWnd, FALSE);
				if (fakeFull) {
					return faDeferWindowPos(hWinPosInfo, hWnd, HWND_NOTOPMOST, x, y, cx, cy, uFlags);
				}

			}
		}

		if (VIsInIllegalWindows(hWnd))
			return NULL;
	}
	return faDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags);
}
BOOL WINAPI hkMoveWindow(HWND hWnd, int x, int y, int cx, int cy, BOOL bRepaint)
{
	if (loaded)
	{
		if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) {
			VSwitchLockState(hWnd, TRUE);
			if (fakeFull && VIsWindowGbOrHp(hWnd))
				return raMoveWindow(hWnd, x, y, cx, cy, bRepaint);
			if (VIsInIllegalCanSizeWindows(hWnd)) {
				jiYuWndCanSize.remove(hWnd);
				return raMoveWindow(hWnd,  x, y, cx, cy, bRepaint);
			}
			return TRUE;
		}
		else {
			VSwitchLockState(hWnd, FALSE);
			if (fakeFull) 
				return raMoveWindow(hWnd, x, y, cx, cy, bRepaint);
		}
		if (VIsInIllegalWindows(hWnd))
			return TRUE;
	} 
	return raMoveWindow(hWnd, x, y, cx, cy, bRepaint);
}
BOOL WINAPI hkDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode, LPVOID lpInBuffer, DWORD nInBufferSize, LPVOID lpOutBuffer, DWORD nOutBufferSize, LPDWORD lpBytesReturned,  LPOVERLAPPED lpOverlapped) {
	if (hDeviceTDKeybd) {
		if (hDevice == hDeviceTDKeybd) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	if (hDeviceTDProcHook) {
		if (hDevice == hDeviceTDProcHook) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	if (hDeviceTDNetFilter) {
		if (hDevice == hDeviceTDNetFilter) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	return raDeviceIoControl(hDevice, dwIoControlCode, lpInBuffer, nInBufferSize, lpOutBuffer, nOutBufferSize, lpBytesReturned, lpOverlapped);
}
HANDLE WINAPI hkCreateFileA(LPCSTR lpFileName,  DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile ) {
	if (loaded)
	{
		if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDKeybd")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDProcHook")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		if (StringHlp::StrEqualA(lpFileName, "\\\\.\\TDNetFilter")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}
	return faCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HANDLE WINAPI hkCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess,  DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	if (loaded)
	{
		if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDKeybd")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDProcHook")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
		if (StringHlp::StrEqualW(lpFileName, L"\\\\.\\TDNetFilter")) {
			SetLastError(ERROR_ACCESS_DENIED);
			return INVALID_HANDLE_VALUE;
		}
	}
	return faCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
VOID WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData,  ULONG_PTR dwExtraInfo)
{
	if (allowControl) famouse_event(dwFlags, dx, dy, dwData, dwExtraInfo);
}
UINT WINAPI hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
{
	if (!allowControl && loaded)
		return cInputs;
	return faSendInput(cInputs, pInputs, cbSize);
}
BOOL WINAPI hkBringWindowToTop(HWND hWnd)
{
	if (loaded)
	{
		if (VIsInIllegalWindows(hWnd))
			return TRUE;
	}
	return faBringWindowToTop(hWnd);
}
BOOL WINAPI hkExitWindowsEx(UINT uFlags, DWORD dwReason) {
	const wchar_t* op = L"";
	if((uFlags & EWX_POWEROFF) == EWX_POWEROFF) op = L"关闭您的计算机";
	else if ((uFlags & EWX_REBOOT) == EWX_REBOOT) op = L"重启您的计算机";
	else if ((uFlags & EWX_LOGOFF) == EWX_LOGOFF) op = L"注销您的计算机";
	wchar_t str[37];
	swprintf_s(str, L"极域电子教室试图%s，是否允许极域继续操作？", op);
	if (MessageBox(NULL, str, L"JiYu Killer 防护警告", MB_ICONEXCLAMATION | MB_YESNO) == IDYES) 
		return faExitWindowsEx(uFlags, dwReason);
	SetLastError(ERROR_ACCESS_DENIED);
	return FALSE;
}
HINSTANCE WINAPI hkShellExecuteW(HWND hwnd, LPCWSTR lpOperation, LPCWSTR lpFile, LPCWSTR lpParameters, LPCWSTR lpDirectory, INT nShowCmd) {

	/*if (StrEqual(lpOperation, L"open") || StrEqual(lpOperation, L"runas")) {
		if (VGetExtension(wstring(lpFile)) == L".exe") {
			if (bandAllRunOp || !VShowOpConfirmDialog(lpFile, lpParameters))
				return hInst;
		}
	}*/
	return faShellExecuteW(hwnd, lpOperation, lpFile, lpParameters, lpDirectory, nShowCmd);
}
BOOL WINAPI hkShellExecuteExW(SHELLEXECUTEINFOW *pExecInfo) {

	/*if (StrEqual(pExecInfo->lpFile, L"open") || StrEqual(pExecInfo->lpFile, L"runas")) {
		if (bandAllRunOp || VGetExtension(wstring(pExecInfo->lpFile)) == L".exe") {
			if (!VShowOpConfirmDialog(pExecInfo->lpFile, pExecInfo->lpParameters)) 
				return TRUE;
		}
	}*/
	return faShellExecuteExW(pExecInfo);

}
BOOL WINAPI hkCreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	bool canContinue = true;
	if(allowAllRunOp)
		return faCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

	LPCWSTR exePath = NULL;
	if (!StrEmepty(lpApplicationName) && _waccess_s(lpApplicationName, 0) == 0) exePath = lpApplicationName;
	else if (!StrEmepty(lpCommandLine) && _waccess_s(lpCommandLine, 0) == 0) exePath = lpCommandLine;
	else if(!StrEmepty(lpApplicationName)) exePath = lpApplicationName;
	else if (!StrEmepty(lpCommandLine)) exePath = lpCommandLine;

	LPCWSTR lowStr = StringHlp::StrLoW(exePath);
	if (StringHlp::StrContainsW(lowStr, L"shutdown.exe", NULL)) {
		if (MessageBox(NULL, L"极域电子教室试图关机或重启，是否允许极域继续操作？", L"JiYu Killer 防护警告", MB_ICONEXCLAMATION | MB_YESNO) == IDNO) canContinue = false;
	} 
	else if (StringHlp::StrContainsW(lowStr, L"jiyutrainer.exe", NULL) 
		|| StringHlp::StrContainsW(lowStr, L"explorer", NULL)
		|| StringHlp::StrContainsW(lowStr, L"sogouinput", NULL)
		|| StringHlp::StrContainsW(lowStr, L"ime", NULL)
		|| StringHlp::StrContainsW(lowStr, L"baidupinyin", NULL)
		|| VIsOpInWhiteList(lowStr)) canContinue = true;
	else if (StringHlp::StrContainsW(lowStr, L"tdchalk.exe", NULL))
		canContinue = false;
	else if (bandAllRunOp || !VShowOpConfirmDialog(lpApplicationName, lpCommandLine)) canContinue = false;

	if(canContinue) return faCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	else return TRUE;
}
BOOL WINAPI hkCreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, LPSTARTUPINFOA lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation)
{
	bool canContinue = true;
	if (allowAllRunOp)
		return faCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);

	LPCWSTR lpApplicationNameW = StringHlp::AnsiToUnicode(lpApplicationName);
	LPCWSTR lpCommandLineW = StringHlp::AnsiToUnicode(lpCommandLine);

	LPCSTR exePath = NULL;
	if (!StringHlp::StrEmeptyA(lpApplicationName) && _waccess_s(lpApplicationNameW, 0) == 0) exePath = lpApplicationName;
	else if (!StringHlp::StrEmeptyA(lpCommandLine) && _waccess_s(lpCommandLineW, 0) == 0) exePath = lpCommandLine;
	else if (!StringHlp::StrEmeptyA(lpApplicationName)) exePath = lpApplicationName;
	else if (!StringHlp::StrEmeptyA(lpCommandLine)) exePath = lpCommandLine;

	LPCSTR lowStr = StringHlp::StrLoA(exePath);
	LPCWSTR lowStrW = StringHlp::AnsiToUnicode(lowStr);

	if (StringHlp::StrContainsA(lowStr, "shutdown.exe", NULL)) {
		if (MessageBox(NULL, L"极域电子教室试图关机或重启，是否允许极域继续操作？", L"JiYu Killer 防护警告", MB_ICONEXCLAMATION | MB_YESNO) == IDNO) canContinue = false;
	}
	else if (StringHlp::StrContainsA(lowStr, "jiyutrainer.exe", NULL)
		|| StringHlp::StrContainsA(lowStr, "explorer", NULL)
		|| StringHlp::StrContainsA(lowStr, "sogouinput", NULL)
		|| StringHlp::StrContainsA(lowStr, "ime", NULL)
		|| StringHlp::StrContainsA(lowStr, "baidupinyin", NULL)
		|| VIsOpInWhiteList(lowStrW)) canContinue = true;
	else if (StringHlp::StrContainsA(lowStr, "tdchalk.exe", NULL))
		canContinue = false;
	else if (bandAllRunOp || !VShowOpConfirmDialog(lpApplicationNameW, lpCommandLineW)) canContinue = false;

	StringHlp::FreeStringPtr(lpApplicationNameW);
	StringHlp::FreeStringPtr(lpCommandLineW);
	StringHlp::FreeStringPtr(lowStrW);

	if (canContinue) return faCreateProcessA(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	else return TRUE;
}
UINT WINAPI hkWinExec(LPCSTR lpCmdLine, UINT uCmdShow) {
	bool canContinue = true;
	if (allowAllRunOp)
		return faWinExec(lpCmdLine, uCmdShow);

	LPCWSTR uniStr = StringHlp::AnsiToUnicode(lpCmdLine);
	LPCWSTR lowStr = StringHlp::StrLoW(uniStr);
	if (StringHlp::StrContainsW(lowStr, L"shutdown.exe", NULL)) {
		if (MessageBox(NULL, L"极域电子教室试图关机或重启，是否允许极域继续操作？", L"JiYu Killer 防护警告", MB_ICONEXCLAMATION | MB_YESNO) == IDNO) canContinue = false;
	}
	else if (StringHlp::StrContainsW(lowStr, L"jiyutrainer.exe", NULL)
		|| StringHlp::StrContainsW(lowStr, L"explorer", NULL)
		|| StringHlp::StrContainsW(lowStr, L"sogouinput", NULL)
		|| StringHlp::StrContainsW(lowStr, L"baidupinyin", NULL)
		|| StringHlp::StrContainsW(lowStr, L"ime", NULL)
		|| VIsOpInWhiteList(lowStr)) canContinue = true;
	else if (StringHlp::StrContainsW(lowStr, L"tdchalk.exe", NULL)) canContinue = false;
	else if (bandAllRunOp || !VShowOpConfirmDialog(uniStr, L"")) canContinue = false;

	delete uniStr;
	if (canContinue) return faWinExec(lpCmdLine, uCmdShow);
	else return 32;
}
LONG WINAPI hkChangeDisplaySettingsW(DEVMODEW* lpDevMode, DWORD dwFlags)
{
	return DISP_CHANGE_SUCCESSFUL;
}
HDESK WINAPI hkOpenDesktopA(LPCSTR lpszDesktop, DWORD dwFlags, BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}
HDESK WINAPI hkOpenInputDesktop(DWORD dwFlags,BOOL fInherit, ACCESS_MASK dwDesiredAccess)
{
	SetLastError(ERROR_ACCESS_DENIED);
	return NULL;
}
LONG WINAPI hkSetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded) {
		if (hWnd == mainWindow) return 0;
		if (fakeFull && VIsWindowGbOrHp(hWnd))
			return faSetWindowLongA(hWnd, nIndex, dwNewLong);
		if (VIsInIllegalWindows(hWnd))
			return GetWindowLongA(hWnd, nIndex);
	}
	return faSetWindowLongA(hWnd, nIndex, dwNewLong);
}
LONG WINAPI hkSetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded) {
		if (hWnd == mainWindow) return 0;
		if(fakeFull && VIsWindowGbOrHp(hWnd))
			return faSetWindowLongW(hWnd, nIndex, dwNewLong);
		if (VIsInIllegalWindows(hWnd))
			return GetWindowLongW(hWnd, nIndex);
	}
	return faSetWindowLongW(hWnd, nIndex, dwNewLong);
}
BOOL WINAPI hkShowWindow(HWND hWnd, int nCmdShow)
{
	if (VIsWindowGbOrHp(hWnd)) 
	{
		if (nCmdShow == SW_SHOW || nCmdShow == SW_SHOWNORMAL  || nCmdShow == SW_MAXIMIZE || nCmdShow == SW_MAX || nCmdShow == SW_SHOWNA
			|| nCmdShow ==  SW_SHOWMAXIMIZED || nCmdShow == SW_SHOWNOACTIVATE || nCmdShow == SW_SHOWDEFAULT) {
			if(VIsWindowHp(hWnd))
				VSwitchLockState(hWnd, TRUE);
			if (VIsWindowGb(hWnd)) isGbFounded = true;
			VSendMessageBack(L"hkb:immck", hWndMsgCenter);
		}
		else if (nCmdShow == SW_HIDE) {
			if (VIsWindowGb(hWnd)) isGbFounded = false;
			VSwitchLockState(hWnd, FALSE);
		}
	}
	return faShowWindow(hWnd, nCmdShow);
}
HRESULT WINAPI hkDwmEnableComposition(UINT uCompositionAction)
{
	if(!allowMonitor && uCompositionAction == DWM_EC_DISABLECOMPOSITION)
		return  S_OK;
	return faDwmEnableComposition(uCompositionAction);
}
HHOOK WINAPI hkSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
	if (loaded)
	{
		if (idHook == WH_CBT) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
		if (idHook == WH_MOUSE_LL || idHook == WH_MOUSE || idHook == WH_KEYBOARD_LL) {
			if (hmod == hLibTDMaster) {
				switch (idHook)
				{
				case WH_MOUSE_LL:
					lpMouseHookfn = lpfn;
					hMouseHook = faSetWindowsHookExA(idHook, LowLevelMouseProc, hmod, dwThreadId);
					VOutPutStatus(L"[K] hMouseHook WH_MOUSE_LL hooked 0x%08x", hMouseHook);
					VOutPutStatus(L"[K] lpfn : 0x%08x hmod :  0x%08x dwThreadId : %d", hMouseHook, hmod, dwThreadId);
					return (HHOOK)INVALID_HHOOK_MOUSE;
				case WH_MOUSE:
					lpMouseHookfn = lpfn;
					hMouseHook = faSetWindowsHookExA(idHook, LowLevelMouseProc, hmod, dwThreadId);
					VOutPutStatus(L"[K] hMouseHook WH_MOUSE hooked 0x%08x", hMouseHook);
					VOutPutStatus(L"[K] lpfn : 0x%08x hmod :  0x%08x dwThreadId : %d", hMouseHook, hmod, dwThreadId);
					return (HHOOK)INVALID_HHOOK_MOUSE;
				case WH_KEYBOARD_LL:
					lpKeyboardHookfn = lpfn;
					hKeyboardHook = faSetWindowsHookExA(idHook, LowLevelKeyboardProc, hmod, dwThreadId);
					VOutPutStatus(L"[K] hMouseHook WH_KEYBOARD_LL hooked 0x%08x", hKeyboardHook);
					VOutPutStatus(L"[K] lpfn : 0x%08x hmod :  0x%08x dwThreadId : %d", hMouseHook, hmod, dwThreadId);
					return (HHOOK)INVALID_HHOOK_KEYBOARD;
				}
			}
			return FALSE;
		}
	}
	return faSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
}
LRESULT WINAPI hkCallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam) 
{
	if (hhk == (HHOOK)INVALID_HHOOK_MOUSE) return 0;
	if (hhk == (HHOOK)INVALID_HHOOK_KEYBOARD) return 0;
	return faCallNextHookEx(hhk, nCode, wParam, lParam);
}
BOOL WINAPI hkUnhookWindowsHookEx(HHOOK hhk)
{
	if (hhk == (HHOOK)INVALID_HHOOK_MOUSE) {
		BOOL rs = faUnhookWindowsHookEx(hMouseHook);
		hMouseHook = NULL;
		VOutPutStatus(L"[K] hMouseHook Unkooked !");
		return rs;
	}
	if (hhk == (HHOOK)INVALID_HHOOK_KEYBOARD) {
		BOOL rs = faUnhookWindowsHookEx(hKeyboardHook);
		hMouseHook = hKeyboardHook;
		VOutPutStatus(L"[K] hKeyboardHook Unkooked !");
		return rs;
	}
	return faUnhookWindowsHookEx(hhk);
}
BOOL WINAPI hkPostMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (loaded && hWnd == mainWindow) return 0;
	if (loaded && VIsInIllegalWindowMessage(Msg)  && ProhibitCloseWindow) {
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		if(pid != currentPid)
			return 0;
	}
	return faPostMessageW(hWnd, Msg, wParam, lParam);
}
LRESULT WINAPI hkSendMessageW(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (loaded && hWnd == mainWindow) return 0;
	if (loaded && VIsInIllegalWindowMessage(Msg) && ProhibitCloseWindow) {
		DWORD pid;
		GetWindowThreadProcessId(hWnd, &pid);
		if (pid != currentPid)
			return 0;
	}
	return faSendMessageW(hWnd, Msg,  wParam,  lParam);
}
BOOL WINAPI hkTerminateProcess(HANDLE hProcess, UINT uExitCode)
{
	if (loaded && ProhibitKillProcess  && hProcess != GetCurrentProcess()) {
		SetLastError(ERROR_ACCESS_DENIED);
		return FALSE;
	}
	return faTerminateProcess(hProcess, uExitCode);
}
HRESULT WINAPI hkFilterConnectCommunicationPort(LPCWSTR lpPortName, DWORD dwOptions, LPCVOID lpContext, WORD wSizeOfContext, LPSECURITY_ATTRIBUTES lpSecurityAttributes, HANDLE * hPort)
{
	if (loaded && StrEqual(lpPortName, L"\\TDFileFilterPort"))
		return S_FALSE;
	return faFilterConnectCommunicationPort(lpPortName, dwOptions, lpContext, wSizeOfContext, lpSecurityAttributes,  hPort);
}
HWND WINAPI hkGetDesktopWindow(VOID)
{
	if (loaded)
		return allowMonitor ? desktopWindow : fakeDesktopWindow;
	return desktopWindow;
}
HDC WINAPI hkGetWindowDC(__in_opt HWND hWnd) {
	if (loaded && hWnd == desktopWindow && !allowMonitor)
		return faGetWindowDC(fakeDesktopWindow);
	return faGetWindowDC(hWnd);
}
BOOL __cdecl hkEncodeToJPEGBuffer(int a1, int a2, int a3, int a4, int a5, DWORD * a6, int a7, int a8, int a9)
{
	if (!allowMonitor) return FALSE;
	return faEncodeToJPEGBuffer(a1, a2, a3, a4, a5, a6, a7, a8, a9);
}
HWND WINAPI hkGetForegroundWindow(VOID)
{
	return allowAllRunOp ? faGetForegroundWindow() : NULL;
}
HDC WINAPI hkCreateDCW(LPCWSTR pwszDriver, LPCWSTR pwszDevice, LPCWSTR pszPort, const DEVMODEW * pdm)
{
	if (isGbFounded)
		return faCreateDCW(pwszDriver, pwszDevice, pszPort, pdm);
	if (!allowMonitor)
	{
		if (StrEqual(pwszDriver, L"DISPLAY"))
			return NULL;
	}
	return faCreateDCW(pwszDriver, pwszDevice, pszPort, pdm);
}
BOOL WINAPI hkEnableMenuItem(HMENU hMenu, UINT uIDEnableItem, UINT uEnable)
{
	if(uIDEnableItem == SC_CLOSE)
		return 0;
	return faEnableMenuItem(hMenu, uIDEnableItem, uEnable);
}
DWORD WINAPI hkSetClassLongA(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded) {
		if (nIndex == GCL_STYLE && (dwNewLong & CS_NOCLOSE) == CS_NOCLOSE)
			return faSetClassLongA(hWnd, nIndex, dwNewLong ^ CS_NOCLOSE);
	}
	return faSetClassLongA(hWnd, nIndex, dwNewLong);
}
DWORD WINAPI hkSetClassLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded) {
		if(nIndex == GCL_STYLE && (dwNewLong & CS_NOCLOSE) == CS_NOCLOSE)
			return faSetClassLongA(hWnd, nIndex, dwNewLong ^ CS_NOCLOSE);
	}
	return faSetClassLongA(hWnd, nIndex, dwNewLong);
}

//HOOK Virus stub
EXTERN_C HRESULT __declspec(dllexport) __cdecl TDAjustCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv)
{
	return faTDAjustCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

HBITMAP hIconRed, hIconGreen, hIconGrey;
HWND hStatusFakeFull, hStatusMain, hStatusLock;


INT_PTR CALLBACK FakeDesktopWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message)
	{
	case WM_INITDIALOG: {
		if(raSetWindowPos) raSetWindowPos(hDlg, HWND_BOTTOM, -screenWidth, -screenHeight, screenWidth, screenHeight, SWP_NOACTIVATE);
		else SetWindowPos(hDlg, HWND_BOTTOM, -screenWidth, -screenHeight, screenWidth, screenHeight, SWP_NOACTIVATE);
		return TRUE;
	}
	case WM_DESTROY: {

		break;
	}
	case WM_DISPLAYCHANGE: {
		VParamInit();
		SendMessage(hDlg, WM_INITDIALOG, NULL, NULL);
		break;
	}
	case WM_QUERYENDSESSION: {
		DestroyWindow(hDlg);
		break;
	}
	default:
		break;
	}
	return 0;
}
INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		SetWindowText(hDlg, L"JiYu Trainer Virus Window");

		hIconRed = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_RED));
		hIconGreen = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_GREEN));
		hIconGrey = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_GRRY));

		hStatusMain = GetDlgItem(hDlg, IDC_STATUS_RUNNING);
		hStatusLock = GetDlgItem(hDlg, IDC_STATUS_LOCK);
		hStatusFakeFull = GetDlgItem(hDlg, IDC_STATUS_FAKEFULL);

		SetTimer(hDlg, TIMER_WATCH_DOG_SRV, 6666, NULL);
		SetTimer(hDlg, TIMER_AUTO_HIDE, 2600, NULL);
		break;
	}
	case WM_DESTROY: {
		DeleteBitmap(hIconRed);
		DeleteBitmap(hIconGreen);
		DeleteBitmap(hIconGrey);
		KillTimer(hDlg, TIMER_WATCH_DOG_SRV);
		PostQuitMessage(0);
		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE)
			DestroyWindow(hWndMsgCenter);
		break;
	}
	case WM_DISPLAYCHANGE: {
		VParamInit();
		break;
	}
	case WM_COMMAND: {
		switch (wParam)
		{
		case IDC_KILL: {
			PostQuitMessage(0);
			ExitProcess(0);
			break;
		}
		case IDC_SMINSIZE: {
			ShowWindow(hDlg, SW_MINIMIZE);
			break;
		}
		case IDC_SHIDE: {
			RECT rc; GetWindowRect(hDlg, &rc);
			SetWindowPos(hDlg, 0, rc.left, -(rc.bottom - rc.top - 8), 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			break;
		}
		case IDC_SW_STATUS_FAKEFULL:
		case IDC_SW_STATUS_LOCKED: {
			if (isLocked) {
				if (fakeFull) SendMessage(hStatusLock, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGreen);
				else SendMessage(hStatusLock, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconRed);
			}
			else SendMessage(hStatusLock, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGrey);
			break;
		}
		case IDC_SW_STATUS_MAIN_FLASH: {
			SendMessage(hStatusMain, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGreen);
			SetTimer(hDlg, TIMER_LIGNT_DELAY1, 1000, NULL);
			break;
		}
		case IDC_SW_STATUS_MAIN: {
			SendMessage(hStatusMain, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGrey);
			break;
		}
		case IDC_SW_STATUS_MAIN_OUT: {
			SendMessage(hStatusMain, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconRed);
			break;
		}
		case IDC_SW_STATUS_RB_FLASH: {
			SendMessage(hStatusFakeFull, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGreen);
			SetTimer(hDlg, TIMER_LIGNT_DELAY2, 500, NULL);
			break;
		}
		case IDC_SW_STATUS_RB: {
			SendMessage(hStatusFakeFull, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGrey);
			break;
		}
		case IDC_SW_STATUS_RB_FAIL: {
			SendMessage(hStatusFakeFull, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconRed);
			break;
		}
		case IDC_SW_STATUS_RB_FAIL_FLASH: {
			SendMessage(hStatusFakeFull, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconRed);
			SetTimer(hDlg, TIMER_LIGNT_DELAY2, 500, NULL);
			break;
		}
		case IDC_SW_STATUS_RB_FLASH_FAST: {
			SendMessage(hStatusFakeFull, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hIconGreen);
			SetTimer(hDlg, TIMER_LIGNT_DELAY2, 200, NULL);
		}
		default:
			break;
		}
		break;
	}
	case WM_COPYDATA: {
		PCOPYDATASTRUCT  pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
		if (pCopyDataStruct->cbData > 0)
		{
			WCHAR recvData[256] = { 0 };
			wcsncpy_s(recvData, (WCHAR *)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
			VHandleMsg(recvData);
		}
		break;
	}
	case WM_LBUTTONDOWN: {
		RECT rc; GetWindowRect(hDlg, &rc);
		if (rc.top < 0) {
			SetWindowPos(hDlg, 0, rc.left, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
		else {
			ReleaseCapture();
			SendMessage(hDlg, WM_NCLBUTTONDOWN, HTCAPTION, 0);
		}
		break;
	}
	case WM_TIMER: {
		if (wParam == TIMER_AUTO_HIDE) {
			KillTimer(hDlg, TIMER_AUTO_HIDE);
			SendMessage(hDlg, WM_COMMAND, IDC_SHIDE, NULL);
			if (doNotShowVirusWindow)
				ShowWindow(hDlg, SW_HIDE);
			if (forceDisableWatchDog)
				KillTimer(hDlg, TIMER_WATCH_DOG_SRV);
		}
		if (wParam == TIMER_WATCH_DOG_SRV) {
			if (wdCount < 32768)  wdCount++;
			else wdCount = 0;
			WCHAR str[21];
			swprintf_s(str, L"wcd:%d", wdCount);
			VSendMessageBack(str, hDlg);
			SendMessage(hDlg, WM_COMMAND, IDC_SW_STATUS_MAIN_FLASH, NULL);
		}
		if (wParam == TIMER_LIGNT_DELAY1) {
			KillTimer(hDlg, TIMER_LIGNT_DELAY1);
			SendMessage(hDlg, WM_COMMAND, IDC_SW_STATUS_MAIN, NULL);
		}
		if (wParam == TIMER_LIGNT_DELAY2) {
			KillTimer(hDlg, TIMER_LIGNT_DELAY2);
			SendMessage(hDlg, WM_COMMAND, IDC_SW_STATUS_RB, NULL);
		}
		break;
	}
	case WM_QUERYENDSESSION: {
		VCloseMsgCenter();
		break;
	}
	default:
		break;
	}
	return 0;
}
INT_PTR CALLBACK JiYuWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	/*
	if (message == WM_TIMER) {
		KillTimer(hWnd, wParam);
		return 0;
	}
	*/
	if (message == WM_GETMINMAXINFO) {
		PMINMAXINFO pMinMaxInfo = (PMINMAXINFO)lParam;
		pMinMaxInfo->ptMinTrackSize.x = 0;
		pMinMaxInfo->ptMinTrackSize.y = 0;
		return 0;
	}
	else if (message == WM_SIZE) {
		RECT rcWindow;
		RECT rcClient;
		GetWindowRect(hWnd, &rcWindow);
		GetClientRect(hWnd, &rcClient);
	REGIET:
		if (jiYuGBDeskRdWnd == NULL) {
			jiYuGBDeskRdWnd = FindWindowExW(hWnd, NULL, NULL, L"TDDesk Render Window");
			if (jiYuGBDeskRdWnd == NULL) goto JOUT;

			//处理新版极域的广播工具栏
			if (jiYuVersion == jiYuVersions::jiYuVersions402016HH) {
				jiYuGBToolWnd = FindWindowExW(hWnd, jiYuGBDeskRdWnd, NULL, L"");
				if (jiYuGBToolWnd != NULL) {
					RECT rcTool; GetClientRect(jiYuGBToolWnd, &rcTool);
					jiYuGBToolHeight = rcTool.bottom - rcTool.top;
					VOutPutStatus(L"[D] jiYuGBToolWnd : 0x%08x", jiYuGBToolWnd);
					VOutPutStatus(L"[D] jiYuGBToolHeight : %d", jiYuGBToolHeight);
					//Find tool bar window
					//HWND hBtnTop = GetDlgItem(jiYuGBToolWnd, 0x07E5);
					//HWND hBtnFull = GetDlgItem(jiYuGBToolWnd, 0x03EC);
					//EnableWindow(hBtnFull, FALSE);
				}
			}

			//HOOK TDDesk Render Window for WM_SIZE
			WNDPROC oldWndProc = (WNDPROC)GetWindowLong(jiYuGBDeskRdWnd, GWL_WNDPROC);
			if (oldWndProc != (WNDPROC)JiYuTDDeskWndProc) {
				jiYuTDDeskWndProc = (WNDPROC)oldWndProc;
				SetWindowLong(jiYuGBDeskRdWnd, GWL_WNDPROC, (LONG)JiYuTDDeskWndProc);
				VOutPutStatus(L"[J] Hooked jiYuGBDeskRdWnd %d (0x%08x) WNDPROC", jiYuGBDeskRdWnd, jiYuGBDeskRdWnd);
			}
		}
		if (!IsWindow(jiYuGBDeskRdWnd) || GetParent(jiYuGBDeskRdWnd) != hWnd) {
			jiYuGBDeskRdWnd = NULL;
			goto REGIET;
		}
		if (jiYuGBDeskRdWnd != NULL) {
			ShowWindow(jiYuGBDeskRdWnd, SW_SHOW);
			raMoveWindow(jiYuGBDeskRdWnd, 0, isLocked ? 0 : jiYuGBToolHeight, rcClient.right - rcClient.left,
				rcClient.bottom - rcClient.top - (isLocked ? 0 : jiYuGBToolHeight), TRUE);
		}
	}
	else if (message == WM_SHOWWINDOW)
	{
		if (wParam)
		{
			SendMessage(hWnd, WM_SIZE, NULL, NULL);
			int w = (int)((double)screenWidth * (3.0 / 4.0)), h = (int)((double)screenHeight * (double)(4.0 / 5.0));
			if (raSetWindowPos) raSetWindowPos(hWnd, 0, (screenWidth - w) / 2, (screenHeight - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
			else SetWindowPos(hWnd, 0, (screenWidth - w) / 2, (screenHeight - h) / 2, w, h, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else jiYuWndCanSize.push_back(hWnd);
	}
	else if (message == WM_DESTROY)
	{
		jiYuWnds.remove(hWnd);
		jiYuWndCanSize.remove(hWnd);
		VSwitchLockState(false);
		isGbFounded = false;
	}
	else if (message == WM_COMMAND) {
		switch (wParam)
		{
		case IDM_FULL:
		case IDM_TOPMOST: return SendMessage(hWnd, WM_SYSCOMMAND, wParam, NULL);
		case IDM_HELP_GB: {
			MessageBox(hWnd, L"这个右键菜单是 JiYuTrainer 为方便您控制广播全屏和窗口添加的。\n如果您需要打开极域的右键菜单，请在窗口中心右键。\n关于更多帮助，请查看 JiYuTrainer 帮助文档。 ", L"JiYuTrainer 提示", MB_ICONINFORMATION);
			return 0;
		}
		case IDM_HELP_GB2: {
			VSendMessageBack(L"hkb:showhelp", hWndMsgCenter);
			return 0;
		}
		case IDM_CLOSE_GB: return SendMessage(hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
		}
		if (jiYuVersion == jiYuVersions::jiYuVersions402016HH) {
			switch (wParam)
			{
			case 0x07E5://Top
				return SendMessage(hWnd, WM_SYSCOMMAND, IDM_TOPMOST, (LPARAM)-1);
			case 0x03EC://Full
				return SendMessage(hWnd, WM_SYSCOMMAND, IDM_FULL, NULL);
			default: break;
			}
		}
	}
	else if (message == WM_SYSCOMMAND) {
		switch (wParam)
		{
		case IDM_TOPMOST: {
			LONG oldLong = GetWindowLong(hWnd, GWL_EXSTYLE);
			if ((oldLong & WS_EX_TOPMOST) == WS_EX_TOPMOST)
			{
				gbCurrentIsTop = false;

				CheckMenuItem(hMenuGb, IDM_TOPMOST, MF_UNCHECKED);
				CheckMenuItem(GetSystemMenu(hWnd, FALSE), IDM_TOPMOST, MF_UNCHECKED);
				VSendMessageBack(L"hkb:gbuntop", hWndMsgCenter);
			}
			else {
				//通知允许置顶设置
				if (!allowGbTop) VSendMessageBack(L"hkb:algbtop", hWndMsgCenter);
				gbCurrentIsTop = true;

				CheckMenuItem(hMenuGb, IDM_TOPMOST, MF_CHECKED);
				CheckMenuItem(GetSystemMenu(hWnd, FALSE), IDM_TOPMOST, MF_CHECKED);
				VSendMessageBack(L"hkb:gbtop", hWndMsgCenter);
			}
			if (jiYuVersion == jiYuVersions::jiYuVersions402016HH)
				CheckDlgButton(jiYuGBToolWnd, 0x07E5, gbCurrentIsTop);
			break;
		}
		case IDM_FULL: {
			HMENU hMenu = GetSystemMenu(hWnd, FALSE);
			gbFullManual = !gbFullManual;
			if (gbFullManual) {
				CheckMenuItem(hMenuGb, IDM_FULL, MF_CHECKED);
				CheckMenuItem(hMenu, IDM_FULL, MF_CHECKED);
				VSendMessageBack(L"hkb:gbmfull", hWndMsgCenter);
			}
			else {
				CheckMenuItem(hMenuGb, IDM_FULL, MF_UNCHECKED);
				CheckMenuItem(hMenu, IDM_FULL, MF_UNCHECKED);
				VSendMessageBack(L"hkb:gbmnofull", hWndMsgCenter);
			}
			break;
		}
		case SC_CLOSE: {
			if (MessageBox(hWnd, L"您真的要关闭广播窗口吗？", L"JiYuTrainer - 提示", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				CloseWindow(hWnd);
				DestroyWindow(hWnd);
			}
			break;
		}
		default: break;
		}
	}
	else if (message == WM_KEYUP) {
		if (wParam == VK_ESCAPE && fakeFull)
			SendMessage(hWnd, WM_SYSCOMMAND, IDM_FULL, NULL);
	}
	else if (message == WM_RBUTTONUP) {

		bool showMenu = true;
		POINT pos;
		pos.x = GET_X_LPARAM(lParam);
		pos.y = GET_Y_LPARAM(lParam);

		if (jiYuVersion == jiYuVersions::jiYuVersions40 && (pos.x > 60 || pos.y > 20)) 
			showMenu = false;
		if (showMenu) {
			ClientToScreen(hWnd, &pos);
			int id = TrackPopupMenu(hMenuGb, TPM_RETURNCMD | TPM_LEFTALIGN | TPM_TOPALIGN, pos.x + 10, pos.y + 10, NULL, hWnd, NULL);
			SendMessage(hWnd, WM_COMMAND, id, NULL);
		}
	}
JOUT:
	if (jiYuWndProc) return jiYuWndProc(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}
INT_PTR CALLBACK JiYuTDDeskWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_SIZE) {
		RECT rcParent;
		GetClientRect(GetParent(hWnd), &rcParent);
		int w = LOWORD(lParam), h = HIWORD(lParam),
			rw = rcParent.right - rcParent.left, rh = rcParent.bottom - rcParent.top - (isLocked ? 0 : jiYuGBToolHeight);
		if (w != rw || h != rh) 
			raMoveWindow(hWnd, 0, isLocked ? 0 : jiYuGBToolHeight, rw, rh, TRUE);
	}
	else if (message == WM_RBUTTONUP) SendMessage(GetParent(hWnd), message, wParam, lParam);
	else if (message == WM_KEYUP) SendMessage(GetParent(hWnd), message, wParam, lParam);
	if (jiYuTDDeskWndProc) return jiYuTDDeskWndProc(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}


