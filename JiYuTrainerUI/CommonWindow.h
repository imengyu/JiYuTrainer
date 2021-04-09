#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JyUdpAttack.h"

extern JTApp* currentApp;


class CommonWindow : public sciter::event_handler
{
public:
	CommonWindow(HWND parentHWnd, int w, int h, LPCWSTR className, LPCWSTR title, int htmlResId);
	~CommonWindow();

	HWND _hWnd, _parentHWnd;

	int RunLoop();

	void Release();
	void Close();
	void Show();
	void Hide();
	void SetToTop();

	BEGIN_FUNCTION_MAP
		FUNCTION_0("docunmentComplete", docunmentComplete);
		FUNCTION_0("inspectorIsPresent", inspectorIsPresent);
	END_FUNCTION_MAP

protected:

	static LRESULT CALLBACK	wndProc(HWND, UINT, WPARAM, LPARAM);
	static CommonWindow* ptr(HWND hwnd);
	bool initClass();

	virtual LRESULT onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled);

	virtual bool init();
	virtual void destroy();

	virtual sciter::value docunmentComplete();
	virtual sciter::value inspectorIsPresent();

	virtual bool onLoadHtml(LPCBYTE pData, DWORD len);

	LPCWSTR wndClassName = nullptr;
	int wndHtmlResId = 0;
public:
	// notification_handler traits:
	HWND get_hwnd() { return _hWnd; }
	HINSTANCE get_resource_instance() { return currentApp->GetInstance(); }

	bool isValid() const { return _hWnd != 0; }
};

