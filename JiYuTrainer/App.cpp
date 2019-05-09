#include "stdafx.h"
#include "resource.h"
#include "App.h"
#include "PathHelper.h"
#include "SysHlp.h"
#include "NtHlp.h"
#include "KernelUtils.h"
#include "DriverLoader.h"
#include <Shlwapi.h>
#include <winioctl.h>
#include <CommCtrl.h>
#include <ShellAPI.h>

extern LoggerInternal * currentLogger;

JTAppInternal::JTAppInternal(HINSTANCE hInstance)
{
	this->hInstance = hInstance;
}
JTAppInternal::~JTAppInternal()
{
	ExitClear();
}

int JTAppInternal::CheckInstall()
{
	WCHAR installDir[MAX_PATH];
	if (CheckIsPortabilityDevice(fullPath.c_str())) {//是usb设备
		WCHAR sysTempPath[MAX_PATH + 1];//在TEMP目录安装
		GetTempPath(MAX_PATH, sysTempPath);
		wcscpy_s(installDir, sysTempPath);
		wcscat_s(installDir, L"JiYuKiller");
		if (!Path::Exists(installDir) && !CreateDirectory(installDir, NULL)) {
			wcscpy_s(installDir, fullDir.c_str());
			MessageBox(NULL, L"无法创建临时目录，请尝试使用管理员身份运行本程序", L"错误", MB_ICONERROR);
			ExitProcess(-1);
		}
		appStartType = AppStartTypeInTemp;
	}
	else {//不是usb
		wcscpy_s(installDir, fullDir.c_str());//当前目录
		appStartType = AppStartTypeNormal;
	}

	//拼合路径字符串
	for (int i = 0; i < PART_COUNT; i++) 
		parts[i] = std::wstring(installDir) + L"\\" + parts[i];

	for (int i = 1; i < PART_COUNT; i++) {
		LPCWSTR path = parts[i].c_str();

		if (!Path::Exists(path))
		{
			if (InstallResFile(hInstance, MAKEINTRESOURCE(partsResId[i]), L"BIN", path) != ExtractSuccess) {
				FAST_STR_BINDER(str, L"安装部件 %s 时，发生错误。请尝试在其他地方安装本程序。", 400, path);
				MessageBox(NULL, str, APP_TITLE, MB_ICONERROR);
				return -1;
			}
		}
	}

	if (!Path::Exists(parts[0]))
	{
		if (!CopyFile(fullPath.c_str(), parts[0].c_str(), FALSE)) {
			MessageBox(NULL, L"安装主部件时，发生错误。请尝试在其他地方安装本程序。", APP_TITLE, MB_ICONERROR);
			return -1;
		}
	}

	return 0;
}
bool JTAppInternal::CheckIsPortabilityDevice(LPCWSTR path) {
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

EXTRACT_RES JTAppInternal::InstallResFile(HINSTANCE resModule, LPWSTR resId, LPCWSTR resType, LPCWSTR extractTo)
{
	EXTRACT_RES result = ExtractUnknow;
	HANDLE hFile = CreateFile(extractTo, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		result = ExtractCreateFileError;
		return result;
	}

	HRSRC hResource = FindResourceW(resModule, resId, resType);
	if (hResource) {
		HGLOBAL hg = LoadResource(resModule, hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
			{
				DWORD dwSize = SizeofResource(resModule, hResource);
				DWORD writed;
				if (WriteFile(hFile, pData, dwSize, &writed, NULL)) result = ExtractSuccess;
				else result = ExtractWriteFileError;;
			}
			else result = ExtractReadResError;
		}
		else result = ExtractReadResError;
	}
	else result = ExtractReadResError;
	CloseHandle(hFile);

	return result;
}
bool JTAppInternal::IsCommandExists(LPCWSTR cmd)
{
	return FindArgInCommandLine(appArgList, appArgCount, cmd) >= 0;
}
int JTAppInternal::FindArgInCommandLine(LPWSTR *szArgList, int argCount, const wchar_t * arg) {
	for (int i = 0; i < argCount; i++) {
		if (wcscmp(szArgList[i], arg) == 0)
			return i;
	}
	return -1;
}

int JTAppInternal::Run()
{
	appResult = RunInternal();
	return appResult;
}
int JTAppInternal::RunCheckRunningApp()//如果程序已经有一个在运行，则返回true
{
	HWND oldWindow = FindWindow(L"sciter-jytrainer-main-window", L"JiYu Trainer Main Window");
	if (oldWindow != NULL) {
		if (!IsWindowVisible(oldWindow)) ShowWindow(oldWindow, SW_SHOW);
		if (IsIconic(oldWindow)) ShowWindow(oldWindow, SW_RESTORE);
		SetForegroundWindow(oldWindow);
		return -1;
	}
	HANDLE  hMutex = CreateMutex(NULL, FALSE, L"JYT");
	if (hMutex && (GetLastError() == ERROR_ALREADY_EXISTS))
	{
		CloseHandle(hMutex);
		hMutex = NULL;
		return 1;
	}
	return 0;
}
bool JTAppInternal::RunArgeeMentDialog()
{
	int rs = DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_DIALOG_ARGEEMENT), NULL, ArgeementWndProc);
	if (rs == IDYES) {
		appSetting->SetSettingBool(L"Argeed", true, L"JTArgeement");
		return true;
	}
	return false;
}
int JTAppInternal::RunInternal()
{
	if (SysHlp::GetSystemVersion() == SystemVersionNotSupport) {
		MessageBox(NULL, L"运行本程序最低要求 Windows XP，请使用更高版本的系统", L"JiYuTrainer - 错误", MB_ICONERROR);
		return 0;
	}

	MLoadNt();

	InitPrivileges();
	InitLogger();
	InitPath();
	InitCommandLine();
	InitArgs();
	InitSettings();
	InitPP();

	if(!appForceNoDriver) XLoadDriver();
	if (!appArgForceNoInstall && CheckInstall())
		return 0;

	if(!appArgeementArgeed && !RunArgeeMentDialog())
		return 0;

	int oldStatus = RunCheckRunningApp();
	if (oldStatus == 1) {
		MessageBox(0, L"已经有一个程序正在运行，同时只能运行一个实例，请关闭之前那个", L"JiYuTrainer - 错误", MB_ICONERROR);
		return 0;
	}
	if (oldStatus == -1)
		return 0;

	appWorker = new TrainerWorkerInternal();

	if (appArgForceTemp || appStartType == AppStartTypeInTemp) {
		SysHlp::RunApplication(parts[0].c_str(), (L"-f " + fullPath).c_str());
	}
	else if (appStartType == AppStartTypeNormal) {
		HMODULE hMain = LoadLibrary(parts[1].c_str());
		if (!hMain) {
			LPCWSTR errStr = SysHlp::ConvertErrorCodeToString(GetLastError());
			FAST_STR_BINDER(str, L"加载主部件发生错误。请尝试重新安装本程序。\n错误：%s (%d)", 100, errStr, GetLastError());
			LocalFree((HLOCAL)errStr);
			MessageBox(NULL, str, APP_TITLE, MB_ICONERROR);
		}
		else {
			typedef int(*fnJTUI_RunMain)();
			fnJTUI_RunMain JTUI_RunMain = (fnJTUI_RunMain)GetProcAddress(hMain, "JTUI_RunMain");
			if (JTUI_RunMain) return JTUI_RunMain();
			else MessageBox(NULL, L"加载主部件发生错误。主部件损坏", APP_TITLE, MB_ICONERROR);
		}
	}
	else if (appStartType == AppStartTypeUpdater) {

	}

	return 0;
}

