#pragma once
#include "stdafx.h"
#include <string>
#include <list>
#include "SysHlp.h"

enum TrainerWorkerOp {
	TrainerWorkerOpVirusBoom,
	TrainerWorkerOpVirusQuit,
	TrainerWorkerOp1,
	TrainerWorkerOpForceUnLoadVirus,
	TrainerWorkerOp2,
	TrainerWorkerOp3,
	TrainerWorkerOp4,
	TrainerWorkerOp5,
};

class TrainerWorkerCallback
{
public:
	
	enum TrainerStatus {
		TrainerStatusNotFound,
		TrainerStatusNotRunning,
		TrainerStatusUnknowProblem,
		TrainerStatusControllFailed,
		TrainerStatusControlled,
		TrainerStatusControlledAndUnLocked,
		TrainerStatusStopped
	};

	virtual void OnUpdateStudentMainInfo(bool running, LPCWSTR fullPath, DWORD pid, bool byuser) {}
	virtual void OnUpdateState(TrainerStatus status, LPCWSTR textMain, LPCWSTR textMore) {}
	virtual void OnResolveBlackScreenWindow() {}
	virtual void OnSimpleMessageCallback(LPCWSTR text) {}
	virtual void OnBeforeSendStartConf() {}
	virtual void OnAllowGbTop() {}
	virtual void OnShowHelp() {}
	virtual void OnSwitchLockStatus(bool locked) {}
	virtual HWND GetMainHWND() { return NULL; }
};
class TrainerWorker
{
public:
	TrainerWorker() {}
	virtual ~TrainerWorker() {}

	virtual void Init() {}
	virtual void UpdateScreenSize() {}
	virtual void InitSettings() {}

	virtual void Start() {}
	virtual void Stop() {}
	virtual bool Running() { return false; }
	virtual DWORD GetStudentMainPid() { return 0; }
	virtual void* RunOperation(TrainerWorkerOp op) { return nullptr; }
	virtual bool SwitchFakeFull() { return false; }
	virtual bool FindProcess(LPCWSTR processName, DWORD*outPid) { return false; }
	virtual bool KillProcess(DWORD pid, bool force){ return false; }

	virtual bool Kill(bool autoWork = false) { return false;  }
	virtual bool Rerun(bool autoWork = false) { return false; }

	virtual void SetUpdateInfoCallback(TrainerWorkerCallback *callback) {}
	virtual void HandleMessageFromVirus(LPCWSTR buf) {}
	virtual void SendMessageToVirus(LPCWSTR buf) {}
	virtual bool AppointStudentMainLocation(LPCWSTR fullPath) { return false; }
};

#ifdef JTEXPORT

struct IncorrectStudentMainFilterData {
	DWORD pid;
	bool checked;
};

class TrainerWorkerInternal : public TrainerWorker
{
public:
	TrainerWorkerInternal();
	~TrainerWorkerInternal();

	void Init();
	void InitSettings();

	void UpdateScreenSize();

	void Start();
	void Stop();
	void StopInternal();
	bool Running() { return _Running; }
	void* RunOperation(TrainerWorkerOp op);

	void SetUpdateInfoCallback(TrainerWorkerCallback *callback);
	void HandleMessageFromVirus(LPCWSTR buf);
	void SendMessageToVirus(LPCWSTR buf);
	bool AppointStudentMainLocation(LPCWSTR fullPath);
	DWORD GetStudentMainPid() { return _StudentMainPid; }

	bool Kill(bool autoWork = false);
	bool KillStAuto();
	bool Rerun(bool autoWork = false);
private:

	TrainerWorkerCallback *_Callback = nullptr;
	bool _Running = false;
	HDESK hDesktop = NULL;

	bool _VirusInstalled = false;
	DWORD _StudentMainPid = 0;
	DWORD _MasterHelperPid = 0;
	std::wstring _StudentMainPath;
	bool _StudentMainFileLocated = false;
	bool _StudentMainFileLocatedByProcess = false;
	bool _StudentMainControlled = false;
	bool _StudentMainRunningLock = false;
	std::wstring _StatusTextMain;
	std::wstring _StatusTextMore;
	std::wstring _TopDomainPassword;

	std::list<IncorrectStudentMainFilterData> incorrectStudentMainPids;

	HHOOK hMsgBoxHook;
	HWND hWndMain;

	bool setAllowGbTop = false;
	bool setAutoIncludeFullWindow = false;
	int setCkInterval = 3100;
	int screenWidth, screenHeight;
	bool gbFullManual = false;

	bool _NextLoopGetCkStat = false;

	bool _FakeFull = false;
	bool _LastResoveBroadcastWindow = false, _LastResoveBlackScreenWindow = false;
	bool _FirstBlackScreenWindow = false;
	int _LastResolveWindowCount = 0;
	bool _FakeBroadcastFull = false, _FakeBlackScreenFull = false;
	HWND _CurrentBroadcastWnd, _CurrentBlackScreenWnd;

	bool RunCk();
	void RunResetPid();

	/* Locate and found*/

	bool FlushProcess();
	void ClearProcess();
	bool FindProcess(LPCWSTR processName, DWORD*outPid);
	bool KillProcess(DWORD pid, bool force);

	bool ReadTopDomanPassword(BOOL forceKnock);
	bool LocateStudentMainLocation();
	bool LocateStudentMain(DWORD *outFirstPid);
	bool LocateMasterHelper(DWORD *outFirstPid);
	bool CheckPidFilterd(DWORD pid);
	void AddFilterdPid(DWORD pid);
	void ClearFilterdPid();

	/* Update ui*/

	void UpdateStudentMainInfo(bool byuser);
	void UpdateState();

	/*Virus*/

	bool InstallVirus();
	bool InstallVirusForMaster();
	bool InjectDll(DWORD pid, LPCWSTR dllPath);
	bool UnInjectDll(DWORD pid, LPCWSTR moduleName);
	bool UnLoadAllVirus();
	bool UnLoadNetFilterDriver();
	bool UnLoadJiYuProtectDriver();

	/*Window resolver*/

	bool SwitchFakeFull();
	void FakeFull(bool fk);
	void ManualFull(bool fk);
	void ManualTop(bool fk);
	bool ChecIsJIYuWindow(HWND hWnd, LPDWORD outPid, LPDWORD outTid);
	bool CheckIsTargetWindow(LPWSTR text, HWND hWnd);
	bool CheckWindowTextIsGb(const wchar_t* text);
	void FixWindow(HWND hWnd, LPWSTR text);

	static TrainerWorkerInternal * currentTrainerWorker;

	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
	static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
	static VOID CALLBACK MsgBoxCallback(LPHELPINFO lpHelpInfo);
	static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
};

#endif

bool UnDecryptJiyuKnock(BYTE * Data, DWORD cbData, WCHAR *ss);
