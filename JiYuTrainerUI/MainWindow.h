#pragma once
#include "stdafx.h"

#define MAIN_WND_CLS_NAME L"JiYuTrainerWindow"
#define MAIN_WND_NAME L"JiYu Trainer Main Window"

#ifdef JIYUTRAINERUI_EXPORTS

#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include <ShellAPI.h>
#include "../JiYuTrainer/TrainerWorker.h"
#include "../JiYuTrainer/Logger.h"
#include "../JiYuTrainer/SysHlp.h"
#include "HelpWindow.h"
#include "AttackWindow.h"
#include <list>
#include "../JiYuTrainer/AppPublic.h"

extern JTApp* currentApp;

class MainWindow : public sciter::host<MainWindow>, public sciter::event_handler, public TrainerWorkerCallback
{
	HWND _hWnd;
	bool _firstShow = true;

	static LRESULT CALLBACK	wndProc(HWND, UINT, WPARAM, LPARAM);
	static MainWindow* ptr(HWND hwnd);

	bool initClass();

public:
	// notification_handler traits:
	HWND get_hwnd() { return _hWnd; }
	HINSTANCE get_resource_instance() { return currentApp->GetInstance(); }

	WCHAR wndClassName[64];

	MainWindow();
	~MainWindow();
	bool init(); // instance
	bool isValid() const { return _hWnd != 0; }

	int RunLoop();
	void Close();

	sciter::value inspectorIsPresent();
	sciter::value docunmentComplete();
	sciter::value exitClick();
	sciter::value toGithub();

	BEGIN_FUNCTION_MAP
		FUNCTION_0("inspectorIsPresent", inspectorIsPresent);
		FUNCTION_0("docunmentComplete", docunmentComplete);
		FUNCTION_0("toGithub", toGithub);
		FUNCTION_0("exitClick", exitClick);
	END_FUNCTION_MAP

	//Tray

	void CreateTrayIcon(HWND hDlg);
	void ShowTrayBaloonTip(const wchar_t* title, const wchar_t* text);

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)  override;

private:

	NOTIFYICONDATA nid;
	int WM_TASKBARCREATED = 0;
	HMENU hMenuTray;

	int hotkeyShowHide = 0;
	int hotkeySwFull = 0;

	bool isUserCancel = false;
	bool hideTipShowed = false;
	bool domComplete = false;

	bool setTopMost = false;
	bool setAutoIncludeFullWindow = false;
	bool setAllowAllRunOp = false;
	bool setAllowGbTop = false;
	bool setAllowMonitor = false;
	bool setAllowControl = false;
	bool setAutoForceKill = false;
	bool setProhibitKillProcess = true;
	bool setProhibitCloseWindow = true;
	bool setBandAllRunOp = false;
	bool setDoNotShowTrayIcon = false;
	bool setAutoUpdate = true;

	int setCkInterval = 3100;

	HelpWindow*currentHelpWindow = nullptr;
	AttackWindow*currentAttackWindow = nullptr;

	Logger* currentLogger = nullptr;
	TrainerWorker * currentWorker = nullptr;
	TrainerStatus currentStatus;

	bool currentControlled = false;

	std::wstring statusBuffer;

	sciter::dom::element status_jiyu_pid;
	sciter::dom::element status_jiyu_path;

	sciter::dom::element status_area;
	sciter::dom::element status_icon;
	sciter::dom::element status_text_main;
	sciter::dom::element status_text_more;
	sciter::dom::element status_protect;
	sciter::dom::element btn_protect_stat;
	sciter::dom::element btn_kill;
	sciter::dom::element btn_top;
	sciter::dom::element btn_restart;
	sciter::dom::element wnd;
	sciter::dom::element input_cmd;
	sciter::dom::element tooltip_top;
	sciter::dom::element tooltip_fast;
	sciter::dom::element body;

	sciter::dom::element extend_area;

	sciter::dom::element link_read_jiyu_password2;
	sciter::dom::element link_unload_netfilter;

	sciter::dom::element check_band_op;
	sciter::dom::element check_probit_close_window;
	sciter::dom::element check_probit_terminate_process;
	sciter::dom::element check_allow_op;
	sciter::dom::element check_allow_control;
	sciter::dom::element check_allow_monitor;
	sciter::dom::element check_auto_update;
	sciter::dom::element check_allow_top;

	sciter::dom::element cmds_message;
	sciter::dom::element common_message;
	sciter::dom::element common_message_title;
	sciter::dom::element common_message_text;
	sciter::dom::element update_message;
	sciter::dom::element update_message_newver;
	sciter::dom::element update_message_text;
	sciter::dom::element isnew_message;
	sciter::dom::element isnew_message_text;
	sciter::dom::element isnew_message_title;

	int autoShutSec = 0;

	void OnWmCommand(WPARAM wParam); 
	BOOL OnWmCreate();
	void OnWmDestroy();
	void OnWmHotKey(WPARAM wParam);
	void OnFirstShow();
	void OnWmTimer(WPARAM wParam);
	void OnWmUser(WPARAM wParam, LPARAM lParam);
	void OnRunCmd(LPCWSTR cmd);

	void OnUpdateStudentMainInfo(bool running, LPCWSTR fullPath, DWORD pid, bool byuser) override;
	void OnUpdateState(TrainerStatus status, LPCWSTR textMain, LPCWSTR textMore) override;
	void OnResolveBlackScreenWindow() override;
	void OnBeforeSendStartConf() override;
	HWND GetMainHWND() override { return _hWnd; }
	void OnSimpleMessageCallback(LPCWSTR text)override;
	void OnAllowGbTop();
	void OnShowHelp();

	void ShowHelp();
	void CloseHelp();

	void ShowFastTip(LPCWSTR text);
	void CloseFastTip();
	void ShowUpdateMessage(LPCWSTR title, LPCWSTR text);
	void ShowFastMessage(LPCWSTR title, LPCWSTR text);
	void CloseCmdsTip();

	void GetUpdateInfo();

	void LoadSettings();
	void LoadSettingsToUi();
	void SaveSettings();
	void SaveSettingsOnQuit();
	void ResetSettings();

	static VOID WINAPI UpdateThread(LPVOID lpFiberParameter);
	static void LogCallBack(const wchar_t*str, LogLevel level, LPARAM lParam);
	void WriteLogItem(const wchar_t * str, LogLevel level);
	void WritePendingLogs();
};

#endif

