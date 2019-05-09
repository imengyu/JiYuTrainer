
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
#include <Shlwapi.h>
#include <ShellAPI.h>
#include <CommCtrl.h>
#include <windowsx.h>

#define TIMER_WATCH_DOG_SRV 10011
#define TIMER_AUTO_HIDE 10012
#define TIMER_COUNTDOWN_TICK 10013

#define DirectorySeparatorChar L'\\'
#define AltDirectorySeparatorChar  L'/'
#define VolumeSeparatorChar  L':'

using namespace std;

extern HINSTANCE hInst;

WNDPROC jiYuWndProc;
WNDPROC jiYuTDDeskWndProc;
list<HWND>  jiYuWnds;
list<HWND>  jiYuWndCanSize;
HWND jiYuGBWnd = NULL;
HWND jiYuGBDeskRdWnd = NULL;

HWND hWndMsgCenter = NULL;
HWND hListBoxStatus = NULL;

LRESULT hWndOpConformRs = 0;
LRESULT hWndOutOfControlConformRs = 0;
INT screenWidth, screenHeight;
bool outlineEndJiy = false;
HWND hWndOpConformNoBtn = NULL;
bool bandAllRunOp = false, allowNextRunOp = false, allowAllRunOp  = true;
std::list<std::wstring> runOPWhiteList;
bool forceKill = false;
int wdCount = 0;
WCHAR mainIniPath[MAX_PATH];
WCHAR mainFullPath[MAX_PATH];
WCHAR mainDir[MAX_PATH];
WCHAR currOpCfPath[MAX_PATH];
WCHAR currOpCfPararm[MAX_PATH];

fnTDAjustCreateInstance faTDAjustCreateInstance = NULL;

//Real api address
typedef void(__fastcall *fnUnHookLocalInput)(void);

fnUnHookLocalInput UnHookLocalInput = NULL;
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
fnCreateProcessW faCreateProcessW = NULL;
fnDwmEnableComposition faDwmEnableComposition = NULL;
fnWinExec faWinExec = NULL;
fnCallNextHookEx faCallNextHookEx = NULL;

bool loaded = false;

void VUnloadAll() {

	if (loaded)
	{
		VCloseMsgCenter();
		VCloseFuckDrivers();
		jiYuWnds.clear();
		jiYuWndCanSize.clear();
		runOPWhiteList.clear();
		VUnInstallHooks();
		loaded = false;
		FreeLibrary(hInst);
	}
}
void VLoad() {
	VParamInit();

	//Get main mod name
	WCHAR mainModName[MAX_PATH];
	GetModuleFileName(NULL, mainModName, MAX_PATH);

	std::wstring path(mainModName);

	int lastQ = path.find_last_of(L'\\');
	std::wstring name = path.substr(lastQ + 1, path.length() - lastQ - 1);

	if (name == L"StudentMain.exe") {
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
		//本进程，需要挂钩以保证极域 LibTDProcHook32 无法正常运行
		VInstallHooks(VirusModeProtet);
	}
	else if (name == L"MasterHelper.exe") {
		//MasterHelper.exe 搞一些事情
		VInstallHooks(VirusModeMaster);
	}

	loaded = true;
}
void VRunMain() {
	
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
	hWndMsgCenter = CreateDialog(hInst, MAKEINTRESOURCE(IDD_MSGCT), NULL, MainWndProc);
	ShowWindow(hWndMsgCenter, SW_SHOW);
	UpdateWindow(hWndMsgCenter);

	hListBoxStatus = GetDlgItem(hWndMsgCenter, IDC_STATUS_LIST);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return 0;
}

