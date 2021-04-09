#include "stdafx.h"
#include "CommonWindow.h"
#include "ScanIPWindow.h"
#include "resource.h"
#include "../JiYuTrainer/SysHlp.h"
#include "../JiYuTrainer/NetUtils.h"
#include "../JiYuTrainer/StringSplit.h"
#include <regex>

extern int screenWidth, screenHeight;

CommonWindow::CommonWindow(HWND parentHWnd, int w, int h, LPCWSTR c, LPCWSTR title, int htmlResId)
{
	_parentHWnd = parentHWnd;
	wndClassName = c;
	wndHtmlResId = htmlResId;

	if (!initClass()) return;

	_hWnd = CreateWindowExW(0, wndClassName, title,
		WS_OVERLAPPEDWINDOW ^ (WS_MAXIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, 
		w, h, nullptr, nullptr, currentApp->GetInstance(), this);
	if (!_hWnd) return;
	

	asset_add_ref();
	attach_dom_event_handler(_hWnd, this); // to receive DOM events
	UpdateWindow(_hWnd);
}
CommonWindow::~CommonWindow()
{
	fuck();
	_hWnd = nullptr;
}

//公共方法

int CommonWindow::RunLoop()
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

	SetForegroundWindow(_parentHWnd);

	return msg.lParam;
}
void CommonWindow::Release()
{
	delete this;
}
void CommonWindow::Close()
{
	DestroyWindow(_hWnd);
}
void CommonWindow::SetToTop()
{
	SetForegroundWindow(_hWnd);
}
void CommonWindow::Show()
{
	ShowWindow(_hWnd, SW_SHOW);
	SetForegroundWindow(_hWnd);
}
void CommonWindow::Hide()
{
	ShowWindow(_hWnd, SW_HIDE);
}

//窗口处理函数

CommonWindow* CommonWindow::ptr(HWND hwnd)
{
	return reinterpret_cast<CommonWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}
bool CommonWindow::initClass()
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
	wcex.hInstance = currentApp->GetInstance();
	wcex.hIcon = LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_APP));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = wndClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP));

	if (RegisterClassExW(&wcex) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		return TRUE;
	return FALSE;
}

LRESULT CommonWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//SCITER integration starts
	BOOL handled = FALSE;
	LRESULT lr = SciterProcND(hWnd, message, wParam, lParam, &handled);
	if (handled)
		return lr;
	//SCITER integration ends

	CommonWindow* _this = ptr(hWnd);
	lr = _this ? _this->onWndProc(hWnd, message, wParam, lParam, &handled) : 0;
	if (handled)
		return lr;

	switch (message)
	{
	case WM_GETMINMAXINFO: {
		MINMAXINFO* lpmm = (MINMAXINFO*)lParam;
		lpmm->ptMaxTrackSize = { 625, 660 };
		lpmm->ptMinTrackSize = { 360, 360 };
		return 0;
	}
	case WM_DESTROY: {
		_this->destroy();
		break;
	}
	case WM_QUERYENDSESSION: {
		DestroyWindow(hWnd);
		break;
	}
	case WM_SHOWWINDOW: {
		//窗口居中
		RECT rect; GetWindowRect(hWnd, &rect);
		rect.left = (screenWidth - (rect.right - rect.left)) / 2;
		rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
LRESULT CommonWindow::onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled)
{
	*handled = FALSE;
	return 0;
}

bool CommonWindow::init()
{
	SetWindowLongPtr(_hWnd, GWLP_USERDATA, LONG_PTR(this));	
	BOOL result = FALSE;
	HRSRC hResource = FindResource(currentApp->GetInstance(), MAKEINTRESOURCE(wndHtmlResId), RT_HTML);
	if (hResource) {
		HGLOBAL hg = LoadResource(currentApp->GetInstance(), hResource);
		if (hg) {
			LPVOID pData = LockResource(hg);
			if (pData)
				result = onLoadHtml((LPCBYTE)pData, SizeofResource(currentApp->GetInstance(), hResource));
		}
	}

	return result;
}
void CommonWindow::destroy() {
	SetWindowLong(_hWnd, GWL_USERDATA, 0);
	PostMessage(_parentHWnd, WM_MY_WND_CLOSE, (WPARAM)this, 0);
}

sciter::value CommonWindow::inspectorIsPresent()
{
	HWND hwnd = FindWindow(WSTR("H-SMILE-FRAME"), WSTR("Sciter's Inspector"));
	return sciter::value(hwnd != NULL);
}
bool CommonWindow::onLoadHtml(LPCBYTE pData, DWORD len)
{
	return false;
}
sciter::value CommonWindow::docunmentComplete()
{
	return sciter::value(true);
}

