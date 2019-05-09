#include "stdafx.h"
#include "StringHlp.h"

std::string & FormatString(std::string & _str, const char * _Format, ...) {
	std::string tmp;

	va_list marker = NULL;
	va_start(marker, _Format);

	size_t num_of_chars = _vscprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

	vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);

	va_end(marker);

	_str = tmp.c_str();
	return _str;
}
std::wstring & FormatString(std::wstring & _str, const wchar_t * _Format, ...) {
	std::wstring tmp;
	va_list marker = NULL;
	va_start(marker, _Format);
	size_t num_of_chars = _vscwprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), _Format, marker);
	va_end(marker);
	_str = tmp.c_str();
	return _str;
}
std::wstring FormatString(const wchar_t *_Format, va_list marker)
{
	std::wstring tmp;
	size_t num_of_chars = _vscwprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), _Format, marker);
	std::wstring  _str = tmp.c_str();
	return _str;
}
std::string FormatString(const char *_Format, va_list marker)
{
	std::string tmp;
	size_t num_of_chars = _vscprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

	vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);
	std::string _str = tmp.c_str();
	return _str;
}
std::wstring FormatString(const wchar_t *_Format, ...)
{
	std::wstring tmp;
	va_list marker = NULL;
	va_start(marker, _Format);
	size_t num_of_chars = _vscwprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), _Format, marker);
	va_end(marker);
	std::wstring  _str = tmp.c_str();
	return _str;
}
std::string FormatString(const char *_Format, ...)
{
	std::string tmp;

	va_list marker = NULL;
	va_start(marker, _Format);

	size_t num_of_chars = _vscprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

	vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);

	va_end(marker);

	std::string _str = tmp.c_str();
	return _str;
}

StringHlp::StringHlp()
{
}
StringHlp::~StringHlp()
{
}

std::string *StringHlp::FormatStringPtr2A(std::string *_str, const char * _Format, ...) {
	std::string tmp;

	va_list marker = NULL;
	va_start(marker, _Format);

	size_t num_of_chars = _vscprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

	vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);

	va_end(marker);

	*_str = tmp.c_str();
	return _str;
}
std::wstring *StringHlp::FormatStringPtr2W(std::wstring *_str, const wchar_t * _Format, ...) {
	std::wstring tmp;
	va_list marker = NULL;
	va_start(marker, _Format);
	size_t num_of_chars = _vscwprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), _Format, marker);
	va_end(marker);
	*_str = tmp.c_str();
	return _str;
}
std::wstring *StringHlp::FormatStringPtrW(const wchar_t *_Format, ...)
{
	std::wstring tmp;
	va_list marker = NULL;
	va_start(marker, _Format);
	size_t num_of_chars = _vscwprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}
	vswprintf_s((wchar_t *)tmp.data(), tmp.capacity(), _Format, marker);
	va_end(marker);
	std::wstring *_str = new std::wstring();
	*_str = tmp.c_str();
	return _str;
}
std::string *StringHlp::FormatStringPtrA(const char *_Format, ...)
{
	std::string tmp;

	va_list marker = NULL;
	va_start(marker, _Format);

	size_t num_of_chars = _vscprintf(_Format, marker);

	if (num_of_chars > tmp.capacity()) {
		tmp.resize(num_of_chars + 1);
	}

	vsprintf_s((char *)tmp.data(), tmp.capacity(), _Format, marker);

	va_end(marker);

	std::string *_str = new std::string();
	*_str = tmp.c_str();
	return _str;
}

#undef FreeStringPtr
void StringHlp::FreeStringPtr(void *ptr)
{
	if (ptr)
		delete ptr;
}

char* StringHlp::UnicodeToAnsi(const wchar_t* szStr)
{
	int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
	if (nLen == 0)
		return NULL;
	char* pResult = new char[nLen];
	WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
	return pResult;
}
char* StringHlp::UnicodeToUtf8(const wchar_t* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char *szUtf8 = (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
	return szUtf8;
}
wchar_t* StringHlp::AnsiToUnicode(const char* szStr)
{
	int nLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, NULL, 0);
	if (nLen == 0)
		return NULL;
	wchar_t* pResult = new wchar_t[nLen];
	MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szStr, -1, pResult, nLen);
	return pResult;
}
wchar_t* StringHlp::Utf8ToUnicode(const char* szU8)
{
	//预转换，得到所需空间的大小;
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL,szU8, (int)strlen(szU8), NULL, 0);
	//分配空间要给'\0'留个空间，MultiByteToWideChar不会给'\0'空间
	wchar_t* wszString = new wchar_t[wcsLen + 1];
	//转换
	::MultiByteToWideChar(CP_UTF8, NULL, szU8, (int)strlen(szU8), wszString, wcsLen);
	//最后加上'\0'
	wszString[wcsLen] = '\0';
	return wszString;
}

BOOLEAN StrEmepty(LPCWSTR str)
{
	return !str || StringHlp::StrEqualW(str, L"");
}
BOOLEAN StrEmeptyAnsi(LPCSTR str)
{
	return !str || StringHlp::StrEqualA(str, "");
}
LPCWSTR StringHlp::StrUpW(LPCWSTR str)
{
	size_t len = wcslen(str) + 1;
	_wcsupr_s((wchar_t *)str, len);
	return str;
}
LPCSTR StringHlp::StrUpA(LPCSTR str)
{
	size_t len = strlen(str) + 1;
	_strupr_s((char*)str, len);
	return str;
}

