#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "CommonWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JyUdpAttack.h"

extern JTApp* currentApp;

class ScanIPWindow : public sciter::host<ScanIPWindow>, public CommonWindow
{
public:
	ScanIPWindow(HWND parentHWnd);
	~ScanIPWindow();

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason) override;
	LRESULT onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled) override;

private:

	sciter::value docunmentComplete() override;

	sciter::dom::element ip_list;
	sciter::dom::element btn_refresh_ip_list;
	sciter::dom::element input_start_add;
	sciter::dom::element input_end_add;
	sciter::dom::element select_address;

	void RefreshIpList();
	void ScanAddIpToList(JyNetworkIP* data);
	void ScanFinish();

	int scanCount = 0;
	std::wstring scanCurrentIP;

	void AddStringToIpList(LPCWSTR str); 
	void FillIpScanInput(int index);
	void GetCurrentIP();

	std::vector<std::wstring> currentIPArr;

protected:
	bool onLoadHtml(LPCBYTE pData, DWORD len) override { return load_html(pData, len); };
};

