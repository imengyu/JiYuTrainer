#pragma once
#include "stdafx.h"
#include <string>

enum TrainerWorkerOp {
	TrainerWorkerOpVirusBoom,
	TrainerWorkerOpVirusQuit,
	TrainerWorkerOp1,
	TrainerWorkerOpForceUnLoadVirus,
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
	virtual bool RunOperation(TrainerWorkerOp op) { return false; }
	virtual void SwitchFakeFull() {}
	virtual bool FindProcess(LPCWSTR processName, DWORD*outPid) { return false; }
	virtual bool KillProcess(DWORD pid, bool force){ return false; }

	virtual bool Kill(bool autoWork = false) { return false;  }
	virtual bool Rerun(bool autoWork = false) { return false; }

	virtual void SetUpdateInfoCallback(TrainerWorkerCallback *callback) {}
	virtual void HandleMessageFromVirus(LPCWSTR buf) {}
	virtual void SendMessageToVirus(LPCWSTR buf) {}
};

#ifdef JTEXPORT

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
	bool RunOperation(TrainerWorkerOp op);

	void SetUpdateInfoCallback(TrainerWorkerCallback *callback);
	void HandleMessageFromVirus(LPCWSTR buf);
	void SendMessageToVirus(LPCWSTR buf);

	bool Kill(bool autoWork = false);
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
	bool _StudentMainControlled = false;
	std::wstring _StatusTextMain;
	std::wstring _StatusTextMore;

	HWND hWndMain;

	bool setAutoIncludeFullWindow = false;
	int setCkInterval = 3100;
	int screenWidth, screenHeight;

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

	bool LocateStudentMainLocation();
	bool LocateStudentMain(DWORD *outFirstPid);
	bool LocateMasterHelper(DWORD *outFirstPid);

	/* Update ui*/

	void UpdateStudentMainInfo(bool byuser);
	void UpdateState();

	/*Virus*/

	bool InstallVirus();
	bool InstallVirusForMaster();
	bool InjectDll(DWORD pid, LPCWSTR dllPath);
	bool UnInjectDll(DWORD pid, LPCWSTR moduleName);
	bool UnLoadAllVirus();

	/*Window resolver*/

	void SwitchFakeFull();
	void FakeFull(bool fk);
	bool ChecIsJIYuWindow(HWND hWnd, LPDWORD outPid, LPDWORD outTid);
	bool CheckIsTargetWindow(LPWSTR text, HWND hWnd);
	void FixWindow(HWND hWnd, LPWSTR text);

	static TrainerWorkerInternal * currentTrainerWorker;

	static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam);
	static VOID CALLBACK TimerProc(HWND, UINT, UINT_PTR, DWORD);
};

#endif

