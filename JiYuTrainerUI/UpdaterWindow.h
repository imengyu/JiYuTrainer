#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "../JiYuTrainer/AppPublic.h"

extern JTApp* currentApp;

#define IDC_UPDATE_CLOSE 40012

class UpdaterWindow : public sciter::host<UpdaterWindow>, public sciter::event_handler
{
public:
	UpdaterWindow(HWND parentHWnd);
	~UpdaterWindow();

	HWND _hWnd, _parentHWnd;
	bool _firstShow = true;

	int RunLoop();

	void Close();

	static LRESULT CALLBACK	wndProc(HWND, UINT, WPARAM, LPARAM);
	static UpdaterWindow* ptr(HWND hwnd);
	static bool initClass();

public:
	// notification_handler traits:
	HWND get_hwnd() { return _hWnd; }
	HINSTANCE get_resource_instance() { return currentApp->GetInstance(); }

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)  override;

	bool init(); // instance
	bool isValid() const { return _hWnd != 0; }

private:

	bool firstShow = true;
	bool setDt1 = false;

	std::wstring updateProgressPrect;
	sciter::dom::element progress;
	sciter::dom::element progress_text;
	sciter::dom::element progress_value;

	void OnCancel();
	void OnFirstShow();
	void OnUpdateDownloadCallback(LPCWSTR precent, int status);

	static void UpdateDownloadCallback(LPCWSTR precent, LPARAM lParam, int status);
};

