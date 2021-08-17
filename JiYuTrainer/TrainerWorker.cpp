#include "stdafx.h"
#include "TrainerWorker.h"
#include "JiYuTrainer.h"
#include "AppPublic.h"
#include "NtHlp.h"
#include "PathHelper.h"
#include "StringHlp.h"
#include "MsgCenter.h"
#include "StringSplit.h"
#include "DriverLoader.h"
#include "KernelUtils.h"
#include "SysHlp.h"
#include "SettingHlp.h"
#include "RegHlp.h"

extern JTApp *currentApp;
extern LoggerInternal * currentLogger;

TrainerWorkerInternal * TrainerWorkerInternal::currentTrainerWorker = nullptr;

extern NtQuerySystemInformationFun NtQuerySystemInformation;

using namespace std;

#define TIMER_RESET_PID 40115
#define TIMER_CK 40116

PSYSTEM_PROCESSES current_system_process = NULL;

TrainerWorkerInternal::TrainerWorkerInternal()
{
	currentTrainerWorker = this;

	//For message box center
	hMsgBoxHook = SetWindowsHookEx(
		WH_CBT,        // Type of hook 
		CBTProc,        // Hook procedure (see below)
		NULL,         // Module handle. Must be NULL (see docs)
		GetCurrentThreadId()  // Only install for THIS thread!!!
	);
}
TrainerWorkerInternal::~TrainerWorkerInternal()
{
	if (hDesktop) {
		CloseDesktop(hDesktop);
		hDesktop = NULL;
	}

	UnhookWindowsHookEx(hMsgBoxHook);

	StopInternal();
	ClearProcess();

	currentTrainerWorker = nullptr;
}

void TrainerWorkerInternal::Init()
{
	hDesktop = OpenDesktop(L"Default", 0, FALSE, DESKTOP_ENUMERATE);
	UpdateScreenSize();

	if (LocateStudentMainLocation()) currentLogger->Log(L"已定位极域电子教室位置： %s", _StudentMainPath.c_str());
	else currentLogger->LogWarn(L"无法定位极域电子教室位置");

	UpdateState();
	UpdateStudentMainInfo(false);

	InitSettings();
}
void TrainerWorkerInternal::InitSettings()
{
	SettingHlp*settings = currentApp->GetSettings();
	setAutoIncludeFullWindow = settings->GetSettingBool(L"AutoIncludeFullWindow");
	setCkInterval = currentApp->GetSettings()->GetSettingInt(L"CKInterval", 3100);
	setAllowGbTop = settings->GetSettingBool(L"AllowGbTop", false);
	if (setCkInterval < 1000 || setCkInterval > 10000) setCkInterval = 3000;

	if (_StudentMainControlled)
		SendMessageToVirus(L"hk:reset");
}
void TrainerWorkerInternal::UpdateScreenSize()
{
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
}

void TrainerWorkerInternal::Start()
{
	if (!_Running) 
	{
		_Running = true;

		//Settings
		
		SetTimer(hWndMain, TIMER_CK, setCkInterval, TimerProc);
		SetTimer(hWndMain, TIMER_RESET_PID, 2000, TimerProc);

		UpdateState();

	}
}
void TrainerWorkerInternal::Stop()
{
	if (_Running) {
		StopInternal();
		UpdateState();
	}
}
void TrainerWorkerInternal::StopInternal() {
	if (_Running) {
		_Running = false;
		KillTimer(hWndMain, TIMER_CK);
		KillTimer(hWndMain, TIMER_RESET_PID);
	}
}

void TrainerWorkerInternal::SetUpdateInfoCallback(TrainerWorkerCallback *callback)
{
	if (callback) {
		_Callback = callback;
		hWndMain = callback->GetMainHWND();
	}
}

void TrainerWorkerInternal::HandleMessageFromVirus(LPCWSTR buf)
{
	wstring act(buf);
	vector<wstring> arr;
	SplitString(act, arr, L":");
	if (arr.size() >= 2)
	{
		if (arr[0] == L"hkb")
		{
			if (arr[1] == L"succ") {
				_StudentMainControlled = true;
				currentLogger->LogInfo(L"Receive ctl success message ");
				if (_Callback) _Callback->OnBeforeSendStartConf();
				UpdateState();

				//HS 
				FAST_STR_BINDER(str, L"hs:%ld", 32, (ULONG_PTR)hWndMain);
				SendMessageToVirus(str);
			}
			else if (arr[1] == L"immck") {
				RunCk();
				currentLogger->LogInfo(L"Receive  immck message ");
			}
			else if (arr[1] == L"jyk") {
				if (arr.size() >= 3) {
					_StudentMainRunningLock = arr[2] == L"1";
					UpdateState();
				}
			}
			else if (arr[1] == L"algbtop") {
				currentLogger->LogInfo(L"Receive allow gbTop message ");
				_Callback->OnAllowGbTop();
			}
			else if (arr[1] == L"gbmfull") {
				gbFullManual = true;
				ManualFull(gbFullManual);
			}
			else if (arr[1] == L"gbmnofull") {
				if (_FakeFull) FakeFull(false);
				gbFullManual = false;
				ManualFull(gbFullManual);
			}
			else if (arr[1] == L"gbuntop") ManualTop(false);
			else if (arr[1] == L"gbtop") ManualTop(true);
			else if (arr[1] == L"showhelp" && _Callback) _Callback->OnShowHelp();
			else if (arr[1] == L"wtf") {
				//插入了错误的目标，现在重置状态
				DWORD pid = _wtol(arr[2].c_str());
				AddFilterdPid(pid);

				if (_StudentMainFileLocatedByProcess)
					_StudentMainFileLocated = false;
				_StudentMainPid = 0;

				currentLogger->Log(L"插入了奇怪的 StudentMain.exe");

				UpdateState();
				UpdateStudentMainInfo(false);
			}
		}
		else if (arr[0] == L"wcd")
		{
			//wwcd
			int wcdc = _wtoi(arr[1].c_str());
			if (wcdc % 20 == 0)
				currentLogger->LogInfo(L"Receive watch dog message %d ", wcdc);
		}
		else if (arr[0] == L"vback") {
			wstring strBuff = arr[1];
			if (arr.size() > 2) {
				for (UINT i = 2; i < arr.size(); i++)
					strBuff += L":" + arr[i];
			}
			currentLogger->Log(L"Receive virus echo : %s", strBuff.c_str());
		}
	}
}
void TrainerWorkerInternal::SendMessageToVirus(LPCWSTR buf)
{
	MsgCenterSendToVirus(buf, hWndMain);
}