void JTAppInternal::Exit(int code)
{
	ExitInternal();
	ExitClear();
	ExitProcess(code);
}
bool JTAppInternal::ExitInternal()
{

	return false;
}
void JTAppInternal::ExitClear()
{
	if (appArgList) {
		LocalFree(appArgList);
		appArgList = nullptr;
	}
	if (appWorker) {
		delete appWorker;
		appWorker = nullptr;
	}
	if (appSetting) {
		delete appSetting;
		appSetting = nullptr;
	}
	if (appLogger) {
		delete appLogger;
		appLogger = nullptr;
	}
}


LPCWSTR JTAppInternal::GetPartFullPath(int partId)
{
	if (partId == PART_INI) 
		return fullIniPath.c_str();
	if(partId >=0 && partId < PART_COUNT)
		return parts[partId].c_str();
	return NULL;
}

void JTAppInternal::InitPath()
{
	WCHAR buffer[MAX_PATH];
	GetModuleFileName(hInstance, buffer, MAX_PATH);

	fullPath = buffer;

	PathRemoveFileSpec(buffer);

	fullDir = buffer;

	GetModuleFileName(hInstance, buffer, MAX_PATH);
	PathRenameExtension(buffer, L".ini");

	fullIniPath = buffer;
}
void JTAppInternal::InitCommandLine()
{
	appArgList = CommandLineToArgvW(GetCommandLine(), &appArgCount);
	if (appArgList == NULL)
	{
		MessageBox(NULL, L"Unable to parse command line", L"Error", MB_OK);
		ExitProcess( -1);
	}
}
void JTAppInternal::InitArgs()
{
	if (FindArgInCommandLine(appArgList, appArgCount, L"-noInstall") != -1) appArgForceNoInstall = true;
	if (FindArgInCommandLine(appArgList, appArgCount, L"-rt") != -1) appArgForceTemp = true;

	int argFIndex = FindArgInCommandLine(appArgList, appArgCount, L"-f");
	if (argFIndex >= 0 && (argFIndex + 1) < appArgCount) {
		fullSourceInstallerPath = appArgList[argFIndex + 1];
		if (Path::Exists(fullSourceInstallerPath)) {
			WCHAR buffer[MAX_PATH];
			wcscpy_s(buffer, fullSourceInstallerPath.c_str());
			PathRenameExtension(buffer, L".ini");
			if (Path::Exists(fullSourceInstallerPath))
				fullIniPath = buffer;
		}
	}


}
void JTAppInternal::InitLogger()
{
	currentLogger = new LoggerInternal();
	appLogger = currentLogger;
	appLogger->SetLogOutPut(LogOutPutConsolne);
}
void JTAppInternal::InitPrivileges()
{
	SysHlp::EnableDebugPriv(SE_DEBUG_NAME);
	SysHlp::EnableDebugPriv(SE_SHUTDOWN_NAME);
	SysHlp::EnableDebugPriv(SE_LOAD_DRIVER_NAME);
}
void JTAppInternal::InitSettings()
{
	appSetting = new SettingHlp(fullIniPath.c_str());

	appArgeementArgeed = appSetting->GetSettingBool(L"Argeed", false, L"JTArgeement");
	appForceNoDriver = appSetting->GetSettingBool(L"DisableDriver", false);
	appForceNoSelfProtect = !appSetting->GetSettingBool(L"SelfProtect", true);
}
void JTAppInternal::InitPP()
{
	LoadLibrary(GetPartFullPath(PART_HOOKER));
}

HFONT JTAppInternal::hFontRed = NULL;
HINSTANCE JTAppInternal::hInstance = NULL;

INT_PTR CALLBACK JTAppInternal::ArgeementWndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;

	switch (message)
	{
	case WM_INITDIALOG: {
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP)));
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP)));

		hFontRed = CreateFontW(20, 0, 0, 0, 0, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"宋体");//创建字体
		SendDlgItemMessage(hDlg, IDC_STATIC_RED, WM_SETFONT, (WPARAM)hFontRed, TRUE);//发送设置字体消息

		lResult = TRUE;
		break;
	}
	case WM_COMMAND: EndDialog(hDlg, wParam); lResult = wParam;  break;
	case WM_CTLCOLORSTATIC: {
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_STATIC_RED))  SetTextColor((HDC)wParam, RGB(255, 0, 0));
		return (INT_PTR)GetStockObject(WHITE_BRUSH);
	}
	case WM_CTLCOLORDLG: {
		return (INT_PTR)(HBRUSH)GetStockObject(WHITE_BRUSH);
	}
	case WM_DESTROY: {
		DeleteObject(hFontRed);
		break;
	}
	default: return DefWindowProc(hDlg, message, wParam, lParam);
	}
	return lResult;
}


