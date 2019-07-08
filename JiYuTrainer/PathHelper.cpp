#include "stdafx.h"
#include "PathHelper.h"
#include "StringHlp.h"
#include <shlwapi.h>

#define DirectorySeparatorChar L'\\'
#define AltDirectorySeparatorChar  L'/'
#define VolumeSeparatorChar  L':'

Path::Path()
{
}
Path::~Path()
{
}

bool Path::RemoveQuotes(LPWSTR pathBuffer, size_t bufferSize)
{
	if (pathBuffer[0] == L'\"')
	{
		size_t size = wcslen(pathBuffer);
		if (pathBuffer[size - 1] == L'\"')
		{
			for (size_t i = 1; i < size -1 && i < bufferSize; i++) {
				pathBuffer[i - 1] = pathBuffer[i];
			}
			pathBuffer[size - 1] = L'\0';
			pathBuffer[size - 2] = L'\0';
			return true;
		}
	}	
	return false;
}
bool Path::IsValidateFolderFileName(std::wstring path)
{
	bool ret = true;
	size_t u32Length = 0, u32Index = 0;
	wchar_t u8SpecialChar[] = { '\\','<','>','(',')','[',']','&',':',',','/','|','?','*' };
	wchar_t u8CtrlCharBegin = 0x0, u8CtrlCharEnd = 0x31;

	LPCWSTR pName = (LPCWSTR)path.c_str();
	if (pName == NULL)
		ret = false;
	else
	{
		u32Length = wcslen(pName);
		if (u32Length >= MAX_PATH)
			ret = false;
	}

	for (u32Index = 0; (u32Index < u32Length) && (ret == 0);
		u32Index++)
	{
		if (u8CtrlCharBegin <= pName[u32Index] && pName[u32Index] <= u8CtrlCharEnd)
			ret = false;
		else if (wcschr(u8SpecialChar, pName[u32Index]) != NULL)
			ret = false;
	}
	return ret;
}
bool Path::CheckInvalidPathChars(std::wstring path)
{
	for (size_t i = 0; i < path.size(); i++)
	{
		int num = (int)(path)[i];
		if (num == 34 || num == 60 || num == 62 || num == 124 || num < 32)
		{
			return true;
		}
	}
	return false;
}
std::wstring Path::GetExtension(std::wstring path)
{
	if (StringHlp::StrEmeptyW(path.c_str()))	return std::wstring();
	if(Path::CheckInvalidPathChars(path))	return std::wstring();
	size_t length = path.size();
	size_t num = length;
	while (--num >= 0)
	{
		wchar_t c = (path)[num];
		if (c == L'.')
		{
			if (num != length - 1)
				return std::wstring(path.substr(num, length - num));
			return std::wstring();
		}
		else if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
		{
			break;
		}
	}
	return std::wstring();
}
bool Path::IsPathRooted(std::wstring path)
{
	if (!StringHlp::StrEmeptyW(path.c_str())) 
	{
		if (Path::CheckInvalidPathChars(path)) return false;
		size_t length = path.size();
		if ((length >= 1 && (path[0] == DirectorySeparatorChar || path[0] == AltDirectorySeparatorChar)) || (length >= 2 && path[1] == VolumeSeparatorChar))
		{
			return true;
		}
	}
	return false;
}
bool Path::HasExtension(std::wstring path)
{
	if (!StringHlp::StrEmeptyW(path.c_str()))
	{
		if(Path::CheckInvalidPathChars(path)) 	return false;
		size_t num = path.size();
		while (--num >= 0)
		{
			wchar_t c = (path)[num];
			if (c == L'.')
			{
				return num != path.size() - 1;
			}
			if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
			{
				break;
			}
		}
	}
	return false;
}
std::wstring Path::GetFileNameWithoutExtension(std::wstring path)
{
	path = Path::GetFileName(path);
	if (StringHlp::StrEmeptyW(path.c_str()))
		return std::wstring();
	size_t length;
	if ((length = path.find_last_of(L'.')) == -1)
		return path;
	return  std::wstring(path.substr(0, length));
}
std::wstring Path::GetFileName(std::wstring path)
{
	if (!StringHlp::StrEmeptyW(path.c_str()))
	{
		size_t length = path.size();
		size_t num = length;
		while (--num >= 0)
		{
			wchar_t c = (path)[num];
			if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
				return std::wstring(path.substr(num + 1, length - num - 1));
		}
	}
	return path;
}
std::wstring Path::GetDirectoryName(std::wstring path)
{
	if (!StringHlp::StrEmeptyW(path.c_str())) {
		TCHAR exeFullPath[MAX_PATH];
		wcscpy_s(exeFullPath, path.c_str());
		PathRemoveFileSpec(exeFullPath);
		return std::wstring(exeFullPath);
	}
	return std::wstring();
}

std::wstring Path::GetFileNameWithoutExtension(LPCWSTR path)
{
	return GetFileNameWithoutExtension(std::wstring(path));
}
std::wstring Path::GetExtension(LPCWSTR path)
{
	return GetExtension(std::wstring(path));
}
bool Path::IsPathRooted(LPWSTR path)
{
	return IsPathRooted(std::wstring(path));
}

