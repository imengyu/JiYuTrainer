#include "stdafx.h"
#include "SettingHlp.h"
#include "StringHlp.h"

SettingHlpInternal::SettingHlpInternal(LPCWSTR setttingFilePath)
{
	wcscpy_s(this->setttingFilePath, setttingFilePath);
}
SettingHlpInternal::~SettingHlpInternal()
{
}

bool SettingHlpInternal::GetSettingBool(LPCWSTR key, bool defaultValue, LPCWSTR appKey)
{
	WCHAR w[16];
	GetPrivateProfileString(appKey, key, defaultValue ? L"TRUE" : L"FALSE", w, 16, setttingFilePath);
	return StringHlp::StrToBoolW(w);
}
int SettingHlpInternal::GetSettingInt(LPCWSTR key, int defaultValue, LPCWSTR appKey)
{
	WCHAR w[32];
	swprintf_s(w, L"%d", defaultValue);
	GetPrivateProfileString(appKey, key, w, w, 32, setttingFilePath);
	return StringHlp::StrToIntW(w);
}
std::wstring SettingHlpInternal::GetSettingStr(LPCWSTR key, LPCWSTR defaultValue, size_t bufferSize, LPCWSTR appKey)
{
	std::wstring tmp;
	tmp.resize(bufferSize + 1);
	GetPrivateProfileString(appKey, key, defaultValue, (wchar_t *)tmp.data(), bufferSize + 1, setttingFilePath);
	return tmp;
}

bool SettingHlpInternal::SetSettingBool(LPCWSTR key, bool value, LPCWSTR appKey)
{
	return WritePrivateProfileString(appKey, key, value ? L"TRUE" : L"FALSE", setttingFilePath);
}
bool SettingHlpInternal::SetSettingInt(LPCWSTR key, int value, LPCWSTR appKey)
{
	LPCWSTR intStr = StringHlp::IntToStrW(value);
	bool rs =  WritePrivateProfileString(appKey, key, intStr, setttingFilePath);
	delete intStr;
	return rs;
}
bool SettingHlpInternal::SetSettingStr(LPCWSTR key, LPCWSTR value, LPCWSTR appKey)
{
	return WritePrivateProfileString(appKey, key, value, setttingFilePath);
}
bool SettingHlpInternal::SetSettingStr(LPCWSTR key, std::wstring value, LPCWSTR appKey)
{
	return SetSettingStr(key, value.c_str(), appKey);
}
