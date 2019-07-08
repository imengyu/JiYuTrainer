#include "stdafx.h"
#include "HelpWindow.h"
#include "resource.h"
#include "../JiYuTrainer/SysHlp.h"

extern int screenWidth, screenHeight;

HelpWindow::HelpWindow(HWND parentHWnd)
{
	_parentHWnd = parentHWnd;

	if (!initClass()) return;

	_hWnd = CreateWindowExW(0, L"JiYuTrainerHelpWindow", L"JiYu Trainer Help Window", WS_OVERLAPPEDWINDOW ^ (WS_MAXIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, 470, 520, nullptr, nullptr, currentApp->GetInstance(), this);
	if (!_hWnd) return;
	
	init();

	ShowWindow(_hWnd, SW_SHOW);
	UpdateWindow(_hWnd);
}
HelpWindow::~HelpWindow()
{
}

int HelpWindow::RunLoop()
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
void HelpWindow::Close()
{
	DestroyWindow(_hWnd);
}

LRESULT HelpWindow::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//SCITER integration starts
	BOOL handled = FALSE;
	LRESULT lr = SciterProcND(hWnd, message, wParam, lParam, &handled);
	if (handled)
		return lr;
	//SCITER integration ends

	switch (message)
	{
	case WM_GETMINMAXINFO: {
		MINMAXINFO* lpmm = (MINMAXINFO*)lParam;
		lpmm->ptMaxTrackSize = { 625, 660 };
		lpmm->ptMinTrackSize = { 360, 360 };
		return 0;
	}
	case WM_DESTROY: {
		SetWindowLong(hWnd, GWL_USERDATA, 0);
		PostQuitMessage(0);
		break;
	}
	case WM_QUERYENDSESSION: {
		DestroyWindow(hWnd);
		break;
	}
	case WM_SHOWWINDOW: {
		//´°¿Ú¾ÓÖÐ
		RECT rect; GetWindowRect(hWnd, &rect);
		rect.left = (screenWidth - (rect.right - rect.left)) / 2;
		rect.top = (screenHeight - (rect.bottom - rect.top)) / 2 - 60;
		SetWindowPos(hWnd, 0, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
		break;
	}
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}
HelpWindow * HelpWindow::ptr(HWND hwnd)
{
	return reinterpret_cast<HelpWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
}
bool HelpWindow::initClass()
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
	wcex.lpszMenuName = 0;//MAKEINTRESOURCE(IDC_PLAINWIN);
	wcex.lpszClassName = L"JiYuTrainerHelpWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APP));

	if (RegisterClassExW(&wcex) || GetLastError() == ERROR_CLASS_ALREADY_EXISTS)
		return TRUE;
	return FALSE;
}
sciter::value HelpWindow::goToFullHelp()
{
	return sciter::value::null();
}
bool HelpWindow::init()
{
	SetWindowLongPtr(_hWnd, GWLP_USERDATA, LONG_PTR(this));
	setup_callback(); // to receive SC_LOAD_DATA, SC_DATA_LOADED, etc. notification
	BOOL result = FALSE;
	HRSRC hResource = FindResource(currentApp->GetInstance(), MAKEINTRESOURCE(IDR_HTML_ABOUT), RT_HTML);
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