LPCWSTR StringHlp::StrLoW(LPCWSTR str)
{
	size_t len = wcslen(str) + 1;
	_wcslwr_s((wchar_t *)str, len);
	return str;
}
LPCSTR StringHlp::StrLoA(LPCSTR str)
{
	size_t len = strlen(str) + 1;
	_strlwr_s((char*)str, len);
	return str;
}

BOOL StringHlp::StrEqualA(LPCSTR str1, LPCSTR str2)
{
	return (strcmp(str1, str2) == 0);
}
BOOL StringHlp::StrEqualW(const wchar_t* str1, const wchar_t* str2)
{
	return (wcscmp(str1, str2) == 0);
}

LPCSTR StringHlp::IntToStrA(int i)
{
	int n = 1, i2 = i;
	if (i == 0)
		n = 2;
	else
	{
		while (i2)
		{
			i2 = i2 / 10;
			n++;
		}
		if (i < 0)
			n++;
	}

	char *rs = new char[n];
	_itoa_s(i, rs, n, 10);
	return rs;
}
LPCWSTR StringHlp::IntToStrW(int i)
{
	int n = 1, i2 = i;
	if (i == 0)
		n = 2;
	else
	{
		while (i2)
		{
			i2 = i2 / 10;
			n++;
		}
		if (i < 0)
			n++;
	}

	WCHAR *rs = new WCHAR[n];
	_itow_s(i, rs, n, 10);
	return rs;
}

LPCSTR StringHlp::LongToStrA(long i)
{
	long n = 1, i2 = i;
	if (i == 0)
		n = 2;
	else
	{
		while (i2)
		{
			i2 = i2 / 10;
			n++;
		}
		if (i < 0)
			n++;
	}

	char *rs = new char[n];
	_ltoa_s(i, rs, n, 10);
	return rs;
}
LPCWSTR StringHlp::LongToStrW(long i)
{
	long n = 1, i2 = i;
	if (i == 0)
		n = 2;
	else
	{
		while (i2)
		{
			i2 = i2 / 10;
			n++;
		}
		if (i < 0)
			n++;
	}

	wchar_t *rs = new wchar_t[n];
	_ltow_s(i, rs, n, 10);
	return rs;
}

int StringHlp::StrToIntA(char* str)
{
	return atoi(str);
}
int StringHlp::StrToIntW(LPCWSTR str)
{
	return _wtoi(str);
}

DWORD StringHlp::StrSplitA(char* str, LPCSTR splitStr, LPCSTR * result, char** lead)
{
	if (str)
	{
		char*p = strtok_s(str, splitStr, lead);
		if (p) {
			*result = p;
			return 1;
		}
		else return 0;
	}
	return 0;
}
DWORD StringHlp::StrSplitW(wchar_t * str, LPCWSTR splitStr, LPCWSTR * result, wchar_t ** lead)
{
	if (str)
	{
		wchar_t*p = wcstok_s(str, splitStr, lead);
		if (p) {
			*result = p;
			return 1;
		}
		else return 0;
	}
	return 0;
}

BOOL StringHlp::StrContainsA(LPCSTR str, LPCSTR testStr, LPCSTR *resultStr)
{
	BOOL result = FALSE;
	const char *rs = strstr(str, testStr);
	if (rs) {
		result = TRUE;
		if (resultStr)*resultStr = rs;
	}
	return result;
}
BOOL StringHlp::StrContainsW(LPCWSTR str, LPCWSTR testStr, LPCWSTR *resultStr)
{
	BOOL result = FALSE;
	const wchar_t *rs = wcsstr(str, testStr);
	if (rs) {
		result = TRUE;
		if (resultStr) *resultStr = (LPWSTR)rs;
	}
	return result;
}

BOOL StringHlp::StrContainsCharA(LPCSTR str, CHAR testStr)
{
	return strchr(str, testStr) != NULL;
}
BOOL StringHlp::StrContainsCharW(LPCWSTR str, WCHAR testStr)
{
	return wcsrchr(str, testStr) != NULL;
}

int StringHlp::HexStrToIntW(wchar_t *s)
{
	size_t i, m = lstrlen(s);
	int temp = 0, n;
	for (i = 0; i<m; i++) {
		if (s[i] >= L'A'&&s[i] <= L'F')
			n = s[i] - L'A' + 10;
		else if (s[i] >= L'a'&&s[i] <= L'f')
			n = s[i] - L'a' + 10;
		else n = s[i] - L'0';
		temp = temp * 16 + n;
	}
	return temp;
}
long long StringHlp::HexStrToLongW(wchar_t *s)
{
	bool isx = false;
	int len = lstrlen(s);
	for (int i = 0; i<len; i++)
	{
		if (s[i] == 'x' || s[i] == 'X') {
			isx = true;
			break;
		}
	}
	int i, m = isx ? lstrlen(s) - 2 : lstrlen(s), n, w = m;
	long long temp = 0;
	for (i = isx ? 2 : 0; i < m; i++) {
		if (s[i] >= L'A'&&s[i] <= L'F')
			n = s[i] - L'A' + 10;
		else if (s[i] >= L'a'&&s[i] <= L'f')
			n = s[i] - L'a' + 10;
		else n = s[i] - L'0';
		w--;
		temp += static_cast<long long>(pow(16, w) * n);
	}
	return temp;
}