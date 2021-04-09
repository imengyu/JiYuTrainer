#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "CommonWindow.h"
#include "../JiYuTrainer/AppPublic.h"

extern JTApp* currentApp;

class HelpWindow : public CommonWindow, public sciter::host<HelpWindow>
{
public:
	HelpWindow(HWND parentHWnd);
	~HelpWindow();

private:
	sciter::dom::element root;
protected:
	sciter::value docunmentComplete();

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason) override;
	LRESULT onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled) override;
	bool onLoadHtml(LPCBYTE pData, DWORD len) override { return load_html(pData, len); };
};

