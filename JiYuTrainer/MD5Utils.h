#pragma once
#include "stdafx.h"
#include "md5.h"
#include <string>

class MD5Utils
{
public:
	std::wstring *GetFileMD5(LPCWSTR filePath) ;
	std::wstring *GetStringMD5(LPCWSTR filePath);
};

