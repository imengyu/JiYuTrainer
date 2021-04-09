#include "stdafx.h"
#include "UpdaterWindow.h"
#include "MainWindow.h"
#include "resource.h"
#include "../JiYuTrainerUpdater/JiYuTrainerUpdater.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "JiYuTrainerUI.h"

extern int screenWidth, screenHeight;
extern MainWindow *currentMainWindow;

UpdaterWindow::UpdaterWindow(HWND parentHWnd) : CommonWindow(parentHWnd, 430, 220, L"sciter-jytrainer-update-window", L"JiYu Trainer Update Window", IDR_HTML_UPDATER)
{
	init();
	Show();
}
UpdaterWindow::~UpdaterWindow()
{
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
LRESULT UpdaterWindow::onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled)
{
	switch (message)
	{
	case WM_CREATE: {
		SetTimer(hWnd, 12, 1000, 0);
		break;
	}
	case WM_SYSCOMMAND: {
		if (wParam == SC_CLOSE) {
			OnCancel();
			return TRUE;
		}
	}
	case WM_DESTROY: {
		KillTimer(hWnd, 12);
		SetWindowLong(hWnd, GWL_USERDATA, 0);
		PostQuitMessage(0);
		break;
	}
	case WM_QUERYENDSESSION: {
		DestroyWindow(hWnd);
		break;
	}
	case WM_SHOWWINDOW: {
		if (firstShow) {
			firstShow = false;
			//窗口居中
			RECT rect; GetWindowRect(hWnd, &rect);
			rect.left = (screenWidth - (rect.right - rect.left)) / 2;
			rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
			SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

			OnFirstShow();
		}
		break;
	}
	case WM_TIMER: {
		if (wParam == 12) {
			progress.set_style_attribute("width", updateProgressPrect.c_str());
			progress_value.set_text(updateProgressPrect.c_str());
		}
		break;
	}
	case WM_COMMAND: {
		if (wParam == IDC_UPDATE_CLOSE)
			Close();
		break;
	}
	}
	return 0;
}

void UpdaterWindow::OnCancel()
{
	if (JUpdater_Updatering()) {
		if (MessageBox(_hWnd, L"正在下载更新，您是否确定要取消升级？", L"提示", MB_ICONINFORMATION | MB_YESNO) == IDYES)
		{
			if (JUpdater_CancelDownLoadUpdateFile())
				MessageBox(_hWnd, L"更新已取消", L"提示", MB_ICONINFORMATION);
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
			SendMessage(currentMainWindow->get_hwnd(), WM_COMMAND, IDC_UPDATE_CLOSE, NULL);
	}
}