bool TrainerWorkerInternal::Kill(bool autoWork)
{
	if (_StudentMainPid <= 4) {
		currentLogger->LogError(L"未找到极域主进程");
		return false;
	}
	if (_StudentMainControlled){
		bool vkill = autoWork;
		if(!vkill){
			MSGBOXPARAMSW mbp = { 0 };
			mbp.hwndOwner = hWndMain;
			mbp.lpszCaption = L"JiYuTrainer - 提示";
			mbp.lpszText = L"您是否希望使用病毒进行爆破？";
			mbp.cbSize = sizeof(MSGBOXPARAMSW);
			mbp.dwStyle = MB_ICONEXCLAMATION | MB_TOPMOST | MB_YESNOCANCEL;
			int drs = MessageBoxIndirect(&mbp);
			if (drs == IDCANCEL) return false;
			else if (drs == IDYES) vkill = true;
		}
		if (vkill) {
			//Stop sginal
			SendMessageToVirus(L"ss2:0");
			return true;
		}
	}

	HANDLE hProcess;
	NTSTATUS status = MOpenProcessNt(_StudentMainPid, &hProcess);
	if (!NT_SUCCESS(status)) {
		if (status == STATUS_INVALID_CID || status == STATUS_INVALID_HANDLE) {
			_StudentMainPid = 0;
			_StudentMainControlled = false;
			UpdateState();
			UpdateStudentMainInfo(!autoWork);
			return true;
		}
		else {
			currentLogger->LogError(L"打开进程错误：0x%08X，请手动结束", status);
			return false;
		}
	}
	status = MTerminateProcessNt(0, hProcess);
	if (NT_SUCCESS(status)) {
		_StudentMainPid = 0;
		_StudentMainControlled = false;
		UpdateState();
		UpdateStudentMainInfo(!autoWork);
		CloseHandle(hProcess);
		return TRUE;
	}
	else {
		if (status == STATUS_ACCESS_DENIED) goto FORCEKILL;
		else if (status != STATUS_INVALID_CID && status != STATUS_INVALID_HANDLE) {
			currentLogger->LogError(L"结束进程错误：0x%08X，请手动结束", status);
			if (!autoWork)
				MessageBox(hWndMain, L"无法结束极域电子教室，您需要使用其他工具手动结束", L"JiYuTrainer - 错误", MB_ICONERROR);;
			CloseHandle(hProcess);
			return false;
		}
		else if (status == STATUS_INVALID_CID || status == STATUS_INVALID_HANDLE) {
			_StudentMainPid = 0;
			_StudentMainControlled = false;
			UpdateState();
			UpdateStudentMainInfo(!autoWork);
			CloseHandle(hProcess);
			return true;
		}
	}

FORCEKILL:
	if (XDriverLoaded() && MessageBox(hWndMain, L"普通无法结束极域，是否调用驱动结束极域？\n（驱动可能不稳定，请慎用。您也可以使用 PCHunter 等安全软件进行强杀）", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION | MB_YESNO | MB_TOPMOST) == IDYES)
	{
		if (KForceKill(_StudentMainPid, &status)) {
			_StudentMainPid = 0;
			_StudentMainControlled = false;
			UpdateState();
			UpdateStudentMainInfo(!autoWork);
			CloseHandle(hProcess);
			return true;
		}
		else if(!autoWork) MessageBox(hWndMain, L"驱动也无法结束，请使用 PCHunter 结束它吧！", L"错误", MB_ICONEXCLAMATION);
	}
	CloseHandle(hProcess);
	return false;
}
bool TrainerWorkerInternal::KillStAuto()
{
	RunCk();
	RunResetPid();

	return Kill(true);
}
bool TrainerWorkerInternal::Rerun(bool autoWork)
{
	if (!_StudentMainFileLocated) {
		currentLogger->LogWarn(L"未找到极域电子教室");
		if (!autoWork && _Callback)
			_Callback->OnSimpleMessageCallback(L"<h5>我们无法在此计算机上找到极域电子教室，您需要手动启动</h5>");
		return false;
	}
	return  SysHlp::RunApplication(_StudentMainPath.c_str(), NULL);
}
void* TrainerWorkerInternal::RunOperation(TrainerWorkerOp op) 
{
	switch (op)
	{
	case TrainerWorkerOpVirusBoom: {
		MsgCenterSendToVirus(L"ss:0", hWndMain);
		return nullptr;
	}
	case TrainerWorkerOpVirusQuit: {
		MsgCenterSendToVirus((LPWSTR)L"hk:ckend", hWndMain);
		return nullptr;
	}
	case TrainerWorkerOp1: {
		WCHAR s[300]; swprintf_s(s, L"hk:path:%s", currentApp->GetFullPath());
		MsgCenterSendToVirus(s, hWndMain);
		swprintf_s(s, L"hk:inipath:%s", currentApp->GetPartFullPath(PART_INI));
		MsgCenterSendToVirus(s, hWndMain);
		break;
	}
	case TrainerWorkerOpForceUnLoadVirus: {
		UnLoadAllVirus();
		break;
	}
	case TrainerWorkerOp2: {
		if (ReadTopDomanPassword(false))
			return (LPVOID)_TopDomainPassword.c_str();
		return nullptr;
	}
	case TrainerWorkerOp3: {
		if (ReadTopDomanPassword(true)) 
			return (LPVOID)_TopDomainPassword.c_str();
		return nullptr;
	}
	case TrainerWorkerOp4: {
		return (void*)UnLoadJiYuProtectDriver();
	}
	case TrainerWorkerOp5: {
		return (void*) UnLoadNetFilterDriver();
	}
	}
	return nullptr;
}

