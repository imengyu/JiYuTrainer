#include "stdafx.h"
#include "MsgCenter.h"
#include <list>

std::list<HWND> jiyuWnds;

void MsgCenterSendToVirus(LPCWSTR buff, HWND form) 
{
	HWND receiveWindow = FindWindow(NULL, L"JiYu Trainer Virus Window");
	if (receiveWindow) {
		COPYDATASTRUCT copyData = { 0 };
		copyData.lpData = (PVOID)buff;
		copyData.cbData = sizeof(WCHAR) * (wcslen(buff) + 1);
		SendMessageTimeout(receiveWindow, WM_COPYDATA, (WPARAM)form, (LPARAM)&copyData, SMTO_ABORTIFHUNG | SMTO_NORMAL, 500, 0);
	}
}

bool IsInIllegalWindows(HWND hWnd) {
	std::list<HWND>::iterator testiterator;
	for (testiterator = jiyuWnds.begin(); testiterator != jiyuWnds.end(); ++testiterator)
	{
		if ((*testiterator) == hWnd)
			return true;
	}
	return false;
}
void MsgCenteAppendHWND(HWND hWnd)
{
	if (!IsInIllegalWindows(hWnd)) jiyuWnds.push_back(hWnd);
}
void MsgCenterSendHWNDS(HWND fromHWnd)
{
	int iwndCount = jiyuWnds.size();
	std::list<HWND>::iterator testiterator;
	for (testiterator = jiyuWnds.begin(); testiterator != jiyuWnds.end(); ++testiterator)
	{
		HWND hWnd = (*testiterator);
		WCHAR str[65];
		swprintf_s(str, L"hw:%d", (LONG)hWnd);
		MsgCenterSendToVirus(str, fromHWnd);
	}
	jiyuWnds.clear();
	//ResetGBStatus(iwndCount, lastHasGb, lastHasHp);
}