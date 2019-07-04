#pragma once
#include "stdafx.h"
#include <string>

#define SETTING_DAFAULT_APP_KEY L"JTSettings"

class SettingHlp
{
public:
	virtual bool GetSettingBool(LPCWSTR key, bool defaultValue = true, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return false; }
	virtual int GetSettingInt(LPCWSTR key, int defaultValue = 0, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return 0; }
	virtual std::wstring GetSettingStr(LPCWSTR key, LPCWSTR defaultValue = L"", size_t bufferSize = 32, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return std::wstring(); }
	virtual std::wstring* GetSettingStrPtr(LPCWSTR key, LPCWSTR defaultValue = L"", size_t bufferSize = 32, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return nullptr; }

	virtual bool SetSettingBool(LPCWSTR key, bool value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return false; }
	virtual bool SetSettingInt(LPCWSTR key, int value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return false; }
	virtual bool SetSettingStr(LPCWSTR key, LPCWSTR value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return false; }
	virtual bool SetSettingStr(LPCWSTR key, std::wstring value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { return false; }
};
class SettingHlpInternal : public SettingHlp 
{
public:
	SettingHlpInternal(LPCWSTR setttingFilePath);
	~SettingHlpInternal();

	bool GetSettingBool(LPCWSTR key, bool defaultValue = true, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	int GetSettingInt(LPCWSTR key, int defaultValue = 0, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	std::wstring GetSettingStr(LPCWSTR key, LPCWSTR defaultValue = L"", size_t bufferSize = 32, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	virtual std::wstring* GetSettingStrPtr(LPCWSTR key, LPCWSTR defaultValue = L"", size_t bufferSize = 32, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY) { 
		return new std::wstring(GetSettingStr(key, defaultValue, bufferSize, appKey));
	}

	bool SetSettingBool(LPCWSTR key, bool value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	bool SetSettingInt(LPCWSTR key, int value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	bool SetSettingStr(LPCWSTR key, LPCWSTR value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
	bool SetSettingStr(LPCWSTR key, std::wstring value, LPCWSTR appKey = SETTING_DAFAULT_APP_KEY);
private:
	WCHAR setttingFilePath[MAX_PATH];
};