bool TrainerWorkerInternal::RunCk()
{
	_LastResolveWindowCount = 0;
	_LastResoveBroadcastWindow = false;
	_LastResoveBlackScreenWindow = false;
	_FirstBlackScreenWindow = false;

	EnumDesktopWindows(hDesktop, EnumWindowsProc, (LPARAM)this);

	MsgCenterSendHWNDS(hWndMain);

	return _LastResolveWindowCount > 0;
}
void TrainerWorkerInternal::RunResetPid()
{
	FlushProcess();

	//CK GET STAT DELAY
	if (_NextLoopGetCkStat) {
		_NextLoopGetCkStat = false;
		SendMessageToVirus(L"hk:ckstat");
	}

	//Find jiyu main process
	DWORD newPid = 0;
	if (LocateStudentMain(&newPid)) { //找到极域

		if (_StudentMainPid != newPid)
		{
			_StudentMainPid = newPid;

			if (InstallVirus()) {
				_VirusInstalled = true;
				_NextLoopGetCkStat = true;

				currentLogger->Log(L"向 StudentMain.exe [%d] 注入DLL成功", newPid);
			}
			else  currentLogger->LogError(L"向 StudentMain.exe [%d] 注入DLL失败", newPid);

			currentLogger->Log(L"已锁定 StudentMain.exe [%d]", newPid);

			UpdateState();
			UpdateStudentMainInfo(false);
		}
	}
	else { //没有找到

		if (_StudentMainPid != 0)
		{
			_StudentMainPid = 0;

			currentLogger->Log(L"极域主进程 StudentMain.exe 已退出", newPid);

			UpdateState();
			UpdateStudentMainInfo(false);
		}

	}

	/*
	newPid = 0;
	if (LocateMasterHelper(&newPid)) {
		if (_MasterHelperPid != newPid)
		{
			_MasterHelperPid = newPid;
			if (InstallVirusForMaster()) currentLogger->Log(L"向 MasterHelper.exe [%d] 注入DLL成功", newPid);
			else  currentLogger->LogError(L"向 MasterHelper.exe [%d] 注入DLL失败", newPid);
		}
	}
	else {
		_MasterHelperPid = 0;
	}
	*/
}