void VParamInit() {
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
}
void VInitSettings()
{
	WCHAR w[32];
	GetPrivateProfileString(L"JYK", L"AutoForceKill", L"FALSE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) forceKill = true;
	GetPrivateProfileString(L"JYK", L"AllowAllRunOp", L"FALSE", w, 32, mainIniPath);
	if (StrEqual(w, L"TRUE") || StrEqual(w, L"true") || StrEqual(w, L"1")) allowAllRunOp = true;
	GetPrivateProfileString(L"JYK", L"BandAllRunOp", L"TRUE", w, 32, mainIniPath);
	if (!StrEqual(w, L"TRUE") && !StrEqual(w, L"true") && !StrEqual(w, L"1")) bandAllRunOp = false;
}
void VLaterInit() {

	if (PathFileExists(mainIniPath)) 
	{
		VInitSettings();
		VLoadOpWhiteList();
	}
}
void VCreateMsgCenter() {
	CreateThread(NULL, NULL, VMsgCenterRunThread, NULL, NULL, NULL);
}
void VCloseMsgCenter() {
	DestroyWindow(hWndMsgCenter);
}
void VHandleMsg(LPWSTR buff) {
	VOutPutStatus(L"Receive message : %s", buff);
	wstring act(buff);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2) {
		if (arr[0] == L"hw") 
			VHookWindow(arr[1].c_str());
		else if (arr[0] == L"hwf")
			VHookFWindow(arr[1].c_str());
		else if (arr[0] == L"ss")
			VBoom();
		else if (arr[0] == L"ss2") {
			PostQuitMessage(0);
			ExitProcess(0);
		}
		else if (arr[0] == L"hk") {
			if(arr[1]==L"ckstat") VSendMessageBack(L"hkb:succ", hWndMsgCenter);
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
			}
			else if (arr[1] == L"reset") {
				VInitSettings();
				VSendMessageBack(L"hkb:succ", hWndMsgCenter);
			}

			
		}
		else if (arr[0] == L"ukt") {
			if (UnHookLocalInput) { UnHookLocalInput(); VSendMessageBack(L"ukt:succ", hWndMsgCenter); }
			else VSendMessageBack(L"ukt:fail", hWndMsgCenter);
		}
		else if (arr[0] == L"test") {
			VShowOpConfirmDialog(L"test", L"test");
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
void VHookFWindow(const wchar_t* hWndStr) {
	HWND hWnd = (HWND)_wtol(hWndStr);
	if (IsWindow(hWnd)) {
		//GuangBo window fix
		if (hWnd != jiYuGBWnd) {
			WCHAR text[16];
			GetWindowText(hWnd, text, 16);
			if (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口")) {
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
			WCHAR text[16];
			GetWindowText(hWnd, text, 16);
			if (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口")) {
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
void VFixGuangBoWindow(HWND hWnd) {
	WNDPROC oldWndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
	if (oldWndProc != (WNDPROC)JiYuWndProc) {
		jiYuWndProc = (WNDPROC)oldWndProc;
		SetWindowLong(hWnd, GWL_WNDPROC, (LONG)JiYuWndProc);
		VOutPutStatus(L"Hooked hWnd %d (0x%08x) WNDPROC", hWnd, hWnd);
		SendMessage(hWnd, WM_SHOWWINDOW, TRUE, FALSE);
	}
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
void VBoom() {

	PostQuitMessage(0);
	ExitProcess(0);

	/*
	CHAR*P = 0;
	*P = 0;
	*/
}
bool VCheckIsTargetWindow(LPWSTR text) {
	return (StrEqual(text, L"屏幕广播") || StrEqual(text, L"屏幕演播室窗口") || StrEqual(text, L"BlackScreen Window"));
}
void VSendMessageBack(LPCWSTR buff, HWND hDlg) {
	HWND receiveWindow = FindWindow(NULL, L"JiYu Trainer Main Window");
	if (receiveWindow) {
		COPYDATASTRUCT copyData = { 0 };
		copyData.lpData = (PVOID)buff;
		copyData.cbData = sizeof(WCHAR) * (wcslen(buff) + 1);
		SendMessageTimeout(receiveWindow, WM_COPYDATA, (WPARAM)hDlg, (LPARAM)&copyData, SMTO_NORMAL, 500, 0);
	}
	else if(!outlineEndJiy) {
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
					ShellExecute(hWndMsgCenter, L"open", mainFullPath, NULL, NULL, SW_NORMAL);
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

HHOOK g_hhook = NULL;

BOOL hk1 = 0, hk2 = 0, hk3 = 0, hk4 = 0,
hk5 = 0, hk6 = 0, hk7 = 0, hk8 = 0,
hk9 = 0, hk10 = 0, hk11 = 0, hk12 = 0,
hk13 = 0, hk14 = 0, hk15 = 0, hk16 = 0,
hk17 = 0, hk18 = 0, hk19= 0, hk20 = 0,
hk21 = 0, hk22 = 0, hk23 = 0, hk24 = 0,
hk25 = 0, hk26 = 0, hk27 = 0, hk28 = 0;

void VInstallHooks(VirusMode mode) {

	//Mhook_SetHook
	HMODULE hUser32 = GetModuleHandle(L"user32.dll");
	HMODULE hKernel32 = GetModuleHandle(L"kernel32.dll");
	HMODULE hShell32 = GetModuleHandle(L"shell32.dll");
	HMODULE hDwmApi = GetModuleHandle(L"dwmapi.dll");

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

	raDeviceIoControl = (fnDeviceIoControl)GetProcAddress(hKernel32, "DeviceIoControl");
	faCreateFileA = (fnCreateFileA)GetProcAddress(hKernel32, "CreateFileA");
	faCreateFileW = (fnCreateFileW)GetProcAddress(hKernel32, "CreateFileW");
	faCreateProcessW = (fnCreateProcessW)GetProcAddress(hKernel32, "CreateProcessW");
	faWinExec = (fnWinExec)GetProcAddress(hKernel32, "WinExec");

	faExitWindowsEx = (fnExitWindowsEx)GetProcAddress(hUser32, "ExitWindowsEx");
	faShellExecuteW = (fnShellExecuteW)GetProcAddress(hShell32, "ShellExecuteW");
	faShellExecuteExW = (fnShellExecuteExW)GetProcAddress(hShell32, "ShellExecuteExW");


	if(hDwmApi) faDwmEnableComposition = (fnDwmEnableComposition)GetProcAddress(hDwmApi, "DwmEnableComposition");

	//HMODULE hTDDesk2 = GetModuleHandle(L"libtddesk2.dll");
	//if (hTDDesk2) {
	//	faTDDeskCreateInstance = (fnTDDeskCreateInstance)GetProcAddress(hTDDesk2, "TDDeskCreateInstance");
	//}

	if (mode == VirusModeHook) {

		HMODULE hTDMaster = GetModuleHandle(L"libTDMaster.dll");
		UnHookLocalInput = (fnUnHookLocalInput)GetProcAddress(hTDMaster, "UnHookLocalInput");
		if (UnHookLocalInput) UnHookLocalInput();

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
	}
	if (mode == VirusModeMaster) {
		
		SetWindowsHookExA(WH_CBT, VCBTProc, hInst, GetCurrentThreadId());

		hk8 = Mhook_SetHook((PVOID*)&faSetWindowsHookExA, hkSetWindowsHookExA);
		hk19 = Mhook_SetHook((PVOID*)&faExitWindowsEx, hkExitWindowsEx);
		hk22 = Mhook_SetHook((PVOID*)&faCreateProcessW, hkCreateProcessW);
		hk25 = Mhook_SetHook((PVOID*)&faCallNextHookEx, hkCallNextHookEx);
	}
	if (mode == VirusModeHook || mode == VirusModeProtet) {
		hk25 = Mhook_SetHook((PVOID*)&faCallNextHookEx, hkCallNextHookEx);
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

	if (g_hhook) {
		UnhookWindowsHookEx(g_hhook);
		g_hhook = NULL;
	}
}

//Fuck driver devices
HANDLE hDeviceTDKeybd = NULL;
HANDLE hDeviceTDProcHook = NULL;

void VOpenFuckDrivers() {
	hDeviceTDKeybd = CreateFile(L"\\\\.\\TDKeybd", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hDeviceTDProcHook = CreateFile(L"\\\\.\\TDProcHook", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}
void VCloseFuckDrivers()
{
	if (hDeviceTDProcHook) CloseHandle(hDeviceTDProcHook);
	if (hDeviceTDKeybd) CloseHandle(hDeviceTDKeybd);
}

//Hook stubs

BOOL WINAPI hkSetForegroundWindow(HWND hWnd)
{
	if (loaded && VIsInIllegalWindows(hWnd))
		return TRUE;
	return raSetForegroundWindow(hWnd);
}
BOOL WINAPI hkSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int x, int y, int cx, int cy, UINT uFlags)
{
	if (loaded) 
	{
		if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) 
		{
			if (VIsInIllegalCanSizeWindows(hWnd))
			{
				jiYuWndCanSize.remove(hWnd);
				raSetWindowPos(hWnd, hWndInsertAfter, x, y, cx, cy, uFlags | SWP_NOZORDER);
				SendMessage(hWnd, WM_SIZE, NULL, NULL);
				return TRUE;
			}
			return TRUE;
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
		if (x == 0 && y == 0 && cx == screenWidth && cy == screenHeight) {
			if (VIsInIllegalCanSizeWindows(hWnd)) {
				jiYuWndCanSize.remove(hWnd);
				HDWP rs = faDeferWindowPos(hWinPosInfo, hWnd, hWndInsertAfter, x, y, cx, cy, uFlags | SWP_NOZORDER);
				SendMessage(hWnd, WM_SIZE, NULL, NULL);
				return rs;
			}
			return NULL;
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
			if (VIsInIllegalCanSizeWindows(hWnd)) {
				jiYuWndCanSize.remove(hWnd);
				return raMoveWindow(hWnd,  x, y, cx, cy, bRepaint);
			}
			return TRUE;
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
	}
	return faCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}
HHOOK WINAPI hkSetWindowsHookExA(int idHook, HOOKPROC lpfn, HINSTANCE hmod, DWORD dwThreadId)
{
	if (loaded)
	{
		if (idHook == WH_CBT || idHook == WH_MOUSE_LL || idHook == WH_MOUSE) {
			SetLastError(ERROR_ACCESS_DENIED);
			return FALSE;
		}
	}
	return faSetWindowsHookExA(idHook, lpfn, hmod, dwThreadId);
}
VOID WINAPI hkmouse_event(DWORD dwFlags, DWORD dx, DWORD dy, DWORD dwData,  ULONG_PTR dwExtraInfo)
{
	
}
UINT WINAPI hkSendInput(UINT cInputs, LPINPUT pInputs, int cbSize)
{
	if (loaded)
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
	else if (StringHlp::StrContainsW(lowStr, L"jiyukiller.exe", NULL) 
		|| StringHlp::StrContainsW(lowStr, L"sogouinput", NULL)
		|| StringHlp::StrContainsW(lowStr, L"baidupinyin", NULL)
		|| VIsOpInWhiteList(lowStr)) canContinue = true;
	else if (StringHlp::StrContainsW(lowStr, L"tdchalk.exe", NULL))
		canContinue = false;
	else if (bandAllRunOp || !VShowOpConfirmDialog(lpApplicationName, lpCommandLine)) canContinue = false;

	if(canContinue) return faCreateProcessW(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	else return TRUE;
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
HRESULT __cdecl hkTDDeskCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv)
{
	return E_FAIL;
}
LONG WINAPI hkSetWindowLongA(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded && VIsInIllegalWindows(hWnd))
		return GetWindowLongA(hWnd, nIndex);
	return faSetWindowLongA(hWnd, nIndex, dwNewLong);
}
LONG WINAPI hkSetWindowLongW(HWND hWnd, int nIndex, LONG dwNewLong)
{
	if (loaded && VIsInIllegalWindows(hWnd))
		return GetWindowLongW(hWnd, nIndex);
	return faSetWindowLongW(hWnd, nIndex, dwNewLong);
}
BOOL WINAPI hkShowWindow(HWND hWnd, int nCmdShow)
{
	if (nCmdShow == SW_SHOW || nCmdShow == SW_SHOWNORMAL) {
		WCHAR text[32];
		GetWindowText(hWnd, text, 32);
		if (VCheckIsTargetWindow(text)) {
			VSendMessageBack(L"hkb:immck", hWndMsgCenter);
		}
	}
	return faShowWindow(hWnd, nCmdShow);
}
HRESULT WINAPI hkDwmEnableComposition(UINT uCompositionAction)
{
	return  S_OK;
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
	else if (StringHlp::StrContainsW(lowStr, L"jiyukiller.exe", NULL)
		|| StringHlp::StrContainsW(lowStr, L"sogouinput", NULL)
		|| StringHlp::StrContainsW(lowStr, L"baidupinyin", NULL)
		|| VIsOpInWhiteList(lowStr)) canContinue = true;
	else if (StringHlp::StrContainsW(lowStr, L"tdchalk.exe", NULL)) canContinue = false;
	else if (bandAllRunOp || !VShowOpConfirmDialog(uniStr, L"")) canContinue = false;

	delete uniStr;
	if (canContinue) return faWinExec(lpCmdLine, uCmdShow);
	else return TRUE;
}
LRESULT WINAPI hkCallNextHookEx(HHOOK hhk, int nCode, WPARAM wParam, LPARAM lParam) 
{
	return faCallNextHookEx(hhk, nCode, wParam, lParam);
}

//HOOK Virus stub
EXTERN_C HRESULT __declspec(dllexport) __cdecl TDAjustCreateInstance(CLSID *rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, IID *riid, LPVOID *ppv)
{
	return faTDAjustCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv);
}

INT_PTR CALLBACK MainWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		SetWindowText(hDlg, L"JiYu Trainer Virus Window");
		SetTimer(hDlg, TIMER_WATCH_DOG_SRV, 5000, NULL);
		SetTimer(hDlg, TIMER_AUTO_HIDE, 3000, NULL);
		break;
	}
	case WM_DESTROY: {
		KillTimer(hDlg, TIMER_WATCH_DOG_SRV);
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
		if (wParam == IDC_KILL) {
			PostQuitMessage(0);
			ExitProcess(0);
		}
		if (wParam == IDC_SMINSIZE) {
			ShowWindow(hDlg, SW_MINIMIZE);
		}
		if (wParam == IDC_SHIDE) {
			RECT rc; GetWindowRect(hDlg, &rc);
			SetWindowPos(hDlg, 0, rc.left, -56, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
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
		if (rc.top == -56) {
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
		}
		if (wParam == TIMER_WATCH_DOG_SRV) {
			if (wdCount < 32768)  wdCount++;
			else wdCount = 0;
			WCHAR str[21];
			swprintf_s(str, L"wcd:%d", wdCount);
			VSendMessageBack(str, hDlg);
		}
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
	if (message == WM_SIZE) {
		RECT rcWindow;
		RECT rcClient;
		GetWindowRect(hWnd, &rcWindow);
		GetClientRect(hWnd, &rcClient);
		if (UnHookLocalInput) UnHookLocalInput();
		bool setToFull = false;
		if (rcWindow.right - rcWindow.left == screenWidth && rcWindow.bottom - rcWindow.top == screenHeight)
			setToFull = true;
		REGIET:
		if (jiYuGBDeskRdWnd == NULL) {
			jiYuGBDeskRdWnd = FindWindowExW(hWnd, NULL, NULL, L"TDDesk Render Window");
			if (jiYuGBDeskRdWnd == NULL) goto JOUT;
			//HOOK TDDesk Render Window for WM_SIZE
			WNDPROC oldWndProc = (WNDPROC)GetWindowLong(jiYuGBDeskRdWnd, GWL_WNDPROC);
			if (oldWndProc != (WNDPROC)JiYuTDDeskWndProc) {
				jiYuTDDeskWndProc = (WNDPROC)oldWndProc;
				SetWindowLong(jiYuGBDeskRdWnd, GWL_WNDPROC, (LONG)JiYuTDDeskWndProc);
				VOutPutStatus(L"Hooked jiYuGBDeskRdWnd %d (0x%08x) WNDPROC", jiYuGBDeskRdWnd, jiYuGBDeskRdWnd);
			}
		}
		if (!IsWindow(jiYuGBDeskRdWnd) || GetParent(jiYuGBDeskRdWnd) != hWnd) {
			jiYuGBDeskRdWnd = NULL;
			goto REGIET;
		}
		if (jiYuGBDeskRdWnd != NULL) {
			ShowWindow(jiYuGBDeskRdWnd, SW_SHOW);
			if (setToFull) raMoveWindow(jiYuGBDeskRdWnd, 0, 0, screenWidth, screenHeight, TRUE);
			else {
				//SendMessage(jiYuGBDeskRdWnd, WM_SIZE, 0, MAKELPARAM(10, 10));
				raMoveWindow(jiYuGBDeskRdWnd, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, TRUE);
			}
		}
	}
	if (message == WM_SHOWWINDOW)
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
	if (message == WM_DESTROY)
	{
		jiYuWnds.remove(hWnd);
		jiYuWndCanSize.remove(hWnd);
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
			rw = rcParent.right - rcParent.left, rh = rcParent.bottom - rcParent.top;
		if (w != rw || h != rh) 
			raMoveWindow(hWnd, 0, 0, rw, rh, TRUE);
	}
	if (jiYuTDDeskWndProc) return jiYuTDDeskWndProc(hWnd, message, wParam, lParam);
	else return DefWindowProc(hWnd, message, wParam, lParam);
}

