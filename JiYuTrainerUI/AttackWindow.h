#pragma once
#include "stdafx.h"
#include "sciter-x.h"
#include "sciter-x-host-callback.h"
#include "CommonWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JyUdpAttack.h"

extern JTApp* currentApp;

class ScanIPWindow;
class AttackWindow : public sciter::host<AttackWindow>, public CommonWindow
{
public:
	AttackWindow(HWND parentHWnd);
	~AttackWindow();

private:

	bool on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason) override;
	LRESULT onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled) override;

	ScanIPWindow* currentScanIPWnd = nullptr;

	sciter::dom::element result_list;
	sciter::dom::element input_target_ip;
	sciter::dom::element input_target_port;
	sciter::dom::element input_send_message;
	sciter::dom::element input_send_cmd;
	
	bool SendCommonCheck(std::vector<std::wstring>* ip, DWORD* port);
	void SendCmd();
	void SendText();
	void SendShutdown();
	void SendReboott();
	void AddStringToReult(LPCWSTR str);

protected:
	bool onLoadHtml(LPCBYTE pData, DWORD len) override { return load_html(pData, len); }
	sciter::value docunmentComplete() override;

};

