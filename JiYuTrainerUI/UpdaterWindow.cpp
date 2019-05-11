#include "stdafx.h"
#include "UpdaterWindow.h"
#include "MainWindow.h"
#include "resource.h"
#include "../JiYuTrainerUpdater/JiYuTrainerUpdater.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "JiYuTrainerUI.h"

extern int screenWidth, screenHeight;

UpdaterWindow::UpdaterWindow(HWND parentHWnd)
{
	_parentHWnd = parentHWnd;

	if(!initClass()) return;

	_hWnd = CreateWindowExW(0, L"sciter-jytrainer-update-window", L"JiYu Trainer Help Window", WS_OVERLAPPEDWINDOW ^ (WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, 430, 220, nullptr, nullptr, hInst, this);
	if (!_hWnd) return;

	init();

	ShowWindow(_hWnd, SW_SHOW);
	UpdateWindow(_hWnd);
}
UpdaterWindow::~UpdaterWindow()
{
}

int UpdaterWindow::RunLoop()
{
	if (!isValid())
		return -1;

	EnableWindow(_parentHWnd, FALSE);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	EnableWindow(_parentHWnd, TRUE);
	SetForegroundWindow(_parentHWnd);

	return msg.lParam;
}
void UpdaterWindow::Close()
{
	DestroyWindow(_hWnd);
}

LRESULT UpdaterWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//SCITER integration starts
	BOOL handled = FALSE;
	LRESULT lr = SciterProcND(hWnd, message, wParam, lParam, &handled);
	if (handled)
		return lr;
	//SCITER integration ends

	UpdaterWindow *self = ptr(hWnd);
	switch (message)
	{
	case WM_CREATE: {
		SetTimer(hWnd, 12, 1000, 0);
		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE) {
			self->OnCancel();
			return TRUE;
		}
	}
	case WM_DESTROY: {
		KillTimer(hWnd, 12);
		PostQuitMessage(0);
		break;
	}
	case WM_QUERYENDSESSION: {
		DestroyWindow(hWnd);
		break;
	}
	case WM_SHOWWINDOW: {
		if (self->firstShow) {		
			self->firstShow = false;
			//窗口居中
			RECT rect; GetWindowRect(hWnd, &rect);
			rect.left = (screenWidth - (rect.right - rect.left)) / 2;
			rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
			SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

			self->OnFirstShow();
		}
		break;
	}
	case WM_TIMER: {
		if (wParam == 12) {
			self->progress.set_style_attribute("width", self->updateProgressPrect.c_str());
			self->progress_value.set_text(self->updateProgressPrect.c_str());
		}
		break;
	}
	case WM_COMMAND: {
		if (wParam == IDC_UPDATE_CLOSE)
			self->Close();
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
UpdaterWindow * UpdaterWindow::ptr(HWND hwnd)
{
	return reinterpret_cast<UpdaterWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}
bool UpdaterWindow::initClass()
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
	wcex.lpszClassName = L"sciter-jytrainer-update-window";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP));

	if (RegisterClassExW(&wcex) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		return TRUE;
	return FALSE;
}
bool UpdaterWindow::on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)
{
	sciter::dom::element ele(he);
	if (type == HYPERLINK_CLICK)
	{
		if (ele.get_attribute("id") == L"link_exit")
		{
			SendMessage(_hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL);
			return true;
		}
	}
	if (type == DOCUMENT_COMPLETE) {
		sciter::dom::element root(get_root());
		progress = root.get_element_by_id(L"progress");
		progress_text = root.get_element_by_id(L"progress_text");
		progress_value = root.get_element_by_id(L"progress_value");
	}
	return false;
}
bool UpdaterWindow::init()
{
	SetWindowLongPtr(_hWnd, GWLP_USERDATA, LONG_PTR(this));
	setup_callback(); // to receive SC_LOAD_DATA, SC_DATA_LOADED, etc. notification
	attach_dom_event_handler(_hWnd, this); // to receive DOM events
	BOOL result = FALSE;
	HRSRC hResource = FindResource(hInst, MAKEINTRESOURCE(IDR_HTML_UPDATER), RT_HTML);
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
void UpdaterWindow::OnCancel()
{
	if (JUpdater_Updatering()) {
		if (MessageBox(0, L"正在下载更新，您是否确定要取消升级？", L"提示", MB_ICONINFORMATION | MB_YESNO) == IDYES)
		{
			if (JUpdater_CancelDownLoadUpdateFile())
				JTLog(L"更新已取消");
			Close();
		}
	}
	else {
		Close();
	}
}
void UpdaterWindow::OnFirstShow()
{
	JUpdater_DownLoadUpdateFile(UpdateDownloadCallback, (LPARAM)this);
}
void UpdaterWindow::UpdateDownloadCallback(LPCWSTR precent, LPARAM lParam, int status)
{
	UpdaterWindow*ptr = (UpdaterWindow*)lParam;
	ptr->OnUpdateDownloadCallback(precent, status);
}
void UpdaterWindow::OnUpdateDownloadCallback(LPCWSTR precent, int status)
{
	if (status == UPDATE_STATUS_COULD_NOT_CONNECT) {
		progress_text.set_text(L"更新下载失败！");
		MessageBox(_hWnd, L"无法连接至更新服务器", L"JiYuTrainer - 更新错误", MB_ICONEXCLAMATION);
		SendMessage(_hWnd, WM_COMMAND, IDC_UPDATE_CLOSE, NULL);
	}
	else if (status == UPDATE_STATUS_COULD_NOT_CREATE_FILE) {
		progress_text.set_text(L"更新安装失败！");
		MessageBox(_hWnd, L"无法写入更新文件，您可以尝试使用管理员身份运行本程序", L"JiYuTrainer - 更新错误", MB_ICONEXCLAMATION);
		SendMessage(_hWnd, WM_COMMAND, IDC_UPDATE_CLOSE, NULL);
	}
	else if (status == UPDATE_STATUS_DWONLAODING) {
		if (!setDt1) {
			progress_text.set_text(L"正在下载更新：");
			setDt1 = true;
		}
		updateProgressPrect = precent;
	}
	else if (status == UPDATE_STATUS_FINISHED) {
		progress_text.set_text(L"更新下载完成！");
		SendMessage(_hWnd, WM_COMMAND, IDC_UPDATE_CLOSE, NULL);
		if (JUpdater_RunInstallion())
			SendMessage(((MainWindow *)JTUI_GetMainWindow())->get_hwnd(), WM_COMMAND, IDC_UPDATE_CLOSE, NULL);
	}
}
