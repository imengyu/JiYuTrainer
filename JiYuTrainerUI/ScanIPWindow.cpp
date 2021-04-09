#include "stdafx.h"
#include "ScanIPWindow.h"
#include "resource.h"
#include "../JiYuTrainer/SysHlp.h"
#include "../JiYuTrainer/NetUtils.h"
#include "../JiYuTrainer/StringSplit.h"
#include <regex>

extern int screenWidth, screenHeight;

ScanIPWindow::ScanIPWindow(HWND parentHWnd) : CommonWindow(parentHWnd, 330, 360, L"JiYuTrainerScanIPWindow", L"扫描局域网IP", IDR_HTML_SCANIP)
{
	SetWindowLong(_hWnd, GWL_STYLE, GetWindowLong(_hWnd, GWL_STYLE) ^ (WS_MAXIMIZEBOX | WS_SIZEBOX));
	init();
	asset_add_ref();
	Show();
}
ScanIPWindow::~ScanIPWindow()
{
}

sciter::value ScanIPWindow::docunmentComplete()
{
	CommonWindow::docunmentComplete();

	sciter::dom::element root = get_root();
	ip_list = root.get_element_by_id(L"ip_list");
	input_start_add = root.get_element_by_id(L"input_start_add");
	input_end_add = root.get_element_by_id(L"input_end_add");
	select_address = root.get_element_by_id(L"select_address");
	btn_refresh_ip_list = root.get_element_by_id(L"btn_refresh_ip_list");
	select_address.attach_event_handler(this);

	GetCurrentIP();

	return sciter::value(true);
}
bool ScanIPWindow::on_event(HELEMENT he, HELEMENT target, BEHAVIOR_EVENTS type, UINT_PTR reason)
{
	sciter::dom::element ele(he);
	if (type == HYPERLINK_CLICK)
	{
		if (ele.get_attribute("data-type") == L"ip_link") {
			SysHlp::CopyToClipBoard(ele.get_attribute("data-ip").c_str());
		}
	}
	else if (type == SELECT_VALUE_CHANGED)
	{
		if (ele.get_attribute("id") == L"select_address") {
			FillIpScanInput(ele.get_value().get(0));
		}
	}
	else if (type == BUTTON_CLICK)
	{
		sciter::string id = ele.get_attribute("id");
		if (id == L"btn_refresh_ip_list" && ele.enabled()) RefreshIpList();
	}
	return false;
}
LRESULT ScanIPWindow::onWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, BOOL* handled)
{
	switch (message)
	{
	case WM_MY_SCAN_ADD_STR: AddStringToIpList((LPCWSTR)wParam); *handled = TRUE; break;
	case WM_MY_SCAN_IP_ADD: ScanAddIpToList((JyNetworkIP*)wParam); *handled = TRUE; break;
	case WM_MY_SCAN_IP_FINISH: ScanFinish(); *handled = TRUE; break;
	case WM_MY_FORCE_HIDE: Hide(); *handled = TRUE; return 0;
	}
	return 0;
}

void ScanIPWindow::GetCurrentIP() {


	std::wstring ips = NetUtils::GetIP(AF_INET);

	if (!ips.empty()) {
		SplitString(ips, currentIPArr, L"\n");

		sciter::dom::element options = select_address.find_first("popup");
		int index = 0;
		for (std::wstring ip : currentIPArr) {
			sciter::dom::element op = options.create("option", ip.c_str());
			wchar_t str[6];
			_itow_s(index++, str, 10);
			op.set_attribute("value", str);
			options.append(op);
		}

		options.refresh();
		select_address.set_value(sciter::value(0));

		FillIpScanInput(0);
	}
}
void ScanIPWindow::FillIpScanInput(int index) {

	scanCurrentIP = currentIPArr[index];
	std::wregex express(L"^(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}$");
	if (std::regex_match(scanCurrentIP.begin(), scanCurrentIP.end(), express)) {
		int lastDotIndex = scanCurrentIP.find_last_of(L'.');
		if (lastDotIndex > 0) {
			std::wstring ind = scanCurrentIP.substr(0, lastDotIndex);

			input_start_add.set_value(sciter::value((ind + L".0").c_str()));
			input_end_add.set_value(sciter::value((ind + L".255").c_str()));
		}
	}
}

void ScanIPWindow::ScanAddIpToList(JyNetworkIP* data) {
	scanCount++;

	sciter::dom::element newEle = ip_list.create("div");
	ip_list.append(newEle);

	sciter::dom::element aEle = newEle.create("a", data->ipAddress.c_str());
	sciter::dom::element spanEle = newEle.create("span", data->hostName.c_str());
	aEle.set_attribute("data-type", L"ip_link");
	aEle.set_attribute("data-ip", data->ipAddress.c_str());
	newEle.append(aEle);
	newEle.append(spanEle);

	delete data;
}
void ScanIPWindow::ScanFinish() {

	btn_refresh_ip_list.set_text(L"扫描完成，重新扫描");
	btn_refresh_ip_list.remove_attribute("disabled");

	if (scanCount == 0)
		AddStringToIpList(L"没有搜索到局域网内的主机");
}
void ScanIPWindow::AddStringToIpList(LPCWSTR str) {
	sciter::dom::element newEle = ip_list.create("div", str);
	ip_list.append(newEle);
}
void ScanIPWindow::RefreshIpList() {

	std::wstring start = input_start_add.get_value().get(L"").c_str();
	std::wstring end = input_end_add.get_value().get(L"").c_str();
	std::wregex express(L"^(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\.(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}$");

	if (!std::regex_match(start.begin(), start.end(), express)) {
		MessageBox(_hWnd, L"起始地址不正确，只支持IPV4地址 (xx.xx.xx.xx) 格式", L"提示", MB_ICONEXCLAMATION);
		return;
	}
	if (!std::regex_match(end.begin(), end.end(), express)) {
		MessageBox(_hWnd, L"结束地址不正确，只支持IPV4地址 (xx.xx.xx.xx) 格式", L"提示", MB_ICONEXCLAMATION);
		return;
	}

	std::wstring ipHost1, ipHost2;
	int lastDotIndex = start.find_last_of(L'.');
	if (lastDotIndex > 0)
		ipHost1 = start.substr(0, lastDotIndex);
	lastDotIndex = end.find_last_of(L'.');
	if (lastDotIndex > 0)
		ipHost2 = end.substr(0, lastDotIndex);

	if (ipHost1 != ipHost2) {
		MessageBox(_hWnd, L"起始地址和结束地址必须在同一个网段", L"提示", MB_ICONEXCLAMATION);
		return;
	}


	scanCount = 0;
	ip_list.clear();
	btn_refresh_ip_list.set_text(L"正在扫描请稍后...");
	btn_refresh_ip_list.set_attribute("disabled", L"disabled");
	JyUdpAttack::currentJyUdpAttack->ScanNetworkIP(this->_hWnd, start, end, ipHost1, scanCurrentIP);
}