bool Path::Exists(LPCWSTR path1)
{
	return PathFileExistsW(path1);
}
bool Path::Exists(std::wstring path1)
{
	return Exists(path1.c_str());
}
bool Path::Exists(LPCSTR path1)
{
	return PathFileExistsA(path1);
}
bool Path::Exists(std::string path1)
{
	return Exists(path1.c_str());
}

bool Path::HasExtension(LPCWSTR path)
{
	return HasExtension(std::wstring(path));
}
bool Path::CheckInvalidPathChars(LPCWSTR path)
{
	return CheckInvalidPathChars(std::wstring(path));
}
std::wstring Path::GetFileName(LPCWSTR path)
{
	return GetFileName(std::wstring(path));
}
std::wstring Path::GetDirectoryName(LPCWSTR path)
{
	return GetDirectoryName(std::wstring(path));
}

bool Path::IsValidateFolderFileName(std::string  path)
{
	bool ret = true;
	size_t u32Length = 0, u32Index = 0;
	char u8SpecialChar[] = { '\\','<','>','(',')','[',']','&',':',',','/','|','?','*' };
	char u8CtrlCharBegin = 0x0, u8CtrlCharEnd = 0x31;

	LPCSTR pName = (LPCSTR)path.c_str();
	if (pName == NULL)
		ret = false;
	else
	{
		u32Length = strlen(pName);
		if (u32Length >= MAX_PATH)
			ret = false;
	}

	for (u32Index = 0; (u32Index < u32Length) && (ret == 0);
		u32Index++)
	{
		if (u8CtrlCharBegin <= pName[u32Index] && pName[u32Index] <= u8CtrlCharEnd)
			ret = false;
		else if (strchr(u8SpecialChar, pName[u32Index]) != NULL)
			ret = false;
	}
	return ret;
}
bool Path::CheckInvalidPathChars(std::string path)
{
	for (size_t i = 0; i < path.size(); i++)
	{
		int num = (int)(path)[i];
		if (num == 34 || num == 60 || num == 62 || num == 124 || num < 32)
		{
			return true;
		}
	}
	return false;
}
std::string Path::GetExtension(std::string path)
{
	if (path.empty()) return path;
	if (Path::CheckInvalidPathChars(path)) return std::string();
	size_t length = path.size();
	size_t num = length;
	while (--num >= 0)
	{
		wchar_t c = (path)[num];
		if (c == L'.')
		{
			if (num != length - 1)
			{				
				return std::string(path.substr(num, length - num));
			}
			return std::string();
		}
		else if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
		{
			break;
		}
	}
	return std::string();
}
bool Path::IsPathRooted(std::string path)
{
	if (!path.empty())
	{
		if (Path::CheckInvalidPathChars(path)) return false;
		size_t length = path.size();
		if ((length >= 1 && (path[0] == DirectorySeparatorChar || path[0] == AltDirectorySeparatorChar)) || (length >= 2 && path[1] == VolumeSeparatorChar))
		{
			return true;
		}
	}
	return false;
}
bool Path::HasExtension(std::string path)
{
	if (!path.empty())
	{
		if (Path::CheckInvalidPathChars(path)) 	return false;
		size_t num = path.size();
		while (--num >= 0)
		{
			wchar_t c = (path)[num];
			if (c == L'.')
			{
				return num != path.size() - 1;
			}
			if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
			{
				break;
			}
		}
	}
	return false;
}
std::string Path::GetFileNameWithoutExtension(std::string path)
{
	path = Path::GetFileName(path);
	if (path.empty()) return path;
	size_t length;
	if ((length = path.find_last_of(L'.')) == -1)
	{
		return path;
	}
	return  std::string(path.substr(0, length));
}
std::string Path::GetFileName(std::string path)
{
	if (!path.empty())
	{
		if (Path::CheckInvalidPathChars(path))return NULL;
		size_t length = path.size();
		size_t num = length;
		while (--num >= 0)
		{
			wchar_t c = (path)[num];
			if (c == DirectorySeparatorChar || c == AltDirectorySeparatorChar || c == VolumeSeparatorChar)
			{
				return  std::string(path.substr(num + 1, length - num - 1));
			}
		}
	}
	return path;
}
std::string Path::GetDirectoryName(std::string path)
{
	if (path.empty())
	{
		CHAR exeFullPath[MAX_PATH];
		strcpy_s(exeFullPath, path.c_str());
		PathRemoveFileSpecA(exeFullPath);
		return std::string(exeFullPath);
	}
	return std::string();;
}

std::string Path::GetFileNameWithoutExtension(LPCSTR path)
{
	return GetFileNameWithoutExtension(std::string(path));
}
std::string Path::GetExtension(LPCSTR path)
{
	return GetExtension(std::string(path));
}
bool Path::IsPathRooted(LPCSTR path)
{
	return IsPathRooted(std::string(path));
}
bool Path::HasExtension(LPCSTR path)
{
	return HasExtension(std::string(path));
}
bool Path::CheckInvalidPathChars(LPCSTR path)
{
	return CheckInvalidPathChars(std::string(path));
}
std::string Path::GetFileName(LPCSTR path)
{
	return GetFileName(std::string(path));
}
std::string Path::GetDirectoryName(LPCSTR path)
{
	return GetDirectoryName(std::string(path));
}


