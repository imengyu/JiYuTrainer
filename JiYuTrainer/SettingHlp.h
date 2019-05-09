#pragma once
#include "stdafx.h"
#include <string>

#define SETTING_DAFAULT_APP_KEY L"JTSettings"

class EXPORT SettingHlp
{
public:
	SettingHlp(LPCWSTR setttingFilePath);
	~SettingHlp();

	bool GetSettingBool(LPCWSTR key, bool defaultValue = true, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	int GetSettingInt(LPCWSTR key, int defaultValue = 0, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	std::wstring GetSettingStr(LPCWSTR key, LPCWSTR defaultValue = L"", size_t bufferSize = 32, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);

	bool SetSettingBool(LPCWSTR key, bool value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	bool SetSettingInt(LPCWSTR key, int value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	bool SetSettingStr(LPCWSTR key, LPCWSTR value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	bool SetSettingStr(LPCWSTR key, std::wstring value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
private:
	WCHAR setttingFilePath[MAX_PATH];
};

