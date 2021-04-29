#include "stdafx.h"
#include "MainWindow.h"
#include "UpdaterWindow.h"
#include "ConfigWindow.h"
#include "AttackWindow.h"
#include "resource.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/StringHlp.h"
#include "../JiYuTrainer/StringSplit.h"
#include "../JiYuTrainer/KernelUtils.h"
#include "../JiYuTrainer/DriverLoader.h"
#include "../JiYuTrainer/SysHlp.h"
#include "../JiYuTrainer/MD5Utils.h"
#include "../JiYuTrainer/PathHelper.h"
#include "../JiYuTrainerUpdater/JiYuTrainerUpdater.h"

using namespace std;

#define TIMER_AOP 2
#define TIMER_RB_DELAY 3
#define TIMER_HIDE_DELAY 4
#define TIMER_AUTO_SHUT 5

extern JTApp* currentApp;
Logger * currentLogger;

HWND hWndMain = NULL;
int screenWidth, screenHeight;

MainWindow::MainWindow()
{
	currentLogger = currentApp->GetLogger();

	asset_add_ref();
	swprintf_s(wndClassName, MAIN_WND_CLS_NAME);

	if (!initClass()) return;

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	hWndMain = CreateWindow(wndClassName, MAIN_WND_NAME, WS_POPUP, 0, 0, 430, 520, NULL, NULL, currentApp->GetInstance(), this);
	if (!hWndMain)
		return;

	_hWnd = hWndMain;

	init();

	if (currentApp->GetAppIsHiddenMode()) 
		ShowWindow(_hWnd, SW_SHOW);
	else 
		ShowWindow(_hWnd, currentApp->GetAppShowCmd());

	UpdateWindow(_hWnd);
}
MainWindow::~MainWindow()
{
	fuck();
	_hWnd = nullptr;
}

bool MainWindow::initClass()
{
	WNDCLASSEXW wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = currentApp->GetInstance();
	wcex.hIcon = LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_APP));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;//MAKEINTRESOURCE(IDC_PLAINWIN);
	wcex.lpszClassName = wndClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP));

	if (RegisterClassExW(&wcex) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		return TRUE;
	return FALSE;
}
MainWindow* MainWindow::ptr(HWND hwnd)
{
	return reinterpret_cast<MainWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}
