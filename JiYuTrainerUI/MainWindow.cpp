#include "stdafx.h"
#include "MainWindow.h"
#include "HelpWindow.h"
#include "resource.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "../JiYuTrainer/App.h"
#include "../JiYuTrainer/StringHlp.h"
#include "../JiYuTrainer/StringSplit.hpp"
#include "../JiYuTrainer/KernelUtils.h"
#include "../JiYuTrainer/DriverLoader.h"
#include "../JiYuTrainer/SysHlp.h"

using namespace std;

#define WINDOW_CLASS_NAME L"sciter-jytrainer-main-window"		

#define TIMER_AOP 2
#define TIMER_RB_DELAY 3

extern JTApp* appCurrent;

HWND hWndMain = NULL;
int screenWidth, screenHeight;

MainWindow::MainWindow()
{
	initClass();

	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	hWndMain = CreateWindowExW(WS_EX_APPWINDOW, WINDOW_CLASS_NAME, L"JiYu Trainer Main Window", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 430, 520, nullptr, nullptr, hInst, this);
	if (!hWndMain)
		return;

	_hWnd = hWndMain;

	init();

	ShowWindow(_hWnd, SW_SHOW);
	UpdateWindow(_hWnd);
}

bool MainWindow::initClass()
{
	static ATOM cls = 0;
	if (cls)
		return true;

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = wndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst;
	wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;//MAKEINTRESOURCE(IDC_PLAINWIN);
	wcex.lpszClassName = WINDOW_CLASS_NAME;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP));

	cls = RegisterClassEx(&wcex);
	return cls != 0;
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

	//Init worker
	currentLogger = appCurrent->GetLogger();
	currentWorker = appCurrent->GetTrainerWorker();
	currentWorker->SetUpdateInfoCallback(this);
	
	BOOL result = FALSE;
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_HTML_MAIN), RT_HTML);
	if (hResource) {
		HGLOBAL hg = LoadResource(hInst, hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
				result = load_html((LPCBYTE)pData, SizeofResource(hInst, hResource));
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
	
	check_auto_fkill = root.get_element_by_id(L"check_auto_fkill");
	check_auto_fck = root.get_element_by_id(L"check_auto_fck");
	check_allow_op = root.get_element_by_id(L"check_allow_op");
	input_ckinterval = root.get_element_by_id(L"input_ckinterval");

	domComplete = true;

	//Appily settings to ui
	LoadSettingsToUi();

	return sciter::value(domComplete);
}
sciter::value MainWindow::test1()
{
	TrainerStatus st = currentStatus;
	int st1 = (int)st;
	if(st1<6) st1++;
	else st1 = 0;
	OnUpdateState((TrainerStatus)st1, L"DEBUG", L"尝试 DEBUG");
	return sciter::value::null();
}
sciter::value MainWindow::exitClick()
{
	SendMessage(_hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
	return sciter::value::null();
}

void MainWindow::OnWmCommand(WPARAM wParam)
{
	switch (wParam)
	{
	case IDM_SHOWMAIN: {
		if (IsWindowVisible(_hWnd)) {
			sciter::dom::element root(get_root());
			root.call_function("closeWindow");
		}
		else
		{
			ShowWindow(_hWnd, SW_SHOW);
			SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			sciter::dom::element root(get_root());
			root.call_function("showWindow"); 
		}
		break;
	}
	case IDM_EXIT: {
		if (currentControlled) {
			int id = MessageBox(_hWnd, L"极域正在运行，您是否还要退出？\n点击“是”将会强制分离极，导致极域脱离控制；点击“否”不退出；点击“取消”将会退出本软件，并杀死极域。", L"注意", MB_YESNOCANCEL | MB_ICONEXCLAMATION);
			if (id == IDYES) {
				isUserCancel = true;
				DestroyWindow(_hWnd);
			}
			else if (id == IDCANCEL) {
				isUserCancel = true;
				DestroyWindow(_hWnd);
			}
		}
		else DestroyWindow(_hWnd);
		break;
	}
	case IDM_HELP:ShowHelp(); break;
	default: break;
	}
}
BOOL MainWindow::OnWmCreate()
{
	//Settings
	LoadSettings();

	return TRUE;
}
void MainWindow::OnWmDestroy()
{
	if (!isUserCancel && currentControlled)
	{
		SysHlp::RunApplicationPriviledge(appCurrent->GetFullPath(), L"-r1");
		ExitProcess(0);
	}

	UnregisterHotKey(_hWnd, hotkeyShowHide);
	UnregisterHotKey(_hWnd, hotkeySwFull);
	PostQuitMessage(0);
}
void MainWindow::OnWmHotKey(WPARAM wParam)
{
	if (wParam == hotkeyShowHide) SendMessage(_hWnd, WM_COMMAND, IDM_SHOWMAIN, NULL);
	if (wParam == hotkeySwFull) currentWorker->SwitchFakeFull();
}
void MainWindow::OnWmTimer(WPARAM wParam)
{
	if (wParam == TIMER_AOP) {
		SetWindowPos(_hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
	}
	if (wParam == TIMER_RB_DELAY) {
		KillTimer(_hWnd, TIMER_RB_DELAY);
		JTLog(L"Send main path message ");
		currentWorker->RunOperation(TrainerWorkerOp1);
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
	if (cmdx == L"")
		MessageBox(hWndMain, L"请输入命令！", L"命令帮助", MB_ICONEXCLAMATION);
	else {
		bool succ = true;
		vector<wstring> cmds;
		SplitString(cmdx, cmds, L" ");
		wstring cmd = (cmds)[0];
		if (cmd == L"killst") {
			if (currentWorker->Kill())
				JTLog(L"已成功结束极域进程");
		}
		else if (cmd == L"rerunst") {
			if (currentWorker->Rerun())
				JTLog(L"已成功运行极域进程");
		}
		else if (cmd == L"ss") { currentWorker->RunOperation(TrainerWorkerOpVirusBoom); JTLog(L"已发送 ss 命令"); }
		else if (cmd == L"sss") ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
		else if (cmd == L"ssr") ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
		else if (cmd == L"ssss") KFShutdown();
		else if (cmd == L"sssr") KFReboot();
		else if (cmd == L"ckend") { currentWorker->RunOperation(TrainerWorkerOpVirusQuit); JTLog(L"已与极域分离"); }
		else if (cmd == L"fuldrv") {
			UnLoadKernelDriver(L"JiYuKillerDriver");
		}
		else if (cmd == L"fuljydrv") {
			UnLoadKernelDriver(L"TDProcHook");
		}
		else if (cmd == L"uninstall") {

		}
		else if (cmd == L"test1") {
			HMODULE fuckModul = GetModuleHandle(L"LibTDProcHook32.dll");
			if (fuckModul) {

				JTLog(L"LibTDProcHook32 : 0x%08X", fuckModul);
			}
			else JTLog(L"未加载 LibTDProcHook32.dll");
		}
		else if (cmd == L"whereisi") {
			JTLog(L"本程序路径是：%s", appCurrent->GetFullPath());
		}
		else if (cmd == L"exit")  SendMessage(hWndMain, WM_COMMAND, IDM_EXIT, NULL);
		else if (cmd == L"hide")  SendMessage(hWndMain, WM_COMMAND, IDM_SHOWMAIN, NULL);
		else {
			succ = false;
			MessageBox(0, L"未知命令\n请点击“所有命令”查看命令帮助。", L"JY Killer", 0);
		}
		if (succ) input_cmd.set_value(sciter::value(L""));
	}
}
void MainWindow::OnFirstShow()
{
	currentLogger->SetLogOutPut(LogOutPutCallback);
	currentLogger->SetLogOutPutCallback(LogCallBack, (LPARAM)this);

	//热键
	hotkeyShowHide = GlobalAddAtom(L"HotKeyShowHide");
	hotkeySwFull = GlobalAddAtom(L"HotKeySwFull");
	if (!RegisterHotKey(_hWnd, hotkeyShowHide, MOD_ALT | MOD_CONTROL, 'J'))
		JTLogWarn(L"热键 Ctrl+Alt+J 注册失败，请检查是否有程序占用，错误：%d", GetLastError());
	if (!RegisterHotKey(_hWnd, hotkeySwFull, MOD_ALT | MOD_CONTROL, 'F'))
		JTLogWarn(L"热键 Ctrl+Alt+F 注册失败，请检查是否有程序占用，错误：%d", GetLastError());

	//托盘图标
	WM_TASKBARCREATED = RegisterWindowMessage(TEXT("TaskbarCreated"));
	CreateTrayIcon(_hWnd);
	hMenuTray = LoadMenu(hInst, MAKEINTRESOURCE(IDR_MAINMENU));
	hMenuTray = GetSubMenu(hMenuTray, 0);

	//初始化控制器
	currentWorker->Init();
	currentWorker->Start();

	JTLogInfo(L"控制器已启动");
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
				SetTimer(_hWnd, TIMER_AOP, 250, NULL);
				SetWindowPos(hWndMain, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			}
		}
		else if (ele.get_attribute("id") == L"btn_kill") {
			if(currentWorker->Kill())
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

		}
		else if (ele.get_attribute("id") == L"link_exit") SendMessage(_hWnd, WM_COMMAND, IDM_EXIT, NULL);
	}
	else if (type == BUTTON_CLICK)
	{
		if (ele.get_attribute("id") == L"check_ck") {
			if (currentWorker->Running()) {
				ele.set_value(sciter::value(false));
				currentWorker->Stop();
				JTLogInfo(L"控制器已停止");
			}
			else {
				currentWorker->Start();
				ele.set_value(sciter::value(true));
				JTLogInfo(L"控制器正在运行");
			}
		}
		else if (ele.get_attribute("id") == L"btn_runcmd") {
			sciter::value cmdsx(input_cmd.get_value());
			OnRunCmd(cmdsx.to_string().c_str());
		}
		else if (ele.get_attribute("id") == L"btn_allcmd") {
			MessageBox(_hWnd, L"所有命令", L"JiYuTrainer - 帮助", 0);
		}
	}
	return false;
}

void MainWindow::OnUpdateStudentMainInfo(bool running, LPCWSTR fullPath, DWORD pid, bool byuser)
{
	if (!domComplete) return;

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

	if (StringHlp::StrEmeptyW(fullPath)) status_jiyu_path.set_text(L"未知");
	else status_jiyu_path.set_text(fullPath);
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

	switch (status)
	{
	case TrainerWorkerCallback::TrainerStatusNotFound: status_icon.set_attribute("class", L"state-not-found"); wnd.set_attribute("class", L"window-box state-notwork");  break;
	case TrainerWorkerCallback::TrainerStatusNotRunning: status_icon.set_attribute("class", L"state-not-run"); wnd.set_attribute("class", L"window-box state-notwork"); break;
	case TrainerWorkerCallback::TrainerStatusUnknowProblem: status_icon.set_attribute("class", L"state-unknow-problem"); wnd.set_attribute("class", L"window-box state-warn"); break;
	case TrainerWorkerCallback::TrainerStatusControllFailed: status_icon.set_attribute("class", L"state-failed"); wnd.set_attribute("class", L"window-box state-warn"); break;
	case TrainerWorkerCallback::TrainerStatusControlled: status_icon.set_attribute("class", L"state-ctl-no-lock"); wnd.set_attribute("class", L"window-box state-work"); break;
	case TrainerWorkerCallback::TrainerStatusControlledAndUnLocked: status_icon.set_attribute("class", L"state-ctl-unlock"); wnd.set_attribute("class", L"window-box state-work"); break;
	case TrainerWorkerCallback::TrainerStatusStopped: status_icon.set_attribute("class", L"state-manual-stop"); wnd.set_attribute("class", L"window-box state-warn"); break;
	default:
		break;
	}
}
void MainWindow::OnResolveBlackScreenWindow()
{
	if (!domComplete) return;
}
void MainWindow::OnBeforeSendStartConf()
{
	SetTimer(_hWnd, TIMER_RB_DELAY, 1500, NULL);
}

void MainWindow::ShowHelp()
{
	HelpWindow helpWindow(_hWnd);
	helpWindow.RunLoop();
}
void MainWindow::ShowFastTip(LPCWSTR text) 
{
	LPCSTR textMore2 = StringHlp::UnicodeToUtf8(text);
	tooltip_fast.set_html((UCHAR*)textMore2, strlen(textMore2));
	FreeStringPtr(textMore2);
	sciter::dom::element(get_root()).call_function("showFastTip");
}

void MainWindow::LoadSettings()
{
	SettingHlp *settings = appCurrent->GetSettings();
	setTopMost = settings->GetSettingBool(L"TopMost");
	setAutoIncludeFullWindow = settings->GetSettingBool(L"AutoIncludeFullWindow");
	setAllowAllRunOp = settings->GetSettingBool(L"AllowAllRunOp");
	setAutoForceKill = settings->GetSettingBool(L"AutoForceKill");
	setCkInterval = settings->GetSettingInt(L"CKInterval", 3100);
	if (setCkInterval < 1000 || setCkInterval > 10000) setCkInterval = 3000;
}
void MainWindow::LoadSettingsToUi()
{
	if (setTopMost) { setTopMost = false;  on_event(btn_top, btn_top, HYPERLINK_CLICK, 0); }
	else { setTopMost = true;  on_event(btn_top, btn_top, HYPERLINK_CLICK, 0); }

	check_auto_fck.set_value(sciter::value(setAutoIncludeFullWindow));
	check_allow_op.set_value(sciter::value(!setAllowAllRunOp));
	check_auto_fkill.set_value(sciter::value(setAutoForceKill));
	input_ckinterval.set_value(sciter::value(setCkInterval));
}
void MainWindow::SaveSettings()
{
	LPCWSTR setCkIntervalStr = input_ckinterval.get_value().to_string().c_str();
	setCkInterval = _wtoi(setCkIntervalStr);
	if (setCkInterval < 1000 || setCkInterval>10000) setCkInterval = 3100;

	setAutoIncludeFullWindow = check_auto_fck.get_value().get(false);
	setAllowAllRunOp = !check_allow_op.get_value().get(true);
	setAutoForceKill = check_auto_fkill.get_value().get(false);

	SettingHlp *settings = appCurrent->GetSettings();
	settings->SetSettingBool(L"TopMost", setTopMost);
	settings->SetSettingBool(L"AutoIncludeFullWindow", setAutoIncludeFullWindow);
	settings->SetSettingBool(L"AllowAllRunOp", setAllowAllRunOp);
	settings->SetSettingBool(L"AutoForceKill", setAutoForceKill);
	settings->SetSettingInt(L"CKInterval", setCkInterval);

	currentWorker->InitSettings();
}
void MainWindow::ResetSettings()
{
	setAutoIncludeFullWindow = false;
	setAllowAllRunOp = false;
	setAutoForceKill = false;
	setCkInterval = 3100;

	LoadSettingsToUi();
	SaveSettings();
}

void MainWindow::LogCallBack(const wchar_t * str, LogLevel level, LPARAM lParam)
{
	MainWindow*self = (MainWindow*)lParam;
	if (self && self->domComplete) {
		sciter::dom::element newEle = self->status_area.create("div", str);
		switch (level)
		{
		case LogLevelText:newEle.set_attribute("class", L"text-black"); break;
		case LogLevelInfo:newEle.set_attribute("class", L"text-info");  break;
		case LogLevelWarn:newEle.set_attribute("class", L"text-warning");  break;
		case LogLevelError: newEle.set_attribute("class", L"text-danger");  break;
		}
		self->status_area.append(newEle);
		newEle.scroll_to_view();
	}
	else {

	}
}

int MainWindow::RunLoop()
{
	if (!isValid())
		return -1;

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.lParam;
}

//Tray

void MainWindow::CreateTrayIcon(HWND hDlg) {
	nid.cbSize = sizeof(nid);
	nid.hWnd = hDlg;
	nid.uID = 0;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_INFO | NIF_TIP;
	nid.uCallbackMessage = WM_USER;
	nid.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_APP));
	lstrcpy(nid.szTip, L"JY Killer");
	Shell_NotifyIcon(NIM_ADD, &nid);
}
void MainWindow::ShowTrayBaloonTip(const wchar_t* title, const wchar_t* text) {
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

	MainWindow* self = ptr(hWnd);

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
	case WM_COMMAND:  self->OnWmCommand(wParam); break;
	case WM_COPYDATA: {
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
		if (wParam)
		{
			if (self->_firstShow)
			{
				self->OnFirstShow();
				self->_firstShow = false;
			}
		}
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_SYSCOMMAND: {
		switch (wParam)
		{
		case SC_RESTORE: ShowWindow(hWnd, SW_RESTORE); SetForegroundWindow(hWnd); return TRUE;
		case SC_MINIMIZE: ShowWindow(hWnd, SW_MINIMIZE);  return TRUE;
		case SC_CLOSE: {
			ShowWindow(hWnd, SW_HIDE);
			if (!self->setTopMost) SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			if (!self->hideTipShowed) {
				self->ShowTrayBaloonTip(L"JiYu Killer 提示", L"窗口隐藏到此处了，双击这里显示主界面");
				self->hideTipShowed = true;
			}
			return TRUE;
		}
		default: return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	}
	case WM_HOTKEY:   self->OnWmHotKey(wParam); break;
	case WM_DESTROY:  self->OnWmDestroy(); break;
	case WM_TIMER:  self->OnWmTimer(wParam);
	case WM_USER: self->OnWmUser(wParam, lParam); break;
	case WM_QUERYENDSESSION: {
		DestroyWindow(hWnd);
		break;
	}
	case WM_DISPLAYCHANGE: {
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
		if (self->currentWorker) self->currentWorker->UpdateScreenSize();
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
