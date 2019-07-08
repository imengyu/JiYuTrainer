#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "../JiYuTrainer/AppPublic.h"

extern JTApp* currentApp;

class HelpWindow  : public sciter::host<HelpWindow>, public sciter::event_handler
{
public:
	HelpWindow(HWND parentHWnd);
	~HelpWindow();

	HWND _hWnd, _parentHWnd;
	bool _firstShow = true;

	int RunLoop();
	void Close();

	static LRESULT CALLBACK	wndProc(HWND, UINT, WPARAM, LPARAM);
	static HelpWindow* ptr(HWND hwnd);
	static bool initClass();

	sciter::value goToFullHelp();

	bool init();

	BEGIN_FUNCTION_MAP
		FUNCTION_0("goToFullHelp", goToFullHelp);
	END_FUNCTION_MAP

public:
	// notification_handler traits:
	HWND get_hwnd() { return _hWnd; }
	HINSTANCE get_resource_instance() { return currentApp->GetInstance(); }

	bool isValid() const { return _hWnd != 0; }
};