bool TrainerWorkerInternal::FlushProcess()
{
	ClearProcess();

	DWORD dwSize = 0;
	NTSTATUS status = NtQuerySystemInformation(SystemProcessInformation, NULL, 0, &dwSize);
	if (status == STATUS_INFO_LENGTH_MISMATCH && dwSize > 0)
	{
		current_system_process = (PSYSTEM_PROCESSES)malloc(dwSize);
		status = NtQuerySystemInformation(SystemProcessInformation, current_system_process, dwSize, 0);
		if (!NT_SUCCESS(status)) {
			currentLogger->LogError2(L"NtQuerySystemInformation failed ! 0x%08X", status);
			return false;
		}
	}

	return true;
}
void TrainerWorkerInternal::ClearProcess()
{
	if (current_system_process) {
		free(current_system_process);
		current_system_process = NULL;
	}
}
bool TrainerWorkerInternal::FindProcess(LPCWSTR processName, DWORD * outPid)
{
	return false;
}
bool TrainerWorkerInternal::KillProcess(DWORD pid, bool force)
{
	HANDLE hProcess;
	NTSTATUS status = MOpenProcessNt(_StudentMainPid, &hProcess);
	if (!NT_SUCCESS(status)) {
		if (status == STATUS_INVALID_CID || status == STATUS_INVALID_HANDLE) {
			currentLogger->LogError2(L"找不到进程 [%d] ", pid);
			return true;
		}
		else {
			currentLogger->LogError2(L"打开进程 [%d] 错误：0x%08X，请手动结束", pid);
			return false;
		}
	}
	status = MTerminateProcessNt(0, hProcess);
	if (NT_SUCCESS(status)) {
		currentLogger->Log(L"进程 [%d] 结束成功", pid);
		CloseHandle(hProcess);
		return TRUE;
	}
	else {
		if (status == STATUS_ACCESS_DENIED) {
			if (force) goto FORCEKILL;
			else currentLogger->LogError2(L"结束进程 [%d] 错误：拒绝访问。可尝试使用驱动结束", pid);
			CloseHandle(hProcess);
		}
		else if (status != STATUS_INVALID_CID && status != STATUS_INVALID_HANDLE) {
			currentLogger->LogError2(L"结束进程 [%d] 错误：0x%08X，请手动结束", pid);
			CloseHandle(hProcess);
			return false;
		}
		else if (status == STATUS_INVALID_CID || status == STATUS_INVALID_HANDLE) {
			currentLogger->LogError2(L"找不到进程 [%d] ", pid);
			CloseHandle(hProcess);
			return true;
		}
	}
FORCEKILL:
	if (XDriverLoaded())
	{
		if (KForceKill(_StudentMainPid, &status)) {
			currentLogger->Log(L"进程 [%d] 强制结束成功", pid);
			CloseHandle(hProcess);
			return true;
		}
		else {
			currentLogger->LogError2(L"驱动强制结束进程 [%d] 错误：0x%08X", pid);
		}
	}
	else currentLogger->Log(L"驱动未加载，无法强制结束进程");
	CloseHandle(hProcess);
	return false;
}
bool TrainerWorkerInternal::ReadTopDomanPassword(BOOL forceKnock)
{
	_TopDomainPassword.clear();

	HKEY hKey;
	LRESULT lastError;

	if (forceKnock) goto READ_EX;
	//普通注册表读取，适用于4.0版本

	WCHAR Data[32];
	if (MRegReadKeyString64And32(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\TopDomain\\e-Learning Class Standard\\1.00",
		L"SOFTWARE\\Wow6432Node\\TopDomain\\e-Learning Class Standard\\1.00", L"UninstallPasswd", Data, 32)) {
		
		if (StrEqual(Data, L"Passwd[123456]")) goto READ_EX; //6.0以后读取不了了，都显示Passwd[123456]，用新的方法读取
		else {
			_TopDomainPassword = Data;
			_TopDomainPassword = _TopDomainPassword.substr(6, _TopDomainPassword.size() - 6);
			return true;
		}

	}
	else currentLogger->LogWarn2(L"MRegReadKeyString64And32 Failed : %s (%d)", PRINT_LAST_ERROR_STR);

	//HKEY_LOCAL_MACHINE\SOFTWARE\TopDomain\e-Learning Class\Student Knock1
READ_EX:

	lastError = RegOpenKeyEx(HKEY_LOCAL_MACHINE, SysHlp::Is64BitOS() ?
		L"SOFTWARE\\Wow6432Node\\TopDomain\\e-Learning Class\\Student" : 
		L"SOFTWARE\\TopDomain\\e-Learning Class\\Student", 0, KEY_WOW64_64KEY | KEY_READ, &hKey);
	if (lastError == ERROR_SUCCESS) {

		DWORD dwType = REG_BINARY;
		BYTE Data[120];
		DWORD cbData = 120;
		lastError = RegQueryValueEx(hKey, L"Knock1", 0, &dwType, (LPBYTE)Data, &cbData);
		if (lastError == ERROR_SUCCESS) {

			RegCloseKey(hKey);

			WCHAR ss[34] = { 0 };
			if (UnDecryptJiyuKnock(Data, cbData, ss)) {
				_TopDomainPassword = ss;
				return true;
			}
			else {
				currentLogger->LogWarn2(L"UnDecryptJiyuKnock failed !");
				return false;
			}
		}
		else currentLogger->LogWarn2(L"RegQueryValueEx Failed : %d", lastError);
		RegCloseKey(hKey);
	}
	else currentLogger->LogWarn2(L"RegOpenKeyEx Failed : %d", lastError);
	return false;
}
bool TrainerWorkerInternal::AppointStudentMainLocation(LPCWSTR fullPath) {
	if (Path::GetFileName(fullPath) != L"StudentMain.exe")	return false;
	if (Path::Exists(fullPath)) 
	{
		_StudentMainPath = fullPath;
		_StudentMainFileLocated = true;
		_StudentMainFileLocatedByProcess = false;

		currentApp->GetSettings()->SetSettingStr(L"StudentMainPath", fullPath);
		currentLogger->Log(L"成功手动定位极域电子教室位置： %s", fullPath);

		UpdateStudentMainInfo(false);
		return true;
	}
	return false;
}
bool TrainerWorkerInternal::LocateStudentMainLocation()
{
	//注册表查找 极域 路径
	WCHAR Data[MAX_PATH];
	if (MRegReadKeyString64And32(HKEY_LOCAL_MACHINE,
		L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\e-Learning Class V6.0",
		L"SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\e-Learning Class V6.0", 
		L"DisplayIcon", Data, MAX_PATH)) {

		if (Path::Exists(Data)) {
			_StudentMainPath = Data;
			_StudentMainFileLocated = true;
			return true;
		}
		else currentLogger->Log(L"读取注册表 [DisplayIcon] 获得了一个无效的极域电子教室路径 : %s", Data);
	}
	else currentLogger->LogWarn2(L"MRegReadKeyString64And32 Failed : %s (%d)", PRINT_LAST_ERROR_STR);

	//读取用户指定的路径
	wstring appointStudentMainPath  = currentApp->GetSettings()->GetSettingStr(L"StudentMainPath", L"", MAX_PATH);
	if (!StrEmepty(appointStudentMainPath.c_str()) && Path::Exists(appointStudentMainPath)) {
		wstring fileName = Path::GetFileName(appointStudentMainPath);
		if (StrEqual(fileName.c_str(), L"StudentMain.exe")) {
			_StudentMainPath = appointStudentMainPath;
			_StudentMainFileLocated = true;
			return true;
		}
	}

	//直接尝试查找
	LPCWSTR mabeInHere[6] = {
		L"c:\\Program Files\\Mythware\\极域课堂管理系统软件V6.0 2016 豪华版\\StudentMain.exe",
		L"C:\\Program Files\\Mythware\\e-Learning Class\\StudentMain.exe",
		L"C:\\Program Files (x86)\\Mythware\\极域课堂管理系统软件V6.0 2016 豪华版\\StudentMain.exe",
		L"C:\\Program Files (x86)\\Mythware\\e - Learning Class\\StudentMain.exe",
		L"C:\\e-Learning Class\\StudentMain.exe",
		L"c:\\极域课堂管理系统软件V6.0 2016 豪华版\\StudentMain.exe",
	};
	for (int i = 0; i < 6; i++) {
		if (Path::Exists(mabeInHere[i])) {
			_StudentMainPath = mabeInHere[i];
			_StudentMainFileLocated = true;
			_StudentMainFileLocatedByProcess = false;
			return true;
		}
	}

	return false;
}
bool TrainerWorkerInternal::LocateStudentMain(DWORD *outFirstPid)
{
	if (current_system_process)
	{
		bool done = false;
		for (PSYSTEM_PROCESSES p = current_system_process; !done; p = PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryOffset)) {
			if (p->ImageName.Length && StrEqual(p->ImageName.Buffer, L"StudentMain.exe"))
			{
				if (CheckPidFilterd((DWORD)p->ProcessId)) continue;

				HANDLE hProcess;
				if (NT_SUCCESS(MOpenProcessNt((DWORD)p->ProcessId, &hProcess))) {
					WCHAR buffer[MAX_PATH];
					if (MGetProcessFullPathEx(hProcess, buffer)) {
						//检查exe相同目录下是否存在 LibTDMaster.dll 不存在则排除
						PathRemoveFileSpec(buffer);
						wcscat_s(buffer, L"\\LibTDMaster.dll");
						if (!PathFileExists(buffer)) {
							AddFilterdPid((DWORD)p->ProcessId);
							currentLogger->Log(L"奇怪的 StudentMain.exe [%d]", (DWORD)p->ProcessId);
							continue;
						}

						//Exe 确定位置
						if (!_StudentMainFileLocated) {
							currentLogger->Log(L"通过进程 StudentMain.exe [%d] 定位到位置： %s", (DWORD)p->ProcessId, _StudentMainPath);
							_StudentMainPath = buffer;
							_StudentMainFileLocated = true;
							_StudentMainFileLocatedByProcess = true;
						}
					}
					CloseHandle(hProcess);
				}

				if (outFirstPid) *outFirstPid = (DWORD)p->ProcessId;

				ClearFilterdPid();
				return true;
			}
			done = p->NextEntryOffset == 0;
		}
		ClearFilterdPid();
	}
	return false;
}
bool TrainerWorkerInternal::LocateMasterHelper(DWORD *outFirstPid)
{
	if (current_system_process)
	{
		bool done = false;
		for (PSYSTEM_PROCESSES p = current_system_process; !done; p = PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryOffset)) {
			if (p->ImageName.Length && StrEqual(p->ImageName.Buffer, L"MasterHelper.exe"))
			{
				if (outFirstPid)*outFirstPid = (DWORD)p->ProcessId;
				return true;
			}
			done = p->NextEntryOffset == 0;
		}
	}
	return false;
}
bool TrainerWorkerInternal::CheckPidFilterd(DWORD pid)
{
	for (auto it = incorrectStudentMainPids.begin(); it != incorrectStudentMainPids.end(); it++) {
		if ((*it).pid == pid)
			return true;
	}
	return false;
}
void TrainerWorkerInternal::AddFilterdPid(DWORD pid)
{
	IncorrectStudentMainFilterData data;
	data.checked = true;
	data.pid = pid;
	incorrectStudentMainPids.push_back(data);
}
void TrainerWorkerInternal::ClearFilterdPid()
{
	if (current_system_process)
	{
		for (auto it = incorrectStudentMainPids.begin(); it != incorrectStudentMainPids.end(); it++) {
			auto pid = (*it).pid;
			bool founded = false;
			bool done = false;
			for (PSYSTEM_PROCESSES p = current_system_process; !done; p = PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryOffset)) {
				if ((DWORD)p->ProcessId == pid)
				{
					founded = true;
					break;
				}
				done = p->NextEntryOffset == 0;
			}
			if (!founded) 
				it = incorrectStudentMainPids.erase(it);
		}
	}
}


