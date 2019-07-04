#pragma once
#include "stdafx.h"
#include "md5.h"
#include <string>

class MD5Utils
{
public:
	virtual std::wstring *GetFileMD5(LPCWSTR filePath) { return nullptr; }
	virtual std::wstring *GetStringMD5(LPCWSTR filePath) { return nullptr; }
};
class MD5UtilsInternal : public MD5Utils
{
public:
	std::wstring *GetFileMD5(LPCWSTR filePath) override;
	std::wstring *GetStringMD5(LPCWSTR filePath) override;
};

