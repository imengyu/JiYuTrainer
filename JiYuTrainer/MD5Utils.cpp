#include "stdafx.h"
#include "MD5Utils.h"
#include <time.h>

#define INPUT_BUFFER_SIZE 100 * 1024

std::wstring* MD5Utils::GetFileMD5(LPCWSTR  filePath)
{
	std::wstring *result = new 	std::wstring();
	FILE *file;
	_wfopen_s(&file, filePath, L"rb");
	if (file)
	{
		time_t startTime = time(NULL);
		md5_state_t md5StateT;
		md5_init(&md5StateT);
		WCHAR buffer[INPUT_BUFFER_SIZE];
		while (!feof(file))
		{
			size_t numberOfObjects = fread(buffer, sizeof(WCHAR), INPUT_BUFFER_SIZE, file);
			md5_append(&md5StateT, (UCHAR*)buffer, numberOfObjects);
		}
		md5_byte_t digest[16];
		md5_finish(&md5StateT, digest);
		WCHAR md5String[33] = { L'\0' }, hexBuffer[3];
		for (size_t i = 0; i != 16; ++i)
		{
			if (digest[i] < 16)
				swprintf_s(hexBuffer, L"0%x", digest[i]);
			else
				swprintf_s(hexBuffer, L"%x", digest[i]);
			wcscat_s(md5String, hexBuffer);
		}
		time_t endTime = time(NULL);
		fclose(file);
		(*result) = md5String;
	}
	return result;
}
std::wstring* MD5Utils::GetStringMD5(LPCWSTR filePath)
{
	return new std::wstring();
}
