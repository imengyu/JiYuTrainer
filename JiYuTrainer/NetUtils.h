#pragma once
#include "stdafx.h"
#include <winsock2.h>
#include <windef.h>   
#include <Nb30.h>
#include <string>

class NetUtils
{
public:
    static std::wstring GetCanonname();
    static std::wstring GetIP(int family = AF_UNSPEC);
    static std::wstring GetIPForHostName(LPCWSTR hostName);
    static bool GetHostNameByIP(LPCWSTR ipAddr, std::wstring* name);
    static bool GetMacByIP(LPCWSTR ipAddr, ULONG srcIp, unsigned char* mac);
    static std::string IPToStringA(sockaddr_in* addr);
    static std::wstring IPToStringW(sockaddr_in* addr);
};