void TrainerWorkerInternal::UpdateStudentMainInfo(bool byUser)
{
	if (_Callback)
		_Callback->OnUpdateStudentMainInfo(_StudentMainPid > 4, _StudentMainPath.c_str(), _StudentMainPid, byUser);
}
void TrainerWorkerInternal::UpdateState()
{
	if (_Callback) 
	{
		TrainerWorkerCallback::TrainerStatus status;
		if (_StudentMainPid > 4) {
			if (_StudentMainControlled) {
				_StatusTextMain = L"已控制极域电子教室";

				if (!_Running) {
					_StatusTextMain += L" 但控制器未启动";
					status = TrainerWorkerCallback::TrainerStatus::TrainerStatusStopped;
				}
				else if (_StudentMainRunningLock)
				{
					_StatusTextMore = L"已为您解锁极域电子教室<br/>您可以放心继续您的工作";
					status = TrainerWorkerCallback::TrainerStatus::TrainerStatusControlledAndUnLocked;
				}
				else {
					_StatusTextMore = L"您可以放心继续您的工作";
					status = TrainerWorkerCallback::TrainerStatus::TrainerStatusControlled;
				}
			}
			else {
				_StatusTextMain = L"无法控制极域电子教室";
				if (!_Running) {
					_StatusTextMain = L"控制器未启动";
					_StatusTextMore = L"您已手动停止控制器<br / >当前不会对极域做任何操作";
					status = TrainerWorkerCallback::TrainerStatus::TrainerStatusStopped;
				}
				else if (_VirusInstalled) {
					_StatusTextMore = L"毒已插入极域，但未正常运行<br / ><span style=\"color:#f41702\">您可能需要重新启动极域</span>";
					status = TrainerWorkerCallback::TrainerStatus::TrainerStatusUnknowProblem;
				}
				else {
					_StatusTextMore = L"向极域电子教室插入病毒失败<br / >错误详情请查看 <a id=\"link_log\">日志</a>";
					status = TrainerWorkerCallback::TrainerStatus::TrainerStatusControllFailed;
				}
			}
		}
		else {
			_StatusTextMain = L"极域电子教室未运行";
			if (!_Running) {
				_StatusTextMain = L"极域电子教室未运行 并且控制器未启动";
				_StatusTextMore = L"您已手动停止控制器<br / >当前不会检测极域的运行";
				status = TrainerWorkerCallback::TrainerStatus::TrainerStatusStopped;
			}
			else if (_StudentMainFileLocated) {
				status = TrainerWorkerCallback::TrainerStatus::TrainerStatusNotRunning;
				_StatusTextMore = L"已在此计算机上找到极域电子教室<br / >你可以点击 <b>下方按钮</b> 运行它";
			}
			else {
				status = TrainerWorkerCallback::TrainerStatus::TrainerStatusNotFound;
				_StatusTextMore = L"未在此计算机上找到极域电子教室";
			}
		}

		_Callback->OnUpdateState(status, _StatusTextMain.c_str(), _StatusTextMore.c_str());
	}
}

