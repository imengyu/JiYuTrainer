#pragma once
#include "stdafx.h"
#include <string>
#include "StringHlp.h"
#include <shlwapi.h>

class EXPORT Path
{
public:
	Path();
	~Path();

	static std::wstring GetFileNameWithoutExtension(std::wstring path);
	static std::wstring GetExtension(std::wstring path);
	static bool IsPathRooted(std::wstring path1);
	static bool HasExtension(std::wstring path);
	static bool CheckInvalidPathChars(std::wstring path);
	static std::wstring GetFileName(std::wstring path);
	static std::wstring GetDirectoryName(std::wstring path);
	static bool IsValidateFolderFileName(std::wstring path);
	static bool RemoveQuotes(LPWSTR pathBuffer, size_t bufferSize);

	static std::wstring GetFileNameWithoutExtension(LPCWSTR path);
	static std::wstring GetExtension(LPCWSTR path);
	static bool IsPathRooted(LPWSTR path1);
	static bool Exists(LPCWSTR path1);
	static bool Exists(std::wstring path1);
	static bool HasExtension(LPCWSTR path);
	static bool CheckInvalidPathChars(LPCWSTR path);
	static std::wstring GetFileName(LPCWSTR path);
	static std::wstring GetDirectoryName(LPCWSTR path);

	static std::string GetFileNameWithoutExtension(std::string path);
	static std::string GetExtension(std::string path);
	static bool IsPathRooted(std::string path1);
	static bool HasExtension(std::string path);
	static bool CheckInvalidPathChars(std::string path);
	static std::string GetFileName(std::string path);
	static std::string GetDirectoryName(std::string path);
	static bool IsValidateFolderFileName(std::string path);

	static std::string GetFileNameWithoutExtension(LPCSTR path);
	static std::string GetExtension(LPCSTR path);
	static bool IsPathRooted(LPCSTR path1);
	static bool HasExtension(LPCSTR path);
	static bool CheckInvalidPathChars(LPCSTR path);
	static std::string GetFileName(LPCSTR path);
	static std::string GetDirectoryName(LPCSTR path);
	static bool Exists(LPCSTR path1);
	static bool Exists(std::string path1);
};