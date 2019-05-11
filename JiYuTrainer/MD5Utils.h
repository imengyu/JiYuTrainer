#pragma once
#include "stdafx.h"
#include "md5.h"
#include <string>

class EXPORT MD5Utils
{
public:
	MD5Utils();
	~MD5Utils();

	static std::wstring *GetFileMD5(LPCWSTR filePath);
	static std::wstring *GetStringMD5(LPCWSTR filePath);
};