bool MainWindow::init()
{
	SciterSetOption(_hWnd, SCITER_ALPHA_WINDOW, TRUE);
	SetWindowLongPtr(_hWnd, GWLP_USERDATA, LONG_PTR(this));
	setup_callback(); // to receive SC_LOAD_DATA, SC_DATA_LOADED, etc. notification
	attach_dom_event_handler(_hWnd, this); // to receive DOM events

	//Settings
	LoadSettings();

	//Init worker
	currentLogger = currentApp->GetLogger();
	currentLogger->SetLogOutPut(LogOutPutCallback);
	currentLogger->SetLogOutPutCallback(LogCallBack, (LPARAM)this);

	currentWorker = currentApp->GetTrainerWorker();
	currentWorker->SetUpdateInfoCallback(this);

	currentApp->LoadDriver();
	
	BOOL result = FALSE;
	HRSRC hResource = FindResource(currentApp->GetInstance(), MAKEINTRESOURCE(IDR_HTML_MAIN), RT_HTML);
	if (hResource) {
		HGLOBAL hg = LoadResource(currentApp->GetInstance(), hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
				result = load_html((LPCBYTE)pData, SizeofResource(currentApp->GetInstance(), hResource));
		}
	}

	return result;
}
sciter::value MainWindow::inspectorIsPresent()
{
	HWND hwnd = FindWindow(WSTR("H-SMILE-FRAME"), WSTR("Sciter's Inspector"));
	return sciter::value(hwnd != NULL);
}
sciter::value MainWindow::docunmentComplete()
{
	sciter::dom::element root = get_root();

	body = root.find_first("body");

	status_jiyu_pid = root.get_element_by_id(L"status_jiyu_pid");
	status_jiyu_path = root.get_element_by_id(L"status_jiyu_path");

	extend_area = root.get_element_by_id(L"extend_area");

	status_icon = root.get_element_by_id(L"status_icon");
	status_text_main = root.get_element_by_id(L"status_text_main");
	status_text_more = root.get_element_by_id(L"status_text_more");
	btn_kill = root.get_element_by_id(L"btn_kill");
	btn_top = root.get_element_by_id(L"btn_top");
	btn_restart = root.get_element_by_id(L"btn_restart");
	wnd = root.get_element_by_id(L"wnd");
	status_area = root.get_element_by_id(L"status_area");
	input_cmd = root.get_element_by_id(L"input_cmd");
	tooltip_top = root.get_element_by_id(L"tooltip_top");
	tooltip_fast = root.get_element_by_id(L"tooltip_fast"); 
	btn_protect_stat = root.get_element_by_id(L"btn_protect_stat");
	status_protect = root.get_element_by_id(L"status_protect");
	
	check_band_op = root.get_element_by_id(L"check_band_op");
	check_probit_close_window = root.get_element_by_id(L"check_probit_close_window");
	check_probit_terminate_process = root.get_element_by_id(L"check_probit_terminate_process");
	check_allow_op = root.get_element_by_id(L"check_allow_op");
	check_allow_top = root.get_element_by_id(L"check_allow_top");
	check_auto_update = root.get_element_by_id(L"check_auto_update");
	check_allow_control = root.get_element_by_id(L"check_allow_control");
	check_allow_monitor = root.get_element_by_id(L"check_allow_monitor");
	link_read_jiyu_password2 = root.get_element_by_id(L"link_read_jiyu_password2");
	link_unload_netfilter = root.get_element_by_id(L"link_unload_netfilter");

	cmds_message = root.get_element_by_id(L"cmds_message");
	common_message = root.get_element_by_id(L"common_message");
	common_message_title = root.get_element_by_id(L"common_message_title");
	common_message_text = root.get_element_by_id(L"common_message_text");
	update_message_newver = root.get_element_by_id(L"update_message_newver");
	update_message_text = root.get_element_by_id(L"update_message_text");
	update_message = root.get_element_by_id(L"update_message");
	isnew_message = root.get_element_by_id(L"isnew_message");
	isnew_message_text = root.get_element_by_id(L"isnew_message_text");
	isnew_message_title = root.get_element_by_id(L"isnew_message_title");

	domComplete = true;

	//Appily settings to ui
	LoadSettingsToUi();
	WritePendingLogs();

	return sciter::value(domComplete);
}
sciter::value MainWindow::exitClick()
{
	SendMessage(_hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
	return sciter::value::null();
}
sciter::value MainWindow::toGithub() {
	SysHlp::OpenUrl(L"https://github.com/imengyu/JiYuTrainer");
	return sciter::value::null();
}

void MainWindow::OnWmCommand(WPARAM wParam)
{
	switch (wParam)
	{
	case IDM_SHOWMAIN: {
		if (IsWindowVisible(_hWnd)) {
			//sciter::dom::element root(get_root());
			//root.call_function("closeWindow");
			ShowWindow(_hWnd, SW_HIDE);
		}
		else
		{
			ShowWindow(_hWnd, SW_SHOW);
			SetWindowPos(_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		}
		break;
	}
	case IDM_EXIT: {
		if (currentControlled) {
			sciter::dom::element(get_root()).call_function("showExitMessage");
			if (!IsWindowVisible(_hWnd)) SendMessage(_hWnd, WM_COMMAND, IDM_SHOWMAIN, NULL);
		}
		else Close();
		break;
	}
	case IDM_HELP: ShowHelp(); break;
	case IDC_UPDATE_CLOSE: {
		Close();
		break;
	}
	default: break;
	}
}
BOOL MainWindow::OnWmCreate()
{
	return TRUE;
}
void MainWindow::OnWmDestroy()
{
	if (!isUserCancel && currentControlled)
		SysHlp::RunApplicationPriviledge(currentApp->GetFullPath(), L"-r1");

	//Save some settings
	SaveSettingsOnQuit();

	CloseHelp();

	UnregisterHotKey(_hWnd, hotkeyShowHide);
	UnregisterHotKey(_hWnd, hotkeySwFull);

	SetWindowLong(_hWnd, GWL_USERDATA, 0);

	PostQuitMessage(0);

	_hWnd = NULL;
}
void MainWindow::OnWmHotKey(WPARAM wParam)
{
	if (wParam == hotkeyShowHide) {

		if (IsWindowVisible(_hWnd)) {
			if (currentAttackWindow) 
				SendMessage(currentAttackWindow->get_hwnd(), WM_MY_FORCE_HIDE, 0, 0);
			if (currentHelpWindow)
				SendMessage(currentHelpWindow->get_hwnd(), WM_MY_FORCE_HIDE, 0, 0);
		}

		SendMessage(_hWnd, WM_COMMAND, IDM_SHOWMAIN, NULL);
	}
	if (wParam == hotkeySwFull) {
		if(!currentWorker->SwitchFakeFull())
			ShowTrayBaloonTip(L"JiYu Trainer 提示", L"您已退出假装全屏模式");
	}
}
void MainWindow::OnWmTimer(WPARAM wParam)
{
	if (wParam == TIMER_AOP) {
		SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	if (wParam == TIMER_RB_DELAY) {
		KillTimer(_hWnd, TIMER_RB_DELAY);
		currentLogger->Log(L"Send main path message ");
		currentWorker->RunOperation(TrainerWorkerOp1);
	}
	if (wParam == TIMER_HIDE_DELAY) {
		KillTimer(_hWnd, TIMER_HIDE_DELAY);
		SendMessage(_hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
	}
	if (wParam == TIMER_AUTO_SHUT) {
		autoShutSec--;
		if (autoShutSec < 0) {
			currentLogger->LogInfo(L"Achieve shutdown time");
			KillTimer(_hWnd, TIMER_AUTO_SHUT);
			OnRunCmd(L"sss");
		}
	}
}
void MainWindow::OnWmUser(WPARAM wParam, LPARAM lParam)
{
	if (lParam == WM_LBUTTONDBLCLK)
		SendMessage(_hWnd, WM_COMMAND, IDM_SHOWMAIN, lParam);
	if (lParam == WM_RBUTTONDOWN)
	{
		POINT pt;
		GetCursorPos(&pt);//取鼠标坐标  
		SetForegroundWindow(_hWnd);//解决在菜单外单击左键菜单不消失的问题  
		TrackPopupMenu(hMenuTray, TPM_RIGHTBUTTON, pt.x - 177, pt.y, NULL, _hWnd, NULL);//显示菜单并获取选项ID  
	}
}
void MainWindow::OnRunCmd(LPCWSTR cmd)
{
	wstring cmdx(cmd);
	if (cmdx == L"") ShowFastTip(L"<h4>请输入命令！</h4>");
	else {
		bool succ = true;
		vector<wstring> cmds;
		SplitString(cmdx, cmds, L" ");
		int len = cmds.size();
		wstring cmd = (cmds)[0];
		if (cmd == L"killst") {
			if (currentWorker->Kill())
				currentLogger->Log(L"已成功结束极域进程");
		}
		else if (cmd == L"rerunst") {
			if (currentWorker->Rerun())
				currentLogger->Log(L"已成功运行极域进程");
		}
		else if (cmd == L"kill") {
			if (len >= 2) {
				bool force = false;
				if (len >= 3)  force = ((cmds)[2] == L"true");
				currentWorker->KillProcess(_wtoi((cmds)[1].c_str()), force);
			}
			else currentLogger->LogError(L"缺少参数 (pid)");
		}
		else if (cmd == L"findps") {
			if (len >= 2) {
				DWORD pid = 0;
				LPCWSTR procName = (cmds)[1].c_str();
				if (currentWorker->FindProcess(procName, &pid)) currentLogger->LogError(L"进程名为：%s 的第一个进程PID 为：%d", procName, pid);
				else currentLogger->LogError(L"未找到进程：%s", procName);
			}
			else currentLogger->LogError(L"缺少参数 (pid)");
		}
		else if (cmd == L"ss") { 
			currentWorker->RunOperation(TrainerWorkerOpVirusBoom);
			currentLogger->Log(L"已发送 ss 命令"); 
		}
		else if (cmd == L"sss") {
			currentWorker->RunOperation(TrainerWorkerOpVirusQuit);
			ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
			SendMessage(_hWnd, WM_COMMAND, IDM_EXIT, NULL);
		}
		else if (cmd == L"ssr") {
			currentWorker->RunOperation(TrainerWorkerOpVirusQuit);
			ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0); 
			SendMessage(_hWnd, WM_COMMAND, IDM_EXIT, NULL);
		}
		else if (cmd == L"sst") {
			if (len >= 2) {
				LPCWSTR str = (cmds)[1].c_str();
				if (StrEqual(str, L"-c")) {
					KillTimer(_hWnd, TIMER_AUTO_SHUT);
					autoShutSec = 0;
					currentLogger->Log(L"关机已取消");
				}
				else {
					autoShutSec = _wtoi(str);
					SetTimer(_hWnd, TIMER_AUTO_SHUT, 1000, NULL);
					currentLogger->Log(L"预定将在 %d 秒后关机，输入 sst -c 取消关机", autoShutSec);
				}
			}
			else {
				currentLogger->Log(L"预定将在 %d 秒后关机，输入 sst -c 取消关机", autoShutSec);
			}
		}
		else if (cmd == L"ssss") currentApp->RunOperation(AppOperationKShutdown);
		else if (cmd == L"sssr") currentApp->RunOperation(AppOperationKReboot);
		else if (cmd == L"ckend") { 
			currentWorker->RunOperation(TrainerWorkerOpVirusQuit); 
			currentLogger->Log(L"已与极域分离");
		}
		else if (cmd == L"unloaddrv") {
			currentApp->RunOperation(AppOperationUnLoadDriver);
		}
		else if (cmd == L"floaddrv") {
			currentApp->RunOperation(AppOperationForceLoadDriver);
		}
		else if (cmd == L"fuljydrv") {
			currentWorker->RunOperation(TrainerWorkerOp4);
			currentLogger->LogWarn2(L"此操作过于危险，已经弃用");
		}
		else if (cmd == L"inspector") sciter::dom::element(get_root()).call_function("runInspector");
		else if (cmd == L"whereisi") {
			currentLogger->Log(L"本程序路径是：%s", currentApp->GetFullPath());
		}
		else if (cmd == L"testupdate") {
			UpdaterWindow u(_hWnd);
			u.RunLoop();
		}	
		else if (cmd == L"jypasswd") { 
			LPCWSTR passwd;
			int res = MessageBox(_hWnd, L"您是否希望使用解密模式读取极域密码？\n选择 [是]  使用解密模式读取极域密码，适用于极域6.0版本\n选择 [否]  则直接读取极域注册表密码，适用于极域老版本", L"JiYuTrainer - 提示", MB_ICONASTERISK | MB_YESNOCANCEL);
			if (res == IDYES) passwd = (LPCWSTR)currentWorker->RunOperation(TrainerWorkerOp3);
			else if (res == IDNO) passwd = (LPCWSTR)currentWorker->RunOperation(TrainerWorkerOp2);
			else return;
			if (passwd) {
				if (StrEmepty(passwd)) {
					MessageBox(_hWnd, L"已成功读取极域密码，密码为空。", L"JiYuTrainer - 提示", MB_ICONINFORMATION);
				}
				else {
					FAST_STR_BINDER(str, L"已成功读取极域密码，\n密码是：%s", 128, passwd);
					MessageBox(_hWnd, str, L"JiYuTrainer - 提示", MB_ICONINFORMATION);
				}
			}
			else MessageBox(_hWnd, L"极域电子教室密码读取失败！或许你可以用 mythware_super_password 试试", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION);
		}
		else if (cmd == L"attack") {
			if (currentAttackWindow == nullptr)
				currentAttackWindow = new AttackWindow(_hWnd);
			else
				currentAttackWindow->Show();
		}
		else if (cmd == L"unload_netfilter") {
			if (MessageBox(_hWnd, L"您是否希望解除极域的网络控制？此操作会卸载极域的网络过滤驱动，卸载以后网络将不受其控制。\n" 
				"卸载过程中可能卡顿，请等待程序执行完成。\n此操作只需执行一次即可。\n提示：在卸载完成以后最好在“控制面板”>"
				"“网络和共享中心”>“更改适配器选项”，选本地连接，右键禁用再启用，这样可以重启网络使设置生效。", L"JiYuTrainer - 提示", MB_ICONWARNING | MB_YESNO) == IDYES)
				if (currentWorker->RunOperation(TrainerWorkerOp5))
					MessageBox(_hWnd, L"卸载极域的网络过滤驱动成功", L"JiYuTrainer - 提示", MB_ICONINFORMATION);
		}
		else if (cmd == L"uj") {
			if (currentWorker) {
				//卸载病毒
				currentWorker->RunOperation(TrainerWorkerOpVirusBoom);
				currentWorker->RunOperation(TrainerWorkerOpForceUnLoadVirus);
			}
		}
#if _DEBUG
		else if (cmd == L"test") currentLogger->Log(L"测试命令，无功能");
		else if (cmd == L"test2") currentWorker->SendMessageToVirus(L"test2:f");
		else if (cmd == L"test3") MessageBox(hWndMain, L"MessageBox", L"test3", 0);
		else if (cmd == L"test5") {
			ShowUpdateMessage(L"您的 JiYu Trainer 是最新版本", L"您的 JiYu Trainer 是最新的版本! 时常更新是个好习惯，可以给你带来更好的软件使用体验");
		}
		else if (cmd == L"test6") {
			ShowUpdateMessage(L"更新失败", L"检查更新失败，请检查您的网络连接？");
		}
		else if (cmd == L"test7") {
			ShowUpdateMessage(L"更新服务器返回了错误的结果", L"(⊙o⊙)？糟糕，更新服务器出了一点故障，请你稍后再试");
		}
#endif
		else if (cmd == L"version") {
			currentLogger->Log(L"当前版本是：%hs", CURRENT_VERSION);
		}
		else if (cmd == L"config") {
			ShowMoreSettings(_hWnd);
		}
		else if (cmd == L"crash") {
			currentLogger->Log(L"测试崩溃功能");
			currentApp->RunOperation(AppOperation3);
		}
		else if (cmd == L"exit" || cmd == L"quit") {
			currentWorker->RunOperation(TrainerWorkerOpVirusQuit);
			SendMessage(hWndMain, WM_COMMAND, IDM_EXIT, NULL);
		}
		else if (cmd == L"hide") { ShowWindow(hWndMain, SW_HIDE); }
		else {
			succ = false;
			ShowFastMessage(L"未知命令", L"要查看所有命令及使用方法，请在源代码中查看。");
		}
		if (succ) input_cmd.set_value(sciter::value(L""));
	}
}
void MainWindow::OnFirstShow()
{
	//热键
	hotkeyShowHide = GlobalAddAtom(L"HotKeyShowHide");
	hotkeySwFull = GlobalAddAtom(L"HotKeySwFull");

	int setHotKeyFakeFull = currentApp->GetSettings()->GetSettingInt(L"HotKeyFakeFull", 1606);
	int setHotKeyShowHide = currentApp->GetSettings()->GetSettingInt(L"HotKeyShowHide", 1604);

	UINT mod = 0, vk = 0;
	SysHlp::HotKeyCtlToKeyCode(setHotKeyShowHide, &mod, &vk);
	if (!RegisterHotKey(_hWnd, hotkeyShowHide, mod, vk))
		currentLogger->LogWarn(L"热键 快速显示/隐藏窗口 注册失败，请检查是否有程序占用，错误：%d", GetLastError());

	SysHlp::HotKeyCtlToKeyCode(setHotKeyFakeFull, &mod, &vk);
	if (!RegisterHotKey(_hWnd, hotkeySwFull, mod, vk))
		currentLogger->LogWarn(L"热键 紧急全屏 注册失败，请检查是否有程序占用，错误：%d", GetLastError());

	//托盘图标
	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	CreateTrayIcon(_hWnd);
	hMenuTray = LoadMenu(currentApp->GetInstance(), MAKEINTRESOURCE(IDR_MAINMENU));
	hMenuTray = GetSubMenu(hMenuTray, 0);

	HBITMAP hIconExit = LoadBitmap(currentApp->GetInstance(), MAKEINTRESOURCE(IDB_CLOSE));
	HBITMAP hIconHelp = LoadBitmap(currentApp->GetInstance(), MAKEINTRESOURCE(IDB_HELP));

	SetMenuItemBitmaps(hMenuTray, IDM_EXIT, MF_BITMAP, hIconExit, hIconExit);
	SetMenuItemBitmaps(hMenuTray, IDM_HELP, MF_BITMAP, hIconHelp, hIconHelp);

	//初始化控制器
	currentWorker->Init();
	currentWorker->Start();

	//显示重启提示
	if (currentApp->IsCommandExists(L"r1")) {
		currentLogger->LogInfo(L"Reboot mode 1");
		ShowFastMessage(L"刚才进程意外退出", L"极域可能试图结束本进程，或是其他软件（比如任务管理器）结束了本进程，为了安全，我们已经重启了软件进程，您如果要退出本软件的话，请手动点击托盘图标>退出软件。");
	}
	else if (currentApp->IsCommandExists(L"r2")) {
		currentLogger->LogInfo(L"Reboot mode 2");
		ShowFastTip(L"刚才意外与病毒失去联系，现已杀死极域并重启软件主进程");
	}
	else if (currentApp->IsCommandExists(L"r3")) {
		currentLogger->LogInfo(L"Reboot mode 3");
		ShowFastTip(L"软件已重启");
	}

	if (currentApp->IsCommandExists(L"ia")) {
		ShowFastMessage(L"更新完成！", L"您已经更新到软件最新版本，我们努力保证您的最佳使用体验，时常更新是非常好的做法。");
	}

	//运行更新
	if (setAutoUpdate)
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)UpdateThread, this, 0, NULL);

	currentLogger->LogInfo(L"控制器已启动");

	if (currentApp->GetAppIsHiddenMode()) {
		hideTipShowed = true;
		SetTimer(_hWnd, TIMER_HIDE_DELAY, 400, NULL);
	}
}

bool MainWindow::on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)
{
	sciter::dom::element ele(he);
	if (type == HYPERLINK_CLICK)
	{
		if (ele.get_attribute("id") == L"btn_about" || ele.get_attribute("id") == L"link_help") ShowHelp();
		else if (ele.get_attribute("id") == L"btn_top") {
			if (setTopMost) {
				setTopMost = false;
				btn_top.set_attribute("class", L"btn-footers btn-top ml-0");
				tooltip_top.set_text(L"本窗口置顶");
				KillTimer(_hWnd, TIMER_AOP);
				SetWindowPos(hWndMain, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			}
			else {
				setTopMost = true;
				btn_top.set_attribute("class", L"btn-footers btn-top ml-0 topmost");
				tooltip_top.set_text(L"取消置顶");
				SetTimer(_hWnd, TIMER_AOP, 400, NULL);
				SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			}
		}
		else if (ele.get_attribute("id") == L"btn_kill") {
			if (currentWorker->Kill())
				ShowFastTip(L"<h4>已成功结束极域电子教室</h4>");
		}
		else if (ele.get_attribute("id") == L"btn_restart") {
			if (currentWorker->Rerun())
				ShowFastTip(L"<h4>已启动极域电子教室</h4>");
		}
		else if (ele.get_attribute("id") == L"link_save_setting") {
			SaveSettings();
			ShowFastTip(L"<h4>设置保存成功！</h4>");
		}
		else if (ele.get_attribute("id") == L"link_setto_default") {
			ResetSettings();
			ShowFastTip(L"<h4>已恢复默认设置</h4>");
		}
		else if (ele.get_attribute("id") == L"link_checkupdate") {
			ShowFastTip(L"正在检查更新... ");
			if (JUpdater_CheckInternet()) {
				int updateStatus = JUpdater_CheckUpdate(true);
				CloseFastTip();
				if (updateStatus == UPDATE_STATUS_LATEST)  ShowUpdateMessage(L"您的 JiYu Trainer 是最新版本", L"您的 JiYu Trainer 是最新的版本！时常更新是个好习惯，可以给你带来更好的软件使用体验");
				else if (updateStatus == UPDATE_STATUS_HAS_UPDATE) GetUpdateInfo();
				else if (updateStatus == UPDATE_STATUS_COULD_NOT_CONNECT) ShowUpdateMessage(L"更新失败",  L"检查更新失败，请检查您的网络连接？");
				else if (updateStatus == UPDATE_STATUS_NOT_SUPPORT) ShowUpdateMessage(L"更新服务器返回了错误的结果", L"(⊙o⊙)？糟糕，更新服务器出了一点故障，请你稍后再试");
			}
			else ShowFastTip(L"检查更新失败，请检查您的网络连接？");
		}
		else if (ele.get_attribute("id") == L"link_runcmd") {
			sciter::value cmdsx(input_cmd.get_value());
			OnRunCmd(cmdsx.to_string().c_str());
		}
		else if (ele.get_attribute("id") == L"link_exit") SendMessage(_hWnd, WM_COMMAND, IDM_EXIT, NULL);
		else if (ele.get_attribute("id") == L"update_message_update") {
			UpdaterWindow updateWindow(_hWnd);
			updateWindow.RunLoop();
		}
		else if (ele.get_attribute("id") == L"exit_message_kill_and_exit") {
			isUserCancel = true;
			currentWorker->Kill(true);
			Close();
		}
		else if (ele.get_attribute("id") == L"exit_message_end_ctl_and_exit") {
			isUserCancel = true;
			currentWorker->RunOperation(TrainerWorkerOpVirusQuit);
			Close();
		}
		else if (ele.get_attribute("id") == L"link_uninstall") {
			if (MessageBox(_hWnd, L"你是否真的要卸载本软件？\n卸载会删除本软件相关安装文件，但不会删除源安装包；并且卸载过程中会暂时结束极域主进程，稍后您需要手动启动极域。", L"JiYuTrainer - 警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				currentApp->UnInstall();
		}
		else if (ele.get_attribute("id") == L"link_read_jiyu_password" || ele.get_attribute("id") == L"link_read_jiyu_password2") { OnRunCmd(L"jypasswd"); CloseCmdsTip(); }
		else if (ele.get_attribute("id") == L"link_unload_netfilter") { OnRunCmd(L"unload_netfilter"); CloseCmdsTip(); }
		else if (ele.get_attribute("id") == L"link_hide") { OnRunCmd(L"hide"); }
		else if (ele.get_attribute("id") == L"link_shutdown") {
			if (MessageBox(_hWnd, L"你是否真的要关闭电脑？", L"JiYuTrainer - 警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) 
				OnRunCmd(L"sss");
		}
		else if (ele.get_attribute("id") == L"link_reboot") {
			if (MessageBox(_hWnd, L"你是否真的要重启电脑？", L"JiYuTrainer - 警告", MB_YESNO | MB_ICONEXCLAMATION) == IDYES) 
				OnRunCmd(L"ssr");
		}
		else if (ele.get_attribute("id") == L"link_more_settings") ShowMoreSettings(_hWnd);
		else if (ele.get_attribute("id") == L"link_locate_jiyu_position") {
			TCHAR strFilename[MAX_PATH] = { 0 };
			if (SysHlp::ChooseFileSingal(_hWnd, NULL, L"请选择极域主进程 StudentMain.exe 的位置", L"StudentMain.exe\0*.exe\0所有文件(*.*)\0*.*\0\0\0",
				strFilename, NULL, strFilename, MAX_PATH)) {		
				if (currentWorker->AppointStudentMainLocation(strFilename)) ShowFastTip(L"已更改极域主进程位置");
				else MessageBox(hWndMain, L"您选择的主进程位置无效。", L"JiYuTrainer - 提示", MB_ICONEXCLAMATION);
			}
		}
	}
	else if (type == BUTTON_CLICK)
	{
		if (ele.get_attribute("id") == L"check_ck") {
			if (currentWorker->Running()) {
				ele.set_value(sciter::value(false));
				currentWorker->Stop();
				currentLogger->LogInfo(L"控制器已停止");
			}
			else {
				currentWorker->Start();
				ele.set_value(sciter::value(true));
				currentLogger->LogInfo(L"控制器正在运行");
			}
		}
	}
	return false;
}

void MainWindow::OnUpdateStudentMainInfo(bool running, LPCWSTR fullPath, DWORD pid, bool byuser)
{
	if (!domComplete) 
		return;

	if (running) {
		btn_kill.set_attribute("style", L"");
		btn_restart.set_attribute("style", L"display: none;");

		WCHAR w[16]; swprintf_s(w, L"%d", pid);
		status_jiyu_pid.set_text(w);
	}
	else {
		btn_restart.set_attribute("style", L"");
		btn_kill.set_attribute("style", L"display: none;");
	}

	if (StringHlp::StrEmeptyW(fullPath)) {
		link_read_jiyu_password2.set_attribute("style", L"display: none;");
		status_jiyu_path.set_text(L"未找到极域电子教室");
	}
	else {
		link_read_jiyu_password2.set_attribute("style", L"");
		std::wstring s1(fullPath);
		s1 += L"<br/><small>点击运行极域电子教室</small>";
		LPCSTR textMore2 = StringHlp::UnicodeToUtf8(s1.c_str());
		status_jiyu_path.set_html((UCHAR*)textMore2, strlen(textMore2));
		FreeStringPtr(textMore2);
	}
}
void MainWindow::OnUpdateState(TrainerStatus status, LPCWSTR textMain, LPCWSTR textMore)
{
	if (!domComplete) return;

	currentStatus = status;
	currentControlled = (currentStatus == TrainerStatus::TrainerStatusControlled || currentStatus == TrainerStatus::TrainerStatusControlledAndUnLocked);

	status_text_main.set_text(textMain);
	LPCSTR textMore2 = StringHlp::UnicodeToUtf8(textMore);
	status_text_more.set_html((UCHAR*)textMore2, strlen(textMore2));
	FreeStringPtr(textMore2);

	int protectStat = 0;

	switch (status)
	{
	case TrainerWorkerCallback::TrainerStatusNotFound: status_icon.set_attribute("class", L"state-not-found"); wnd.set_attribute("class", L"window-box state-notwork"); protectStat = 0;  break;
	case TrainerWorkerCallback::TrainerStatusNotRunning: status_icon.set_attribute("class", L"state-not-run"); wnd.set_attribute("class", L"window-box state-notwork");  protectStat = 0; break;
	case TrainerWorkerCallback::TrainerStatusUnknowProblem: status_icon.set_attribute("class", L"state-unknow-problem"); wnd.set_attribute("class", L"window-box state-warn");  protectStat = 1;  break;
	case TrainerWorkerCallback::TrainerStatusControllFailed: status_icon.set_attribute("class", L"state-failed"); wnd.set_attribute("class", L"window-box state-warn");  protectStat = 1; break;
	case TrainerWorkerCallback::TrainerStatusControlled: status_icon.set_attribute("class", L"state-ctl-no-lock"); wnd.set_attribute("class", L"window-box state-work");  protectStat = 2;  break;
	case TrainerWorkerCallback::TrainerStatusControlledAndUnLocked: status_icon.set_attribute("class", L"state-ctl-unlock"); wnd.set_attribute("class", L"window-box state-work"); protectStat = 2;  break;
	case TrainerWorkerCallback::TrainerStatusStopped: status_icon.set_attribute("class", L"state-manual-stop"); wnd.set_attribute("class", L"window-box state-warn");  protectStat = 1; break;
	default:
		break;
	}
	
	if (protectStat == 0) {
		btn_protect_stat.set_attribute("class", L"btn-footers protect-stat no-danger");
		status_protect.set_text(L"您未受到极域电子教室的控制");
	}
	else if (protectStat == 1) {
		btn_protect_stat.set_attribute("class", L"btn-footers protect-stat not-protected");
		status_protect.set_text(L"出现错误，无法保护您免受极域电子教室的控制");
	}
	else if (protectStat == 2) {
		btn_protect_stat.set_attribute("class", L"btn-footers protect-stat protected");
		status_protect.set_text(L"已保护您免受极域电子教室的控制");
	}
}
void MainWindow::OnResolveBlackScreenWindow()
{
	if (!domComplete) return;
	ShowFastTip(L"<h5>已关闭极域的黑屏窗口，您可以继续您的工作了！</h5>");
}
void MainWindow::OnBeforeSendStartConf()
{
	SetTimer(_hWnd, TIMER_RB_DELAY, 1500, NULL);
}
void MainWindow::OnSimpleMessageCallback(LPCWSTR text)
{
	if (!domComplete) return;
	ShowFastTip(text);
}
void MainWindow::OnAllowGbTop() {
	setAllowGbTop = true;
	LoadSettingsToUi();
	SaveSettings();
}
void MainWindow::OnShowHelp()
{
	ShowHelp();
}

void MainWindow::ShowHelp()
{
	if (currentHelpWindow == nullptr)
		currentHelpWindow = new	HelpWindow(_hWnd);
	else
		currentHelpWindow->Show();
}
void MainWindow::CloseHelp()
{
	if (currentHelpWindow != nullptr) {
		currentHelpWindow->Close();
		delete currentHelpWindow;
		currentHelpWindow = nullptr;
	}
}

void MainWindow::ShowFastTip(LPCWSTR text) 
{
	LPCSTR textMore2 = StringHlp::UnicodeToUtf8(text);
	tooltip_fast.set_html((UCHAR*)textMore2, strlen(textMore2));
	FreeStringPtr(textMore2);
	sciter::dom::element(get_root()).call_function("showFastTip");
}
void MainWindow::CloseFastTip()
{
	sciter::dom::element(get_root()).call_function("closeFastTip");
}
void MainWindow::ShowUpdateMessage(LPCWSTR title, LPCWSTR text)
{
	isnew_message_title.set_text(title);
	LPCSTR textMore2 = StringHlp::UnicodeToUtf8(text);
	isnew_message_text.set_html((UCHAR*)textMore2, strlen(textMore2));
	FreeStringPtr(textMore2);
	isnew_message.set_attribute("class", L"window-extend-area upper with-mask shown");
}
void MainWindow::ShowFastMessage(LPCWSTR title, LPCWSTR text)
{
	common_message_title.set_text(title);
	common_message_text.set_text(text);
	common_message.set_attribute("class", L"window-extend-area upper with-mask shown");
}
void MainWindow::CloseCmdsTip() {
	sciter::dom::element root(get_root());
	root.call_function("close_cmds_tip");
}
void MainWindow::GetUpdateInfo() {
	CHAR newUpdateMessage[256];
	if (JUpdater_GetUpdateNew(newUpdateMessage, 256))
		update_message_text.set_html((UCHAR*)newUpdateMessage, strlen(newUpdateMessage));
	update_message_newver.set_text(JUpdater_GetUpdateNewVer());
	update_message.set_attribute("class", L"window-extend-area upper with-mask shown");
}

void MainWindow::LoadSettings()
{
	SettingHlp *settings = currentApp->GetSettings();
	setTopMost = settings->GetSettingBool(L"TopMost", false);
	setAutoUpdate = settings->GetSettingBool(L"AutoUpdate ", true);
	setAutoIncludeFullWindow = settings->GetSettingBool(L"AutoIncludeFullWindow", false);
	setAllowAllRunOp = settings->GetSettingBool(L"AllowAllRunOp", true);
	setAutoForceKill = settings->GetSettingBool(L"AutoForceKill", false);
	setAllowMonitor = settings->GetSettingBool(L"AllowMonitor", true);
	setAllowControl = settings->GetSettingBool(L"AllowControl", false);
	setAllowGbTop = settings->GetSettingBool(L"AllowGbTop", false);
	setProhibitKillProcess = settings->GetSettingBool(L"ProhibitKillProcess", true);
	setProhibitCloseWindow = settings->GetSettingBool(L"ProhibitCloseWindow", true);
	setBandAllRunOp = settings->GetSettingBool(L"BandAllRunOp", false);
	setDoNotShowTrayIcon = settings->GetSettingBool(L"DoNotShowTrayIcon", false);
	
	setCkInterval = settings->GetSettingInt(L"CKInterval", 3100);
	if (setCkInterval < 1000 || setCkInterval > 10000) setCkInterval = 3000;
}
void MainWindow::LoadSettingsToUi()
{
	if (setTopMost) { setTopMost = false;  on_event(btn_top, btn_top, HYPERLINK_CLICK, 0); }
	else { setTopMost = true;  on_event(btn_top, btn_top, HYPERLINK_CLICK, 0); }

	check_band_op.set_value(sciter::value(setBandAllRunOp));
	check_probit_close_window.set_value(sciter::value(setProhibitCloseWindow));
	check_probit_terminate_process.set_value(sciter::value(setProhibitKillProcess));

	check_allow_op.set_value(sciter::value(!setAllowAllRunOp));
	check_auto_update.set_value(sciter::value(setAutoUpdate));
	check_allow_control.set_value(sciter::value(setAllowControl));
	check_allow_monitor.set_value(sciter::value(setAllowMonitor));
	check_allow_top.set_value(sciter::value(setAllowGbTop));
}
void MainWindow::SaveSettings()
{
	setBandAllRunOp = check_band_op.get_value().get(false);
	setProhibitCloseWindow = check_probit_close_window.get_value().get(true);
	setProhibitKillProcess = check_probit_terminate_process.get_value().get(true);

	setAllowAllRunOp = !check_allow_op.get_value().get(true);
	
	setAutoUpdate = check_auto_update.get_value().get(true);
	setAllowControl = check_allow_control.get_value().get(false);
	setAllowMonitor = check_allow_monitor.get_value().get(false);
	setAllowGbTop = check_allow_top.get_value().get(false);

	SettingHlp *settings = currentApp->GetSettings();
	settings->SetSettingBool(L"TopMost", setTopMost);
	settings->SetSettingBool(L"AutoIncludeFullWindow", setAutoIncludeFullWindow);
	settings->SetSettingBool(L"AllowAllRunOp", setAllowAllRunOp);
	settings->SetSettingBool(L"AutoForceKill", setAutoForceKill);
	settings->SetSettingBool(L"AutoUpdate", setAutoUpdate);
	settings->SetSettingBool(L"AllowControl", setAllowControl);
	settings->SetSettingBool(L"AllowMonitor", setAllowMonitor);
	settings->SetSettingBool(L"AllowGbTop", setAllowGbTop);

	currentWorker->InitSettings();
}
void MainWindow::SaveSettingsOnQuit() 
{
	SettingHlp *settings = currentApp->GetSettings();
	settings->SetSettingBool(L"TopMost", setTopMost);
	settings->SetSettingBool(L"AllowGbTop", setAllowGbTop);
}
void MainWindow::ResetSettings()
{
	setAutoIncludeFullWindow = false;
	setAllowAllRunOp = false;
	setAutoForceKill = false;
	setAutoUpdate = true;
	setAllowControl = false;
	setAllowMonitor = true;
	setAllowGbTop = false;
	setProhibitKillProcess = true;
	setProhibitCloseWindow = true;
	setBandAllRunOp = false;

	LoadSettingsToUi();
	SaveSettings();
}


VOID WINAPI MainWindow::UpdateThread(LPVOID lpFiberParameter)
{
	MainWindow* self = (MainWindow*)lpFiberParameter;
	if (JUpdater_CheckInternet() && JUpdater_CheckUpdate(false) == UPDATE_STATUS_HAS_UPDATE)
		self->GetUpdateInfo();
}

void MainWindow::LogCallBack(const wchar_t * str, LogLevel level, LPARAM lParam)
{
	MainWindow*self = (MainWindow*)lParam;
	if (self && self->domComplete)  self->WriteLogItem(str, level);
	else self->currentLogger->WritePendingLog(str, level);
}
void MainWindow::WriteLogItem(const wchar_t * str, LogLevel level)
{
	sciter::dom::element newEle = status_area.create("div", str);
	switch (level)
	{
	case LogLevelText:newEle.set_attribute("class", L"text-black"); break;
	case LogLevelInfo:newEle.set_attribute("class", L"text-info");  break;
	case LogLevelWarn:newEle.set_attribute("class", L"text-warning");  break;
	case LogLevelError: newEle.set_attribute("class", L"text-danger");  break;
	}
	status_area.append(newEle);
	newEle.scroll_to_view();
}
void MainWindow::WritePendingLogs() {
	currentLogger->ResentNotCaputureLog();
}

int MainWindow::RunLoop()
{
	if (!isValid())
		return -1;

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0) && isValid())
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.lParam;
}
void MainWindow::Close()
{
	DestroyWindow(_hWnd);
}

//Tray

void MainWindow::CreateTrayIcon(HWND hDlg) {
	if (setDoNotShowTrayIcon)
		return;

	nid.cbSize = sizeof(nid);
	nid.hWnd = hDlg;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP;
	nid.uCallbackMessage = WM_USER;
	nid.hIcon = LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_APP));
	lstrcpy(nid.szTip, L"JiYuTrainer");
	Shell_NotifyIcon(NIM_ADD, &nid);
}
void MainWindow::ShowTrayBaloonTip(const wchar_t* title, const wchar_t* text) {
	if (setDoNotShowTrayIcon)
		return;

	lstrcpy(nid.szInfo, text);
	nid.dwInfoFlags = NIIF_NONE;
	lstrcpy(nid.szInfoTitle, title);
	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

LRESULT CALLBACK MainWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

	//SCITER integration starts
	BOOL handled = FALSE;
	LRESULT lr = SciterProcND(hWnd, message, wParam, lParam, &handled);
	if (handled)
		return lr;
	//SCITER integration ends

	MainWindow* self = NULL;

	switch (message)
	{
	case WM_CREATE: {
		hWndMain = hWnd;
		self = (MainWindow*) lParam;

		//窗口居中
		RECT rect; GetWindowRect(hWnd, &rect);
		rect.left = (screenWidth - (rect.right - rect.left)) / 2;
		rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
		SetWindowPos(hWnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
		SetForegroundWindow(hWnd);

		return self->OnWmCreate();
	}
	case WM_COMMAND:  self = ptr(hWnd);  self->OnWmCommand(wParam); break;
	case WM_COPYDATA: {
		self = ptr(hWnd);
		PCOPYDATASTRUCT  pCopyDataStruct = (PCOPYDATASTRUCT)lParam;
		if (pCopyDataStruct->cbData > 0)
		{
			WCHAR recvData[256] = { 0 };
			wcsncpy_s(recvData, (WCHAR *)pCopyDataStruct->lpData, pCopyDataStruct->cbData);
			if (self->currentWorker) self->currentWorker->HandleMessageFromVirus(recvData);
		}
		break;
	}
	case WM_SHOWWINDOW: {
		self = ptr(hWnd);
		if (wParam)
		{
			sciter::dom::element root(self->get_root());
			root.call_function("showWindow");

			if (self->_firstShow)
			{
				self->OnFirstShow();
				self->_firstShow = false;
			}
		}
		break;
	}
	case WM_SYSCOMMAND: {
		self = ptr(hWnd);
		switch (wParam)
		{
		case SC_RESTORE: ShowWindow(hWnd, SW_RESTORE); SetForegroundWindow(hWnd); return TRUE;
		case SC_MINIMIZE:  ShowWindow(hWnd, SW_MINIMIZE);  return TRUE;
		case SC_CLOSE: {
			ShowWindow(hWnd, SW_HIDE);
			if (!self->setTopMost) SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			if (!self->hideTipShowed) {
				self->ShowTrayBaloonTip(L"JiYu Trainer 提示", L"窗口隐藏到此处了，双击这里显示主界面");
				self->hideTipShowed = true;
			}
			return TRUE;
		}
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_HOTKEY: self = ptr(hWnd);  self->OnWmHotKey(wParam); break;
	case WM_DESTROY: self = ptr(hWnd); self->OnWmDestroy(); break;
	case WM_TIMER:  self = ptr(hWnd); self->OnWmTimer(wParam);
	case WM_USER: self = ptr(hWnd); self->OnWmUser(wParam, lParam); break;
	case WM_MY_WND_CLOSE: {
		self = ptr(hWnd);
		void* wnd = (void*)wParam;
		if (wnd == static_cast<CommonWindow*>(self->currentHelpWindow)) {
			self->currentHelpWindow->Release();
			self->currentHelpWindow = nullptr;
		}
		if (wnd == static_cast<CommonWindow*>(self->currentAttackWindow)) {
			self->currentAttackWindow->Release();
			self->currentAttackWindow = nullptr;
		}
		break;
	}
	case WM_QUERYENDSESSION: {
		DestroyWindow(hWnd);
		break;
	}
	case WM_DISPLAYCHANGE: {
		self = ptr(hWnd);
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
		if (self->currentWorker) self->currentWorker->UpdateScreenSize();
		break;
	}
	case WM_CLOSE: return TRUE;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
