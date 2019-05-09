#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"

extern HINSTANCE hInst;

class UpdaterWindow : public sciter::host<UpdaterWindow>, public sciter::event_handler
{
public:
	UpdaterWindow(HWND parentHWnd);
	~UpdaterWindow();

	HWND _hWnd, _parentHWnd;
	bool _firstShow = true;

	int RunLoop();

	static LRESULT CALLBACK	wndProc(HWND, UINT, WPARAM, LPARAM);
	static UpdaterWindow* ptr(HWND hwnd);
	static bool initClass();

public:
	// notification_handler traits:
	HWND get_hwnd() { return _hWnd; }
	HINSTANCE get_resource_instance() { return hInst; }

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)  override;

	bool init(); // instance
	bool isValid() const { return _hWnd != 0; }

private:

	bool firstShow = true;

	std::wstring updateProgressPrect;
	sciter::dom::element progress;
	sciter::dom::element progress_text;

	void OnCancel();
	void OnFirstShow();
	void OnUpdateDownloadCallback(LPCWSTR precent, int status);

	static void UpdateDownloadCallback(LPCWSTR precent, LPARAM lParam, int status);
};