bool TrainerWorkerInternal::InstallVirus()
{
	return InjectDll(_StudentMainPid, currentApp->GetPartFullPath(PART_HOOKER));
}
bool TrainerWorkerInternal::InstallVirusForMaster()
{
	return InjectDll(_MasterHelperPid, currentApp->GetPartFullPath(PART_HOOKER));
}
bool TrainerWorkerInternal::InjectDll(DWORD pid, LPCWSTR dllPath)
{
	HANDLE hRemoteProcess;
	//打开进程
	NTSTATUS ntStatus = MOpenProcessNt(pid, &hRemoteProcess);
	if (!NT_SUCCESS(ntStatus)) {
		currentLogger->LogError2(L"注入病毒失败！打开进程失败：0x%08X", ntStatus);
		return FALSE;
	}

	wchar_t *pszLibFileRemote;

	//使用VirtualAllocEx函数在远程进程的内存地址空间分配DLL文件名空间
	pszLibFileRemote = (wchar_t *)VirtualAllocEx(hRemoteProcess, NULL, sizeof(wchar_t) * (lstrlen(dllPath) + 1), MEM_COMMIT, PAGE_READWRITE);

	//使用WriteProcessMemory函数将DLL的路径名写入到远程进程的内存空间
	WriteProcessMemory(hRemoteProcess, pszLibFileRemote, (void *)dllPath, sizeof(wchar_t) * (lstrlen(dllPath) + 1), NULL);

	//##############################################################################
		//计算LoadLibraryA的入口地址
	PTHREAD_START_ROUTINE pfnStartAddr = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "LoadLibraryW");
	//(关于GetModuleHandle函数和GetProcAddress函数)

	//启动远程线程LoadLibraryW，通过远程线程调用创建新的线程
	HANDLE hRemoteThread;
	if ((hRemoteThread = CreateRemoteThread(hRemoteProcess, NULL, 0, pfnStartAddr, pszLibFileRemote, 0, NULL)) == NULL)
	{
		currentLogger->LogError2(L"注入线程失败! 错误：CreateRemoteThread %d", GetLastError());
		return FALSE;
	}

	// 释放句柄

	CloseHandle(hRemoteProcess);
	CloseHandle(hRemoteThread);

	return true;
}
bool TrainerWorkerInternal::UnInjectDll(DWORD pid, LPCWSTR moduleName)
{
	HANDLE hProcess;
	//打开进程
	NTSTATUS ntStatus = MOpenProcessNt(pid, &hProcess);
	if (!NT_SUCCESS(ntStatus)) {
		currentLogger->LogError2(L"卸载病毒失败！打开进程失败：0x%08X", ntStatus);
		return FALSE;
	}
	DWORD pszLibFileRemoteSize = sizeof(wchar_t) * (lstrlen(moduleName) + 1);
	wchar_t *pszLibFileRemote;
	//使用VirtualAllocEx函数在远程进程的内存地址空间分配DLL文件名空间
	pszLibFileRemote = (wchar_t *)VirtualAllocEx(hProcess, NULL, pszLibFileRemoteSize, MEM_COMMIT, PAGE_READWRITE);
	//使用WriteProcessMemory函数将DLL的路径名写入到远程进程的内存空间
	WriteProcessMemory(hProcess, pszLibFileRemote, (void *)moduleName, pszLibFileRemoteSize, NULL);

	DWORD dwHandle;
	DWORD dwID;
	LPVOID pFunc = GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "GetModuleHandleW");
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, pszLibFileRemote, 0, &dwID);
	if (!hThread) {
		currentLogger->LogError2(L"卸载病毒失败！创建远程线程失败：%d", GetLastError());
		return FALSE;
	}

	// 等待GetModuleHandle运行完毕
	WaitForSingleObject(hThread, INFINITE);
	// 获得GetModuleHandle的返回值
	GetExitCodeThread(hThread, &dwHandle);
	// 释放目标进程中申请的空间
	VirtualFreeEx(hProcess, pszLibFileRemote, pszLibFileRemoteSize, MEM_DECOMMIT);
	CloseHandle(hThread);
	// 使目标进程调用FreeLibrary，卸载DLL
	pFunc = GetProcAddress(GetModuleHandle(TEXT("Kernel32")), "FreeLibrary"); ;
	hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)pFunc, (LPVOID)dwHandle, 0, &dwID);
	if (!hThread) {
		currentLogger->LogError2(L"卸载病毒失败！创建远程线程失败：%d", GetLastError());
		return FALSE;
	}
	
	// 等待FreeLibrary卸载完毕
	WaitForSingleObject(hThread, INFINITE);
	CloseHandle(hThread);
	CloseHandle(hProcess);

	return true;
}
bool TrainerWorkerInternal::UnLoadAllVirus()
{
	if (_MasterHelperPid > 4) {
		if (UnInjectDll(_MasterHelperPid, L"JiYuTrainerHooks.dll"))
			currentLogger->Log(L"已强制卸载 MasterHelper 病毒");
		//KillProcess(_MasterHelperPid, false);
	}
	if (_StudentMainPid > 4)
		if (UnInjectDll(_StudentMainPid, L"JiYuTrainerHooks.dll"))
			currentLogger->Log(L"已强制卸载 StudentMain 病毒");

	return false;
}
bool TrainerWorkerInternal::UnLoadNetFilterDriver()
{
	return MUnLoadDriverServiceWithMessage(L"TDNetFilter");
}
bool TrainerWorkerInternal::UnLoadJiYuProtectDriver()
{
	return false;
}

