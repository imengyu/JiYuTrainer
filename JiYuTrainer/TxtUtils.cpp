#include "stdafx.h"
#include "TxtUtils.h"


bool TxtUtils::WriteStringToTxt(std::wstring &filePath, std::wstring &content)
{
	FILE*fp = NULL;
	_wfopen_s(&fp, filePath.c_str(), L"w");
	if (fp) {
		fwprintf_s(fp, content.c_str());
		fclose(fp);
		return true;
	}
	return false;
}
