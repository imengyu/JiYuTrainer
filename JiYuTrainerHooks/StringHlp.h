#pragma once
#include "stdafx.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <string>  
#include <vector>

//字符串帮助类
class StringHlp
{
public:
	StringHlp();
	~StringHlp();

	static std::string & FormatString(std::string & _str, const char * _Format, ...);
	static std::wstring & FormatString(std::wstring & _str, const wchar_t * _Format, ...);
	static std::wstring FormatString(const wchar_t * format, ...);
	static std::wstring FormatString(const wchar_t *_Format, va_list marker);
	static std::string FormatString(const char *_Format, va_list marker);
	static std::string FormatString(const char * format, ...);

	static wchar_t* Utf8ToUnicode(const char* szU8);
	static char* UnicodeToAnsi(const wchar_t* szStr);
	static char* UnicodeToUtf8(const wchar_t* unicode);
	static wchar_t* AnsiToUnicode(const char* szStr);
	static void FreeStringPtr(void * ptr);

	static std::string* FormatStringPtr2A(std::string *_str, const char * _Format, ...);
	static std::wstring * FormatStringPtr2W(std::wstring *_str, const wchar_t * _Format, ...);
	static std::wstring * FormatStringPtrW(const wchar_t *format, ...);
	static std::string *FormatStringPtrA(const char *format, ...);

	//字符串是否是空的
	static BOOLEAN StrEmeptyW(LPCWSTR str);
	//字符串是否是空的
	static BOOLEAN StrEmeptyA(LPCSTR str);

	static LPCWSTR StrUpW(LPCWSTR str);
	static LPCSTR StrUpA(LPCSTR str);
	static LPCWSTR StrLoW(LPCWSTR str);
	static LPCSTR StrLoA(LPCSTR str);
	static LPCWSTR StrAddW(LPCWSTR str1, LPCWSTR str2);
	static LPCSTR StrAddA(LPCSTR str1, const LPCSTR str2);
	//字符串是否相等
	static BOOL StrEqualA(LPCSTR str1, LPCSTR str2);
	//字符串是否相等
	static BOOL StrEqualW(const wchar_t* str1, const wchar_t* str2);
	static LPCSTR IntToStrA(int i);
	static LPCWSTR IntToStrW(int i);
	static LPCWSTR MIntToStrW(int i);
	static LPCSTR LongToStrA(long i);
	static LPCWSTR LongToStrW(long i);
	static int StrToIntA(char * str);
	static int StrToIntW(LPCWSTR str);
	static DWORD StrSplitA(char * str, LPCSTR splitStr, LPCSTR * result, char ** lead);
	static DWORD StrSplitW(wchar_t * str, const LPCWSTR splitStr, LPCWSTR * result, wchar_t ** lead);
	static BOOL StrContainsA(LPCSTR str, LPCSTR testStr, LPCSTR * resultStr);
	static BOOL StrContainsW(LPCWSTR str, LPCWSTR testStr, LPCWSTR * resultStr);
	static BOOL StrContainsCharA(LPCSTR str, CHAR testStr);
	static BOOL StrContainsCharW(LPCWSTR str, WCHAR testStr);
	static int HexStrToIntW(wchar_t *s);
	static long long HexStrToLongW(wchar_t *s);
private:

};

//格式化字符串（仅在当前模块或/MD运行库编译时使用），其他模块请使用 FormatStringPtr
#define FormatString StringHlp::FormatString
//字符串是否含有某个字符
#define StrContainsChar StringHlp::StrContainsCharW
//检测字符串是否是空的
#define StrEmepty StringHlp::StrEmeptyW
//检测字符串是否是空的
#define StrEmeptyAnsi StringHlp::StrEmeptyA
//字符串是否相等
#define StrEqual StringHlp::StrEqualW
//字符串是否相等A
#define StrEqualAnsi StringHlp::StrEqualA
//窄字符转为宽字符
#define A2W StringHlp::AnsiToUnicode
//宽字符转为窄字符
#define W2A StringHlp::UnicodeToAnsi
//格式化字符串，返回的字符串指针请使用 FreeStringPtr 释放
#define FormatStringPtr StringHlp::FormatStringPtrW
//释放由 StringHlp 类返回的字符串指针
#define FreeStringPtr(szPtr) StringHlp::FreeStringPtr((void*)szPtr)






