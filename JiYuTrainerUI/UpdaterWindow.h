#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "CommonWindow.h"
#include "../JiYuTrainer/AppPublic.h"

extern JTApp* currentApp;

#define IDC_UPDATE_CLOSE 40012

class UpdaterWindow : public sciter::host<UpdaterWindow>, public CommonWindow
{
public:
	UpdaterWindow(HWND parentHWnd);
	~UpdaterWindow();

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason) override;
	LRESULT onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled) override;
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

protected:
	bool onLoadHtml(LPCBYTE pData, DWORD len) override { return load_html(pData, len); };
};