bool TrainerWorkerInternal::SwitchFakeFull()
{
	if (_FakeFull) { 
		_FakeFull = false; 
		SendMessageToVirus(L"hk:fkfull:false");
		FakeFull(_FakeFull);
	}
	else if(_StudentMainRunningLock) { 
		_FakeFull = true; 
		SendMessageToVirus(L"hk:fkfull:true");
		FakeFull(_FakeFull);
	}

	return _FakeFull;
}
void TrainerWorkerInternal::FakeFull(bool fk) {
	if (_CurrentBroadcastWnd)
	{
		if (fk) {
			SetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE, GetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
			SetWindowLong(_CurrentBroadcastWnd, GWL_STYLE, GetWindowLong(_CurrentBroadcastWnd, GWL_STYLE) ^ (WS_BORDER | WS_OVERLAPPEDWINDOW));
			SetWindowPos(_CurrentBroadcastWnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
			SendMessage(_CurrentBroadcastWnd, WM_SIZE, 0, MAKEWPARAM(screenWidth, screenHeight));
			/*HWND jiYuGBDeskRdWnd = FindWindowExW(currentGbWnd, NULL, NULL, L"TDDesk Render Window");
			if (jiYuGBDeskRdWnd != NULL) {

			}*/
			_FakeBroadcastFull = true;
			currentLogger->Log(L"调整广播窗口假装全屏状态");
		}
		else {
			_FakeBroadcastFull = false;
			FixWindow(_CurrentBroadcastWnd, (LPWSTR)L"屏幕广播");
			int w = (int)((double)screenWidth * (3.0 / 4.0)), h = (int)((double)screenHeight * (double)(4.0 / 5.0));
			SetWindowPos(_CurrentBroadcastWnd, 0, (screenWidth - w) / 2, (screenHeight - h) / 2, w, h, SWP_NOZORDER | SWP_SHOWWINDOW);
			currentLogger->Log(L"取消广播窗口假装全屏状态");
		}
	}
	if (_CurrentBlackScreenWnd) 
	{
		if (fk) {
			SetWindowLong(_CurrentBlackScreenWnd, GWL_EXSTYLE, GetWindowLong(_CurrentBlackScreenWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
			SetWindowLong(_CurrentBlackScreenWnd, GWL_STYLE, GetWindowLong(_CurrentBlackScreenWnd, GWL_STYLE) ^ (WS_BORDER | WS_OVERLAPPEDWINDOW));
			SetWindowPos(_CurrentBlackScreenWnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
			SendMessage(_CurrentBlackScreenWnd, WM_SIZE, 0, MAKEWPARAM(screenWidth, screenHeight));
			SendMessage(_CurrentBlackScreenWnd, WM_SIZE, 0, MAKEWPARAM(screenWidth, screenHeight));
			_FakeBlackScreenFull = true;
			currentLogger->Log(L"调整黑屏窗口假装全屏状态");
		}
		else {
			_FakeBlackScreenFull = false;
			FixWindow(_CurrentBlackScreenWnd, (LPWSTR)L"BlackScreen Window");
			currentLogger->Log(L"取消黑屏窗口假装全屏状态");
		}
	}
	if (!fk && !_CurrentBlackScreenWnd && !_CurrentBroadcastWnd && (_FakeBlackScreenFull || _FakeBroadcastFull)) {
		_FakeBroadcastFull = false;
		_FakeBlackScreenFull = false;
	}
}
void TrainerWorkerInternal::ManualFull(bool fk)
{
	currentLogger->Log(L"receive ManualFull %s", fk ? L"true" : L"false");
	if (_CurrentBroadcastWnd) 
	{
		if (fk) {
			SetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE, GetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
			SetWindowLong(_CurrentBroadcastWnd, GWL_STYLE, GetWindowLong(_CurrentBroadcastWnd, GWL_STYLE) ^ (WS_BORDER | WS_OVERLAPPEDWINDOW) | WS_SYSMENU);
			SetWindowPos(_CurrentBroadcastWnd, HWND_TOPMOST, 0, 0, screenWidth, screenHeight, SWP_SHOWWINDOW);
			SendMessage(_CurrentBroadcastWnd, WM_SIZE, 0, MAKEWPARAM(screenWidth, screenHeight));
		}
		else {
			FixWindow(_CurrentBroadcastWnd, (LPWSTR)L"屏幕广播");
			int w = (int)((double)screenWidth * (3.0 / 4.0)), h = (int)((double)screenHeight * (double)(4.0 / 5.0));
			SetWindowPos(_CurrentBroadcastWnd, 0, (screenWidth - w) / 2, (screenHeight - h) / 2, w, h, SWP_NOZORDER | SWP_SHOWWINDOW);
		}
	}
}
void TrainerWorkerInternal::ManualTop(bool fk)
{
	currentLogger->Log(L"receive ManualTop %s", fk ? L"true" : L"false");
	if (_CurrentBroadcastWnd)
	{
		if (fk) {
			SetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE, GetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE) | WS_EX_TOPMOST);
			SetWindowPos(_CurrentBroadcastWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		else {
			SetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE, GetWindowLong(_CurrentBroadcastWnd, GWL_EXSTYLE) ^ WS_EX_TOPMOST);
			SetWindowPos(_CurrentBroadcastWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
	}
}
bool TrainerWorkerInternal::ChecIsJIYuWindow(HWND hWnd, LPDWORD outPid, LPDWORD outTid) {
	if (_StudentMainPid == 0) return false;
	DWORD pid = 0, tid = GetWindowThreadProcessId(hWnd, &pid);
	if (outPid) *outPid = pid;
	if (outTid) *outTid = tid;
	return pid == _StudentMainPid;
}
bool TrainerWorkerInternal::CheckIsTargetWindow(LPWSTR text, HWND hWnd) {
	bool b = false;
	
	if (StrEqual(text, L"BlackScreen Window")) {
		b = true;
		_LastResoveBlackScreenWindow = true;
		if (!_FirstBlackScreenWindow) {
			_FirstBlackScreenWindow = true;
			if (_Callback) _Callback->OnResolveBlackScreenWindow();
		}
		_CurrentBlackScreenWnd = hWnd;
		if (_FakeBlackScreenFull) return false;
	}
	else if (CheckWindowTextIsGb(text)) {
		b = true;
		_LastResoveBroadcastWindow = true;
		_CurrentBroadcastWnd = hWnd;
		if (_FakeBroadcastFull) return false;
	}
	return b;
}
void TrainerWorkerInternal::FixWindow(HWND hWnd, LPWSTR text)
{
	_LastResolveWindowCount++;

	LONG oldLong = GetWindowLong(hWnd, GWL_EXSTYLE);

	if (StrEqual(text, L"BlackScreen Window"))
	{
		oldLong = GetWindowLong(hWnd, GWL_EXSTYLE);
		{
			SetWindowLong(hWnd, GWL_EXSTYLE, oldLong ^ WS_EX_APPWINDOW | WS_EX_NOACTIVATE);
			SetWindowPos(hWnd, 0, 20, 20, 90, 150, SWP_NOZORDER | SWP_DRAWFRAME | SWP_NOACTIVATE);
			ShowWindow(hWnd, SW_HIDE);
		}
	}
	//Un top
	if (CheckWindowTextIsGb(text))
	{
		if (!setAllowGbTop && (oldLong & WS_EX_TOPMOST) == WS_EX_TOPMOST)
		{
			SetWindowLong(hWnd, GWL_EXSTYLE, oldLong ^ WS_EX_TOPMOST);
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}
		if (!gbFullManual) 
		{
			//Set border and sizeable
			SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | (WS_BORDER | WS_OVERLAPPEDWINDOW));
		}
	}
	else if(setAutoIncludeFullWindow)
	{
		if ((oldLong & WS_EX_TOPMOST) == WS_EX_TOPMOST)
		{
			SetWindowLong(hWnd, GWL_EXSTYLE, oldLong ^ WS_EX_TOPMOST);
			SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
		}

		//Set border and sizeable
		SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | (WS_BORDER | WS_OVERLAPPEDWINDOW));

	}

	SetWindowPos(hWnd, 0, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOMOVE | SWP_DRAWFRAME | SWP_NOACTIVATE);
}
bool TrainerWorkerInternal::CheckWindowTextIsGb(const wchar_t* text) {
	return StringHlp::StrContainsW(text, L"广播", nullptr) || StringHlp::StrContainsW(text, L"演示", nullptr)
		|| StringHlp::StrContainsW(text, L"共享", nullptr)
		|| StringHlp::StrEqualW(text, L"屏幕演播室窗口");
}

BOOL CALLBACK TrainerWorkerInternal::EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	TrainerWorkerInternal *self =(TrainerWorkerInternal *)lParam;
	if (IsWindowVisible(hWnd) && self->ChecIsJIYuWindow(hWnd, NULL, NULL)) {
		WCHAR text[50];
		GetWindowText(hWnd, text, 50);
		if (StrEqual(text, L"JiYu Trainer Virus Window")) return TRUE;

		RECT rc;
		GetWindowRect(hWnd, &rc);
		if (self->CheckIsTargetWindow(text, hWnd)) {
			//JiYu window
			MsgCenteAppendHWND(hWnd);
			self->FixWindow(hWnd, text);
		}
		else if (self->setAutoIncludeFullWindow && rc.top == 0 && rc.left == 0 && rc.right == self->screenWidth && rc.bottom == self->screenHeight) {
			//Full window
			MsgCenteAppendHWND(hWnd);
			self->FixWindow(hWnd, text);
		}
	}
	return TRUE;
}
VOID CALLBACK TrainerWorkerInternal::TimerProc(HWND hWnd, UINT message, UINT_PTR iTimerID, DWORD dwTime)
{
	if (currentTrainerWorker != nullptr) 
	{
		if (iTimerID == TIMER_RESET_PID) {
			currentTrainerWorker->RunResetPid();
		}
		if (iTimerID == TIMER_CK) {
			currentTrainerWorker->RunCk();
		}
	}
}
LRESULT CALLBACK TrainerWorkerInternal::CBTProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0)
		return CallNextHookEx(currentTrainerWorker->hMsgBoxHook, nCode, wParam, lParam);
	switch (nCode)
	{
	case HCBT_ACTIVATE: {
		// 现在wParam中就是message box的句柄
		HWND hWnd = (HWND)wParam;
		HWND hWndOwner = GetWindow(hWnd, GW_OWNER);

		// 我们已经有了message box的句柄，在这里我们就可以定制message box了!
		if (hWndOwner && hWndOwner  == currentTrainerWorker->hWndMain)
		{
			//窗口在父窗口居中
			RECT rect; GetWindowRect(hWnd, &rect);
			RECT rectParent; GetWindowRect(hWndOwner, &rectParent);
			rect.left = ((rectParent.right - rectParent.left) - (rect.right - rect.left)) / 2 + rectParent.left;
			rect.top = ((rectParent.bottom - rectParent.top) - (rect.bottom - rect.top)) / 2 + rectParent.top;
			SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}
		return 0;
	}
	}
	// Call the next hook, if there is one
	return CallNextHookEx(currentTrainerWorker->hMsgBoxHook, nCode, wParam, lParam);
}

