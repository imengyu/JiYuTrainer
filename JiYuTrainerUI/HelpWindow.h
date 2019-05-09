#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"

extern HINSTANCE hInst;

class HelpWindow  : public sciter::host<HelpWindow>
{
public:
	HelpWindow(HWND parentHWnd);
	~HelpWindow();

	HWND _hWnd, _parentHWnd;
	bool _firstShow = true;

	int RunLoop();

	static LRESULT CALLBACK	wndProc(HWND, UINT, WPARAM, LPARAM);
	static HelpWindow* ptr(HWND hwnd);
	static bool initClass();

public:
	// notification_handler traits:
	HWND get_hwnd() { return _hWnd; }
	HINSTANCE get_resource_instance() { return hInst; }

	bool init(); // instance
	bool isValid() const { return _hWnd != 0; }
};

