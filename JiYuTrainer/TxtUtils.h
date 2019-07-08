#pragma once
#include "stdafx.h"
#include <string>

class TxtUtils
{
public:

	static bool WriteStringToTxt(std::wstring &filePath, std::wstring &content);
};