bool UnDecryptJiyuKnock(BYTE* Data, DWORD cbData, WCHAR* ss)
{
	//反编译的代码
	__try {
		DWORD v5; // esi
		DWORD v6; // ecx
		BYTE *v7; // eax
		DWORD v8; // edx
		BYTE *i;
		v5 = cbData;
		v6 = cbData >> 2;
		v7 = Data;
		if (cbData >> 2)
		{
			v8 = cbData >> 2;
			do
			{
				*(DWORD *)v7 ^= 0x50434C45u;
				v7 += 4;
				--v8;
			} while (v8);
		}
		for (i = Data; v6; --v6)
		{
			*(DWORD *)i ^= 0x454C4350u;
			i += 4;
		}
		WORD v4[34];
		v4[0] = 0;
		memset(&v4[1], 0, 0x40u);

		int a1 = (int)&v4;

		int v13; // edi
		BYTE *v14; // eax
		__int16 v15; // cx
		v13 = a1 - Data[0];
		v14 = &Data[Data[0]];
		do
		{
			v15 = *(WORD *)v14;
			*(WORD *)&v14[v13 - (DWORD)Data] = *(WORD *)v14;
			v14 += 2;
		} while (v15);


		for (int i = 0; i < 32; i++) ss[i] = v4[i];
		return true;
	}
	__except (1) {
		return false;
	}
}
