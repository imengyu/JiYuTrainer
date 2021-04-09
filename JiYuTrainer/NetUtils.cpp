#include "stdafx.h"
#include "NetUtils.h"
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <Mstcpip.h>
#include <Ip2string.h>

WCHAR canonname[32];

std::wstring NetUtils::GetCanonname() {
	return std::wstring(canonname);
}
std::wstring NetUtils::GetIP(int family)
{
	WSADATA wsaData;
	std::wstring strResult;

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) == 0)
	{

		ADDRINFOW* result = nullptr;
		ADDRINFOW* ptr = nullptr;
		ADDRINFOW hints = { 0 };
		hints.ai_flags = AI_CANONNAME;
		hints.ai_family = family;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		if (GetAddrInfoW(L"", NULL, &hints, &result) == 0)
		{
			for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

				switch (ptr->ai_family) {
				case AF_INET:
				case AF_INET6:
					DWORD ipbufferlength = 128;
					WCHAR ipstringbuffer[128];
					if (ptr->ai_canonname != nullptr)
						wcscpy_s(canonname, ptr->ai_canonname);
					if (WSAAddressToStringW(ptr->ai_addr, (DWORD)ptr->ai_addrlen, NULL,
						ipstringbuffer, &ipbufferlength) == 0) {
						strResult.append(ipstringbuffer);
						strResult.append(L"\n");
					}
					break;
				}
			}
		}
		WSACleanup();
	}
	return strResult;
}
std::wstring NetUtils::GetIPForHostName(LPCWSTR hostName)
{
	std::wstring strResult;

	ADDRINFOW* result = nullptr;
	ADDRINFOW* ptr = nullptr;
	ADDRINFOW hints = { 0 };
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (GetAddrInfoW(hostName, NULL, &hints, &result) == 0)
	{
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

			switch (ptr->ai_family) {
			case AF_INET:
			case AF_INET6:
				DWORD ipbufferlength = 128;
				WCHAR ipstringbuffer[128];
				if (ptr->ai_canonname != nullptr)
					wcscpy_s(canonname, ptr->ai_canonname);
				if (WSAAddressToStringW(ptr->ai_addr, (DWORD)ptr->ai_addrlen, NULL,
					ipstringbuffer, &ipbufferlength) == 0) {
					strResult.append(ipstringbuffer);
					strResult.append(L"\n");
				}
				break;
			}
		}
	}
	return strResult;
}

bool NetUtils::GetHostNameByIP(LPCWSTR ipAddr, std::wstring* name)
{
	struct sockaddr sa;
	struct in_addr s;
	InetPton(AF_INET, ipAddr, &s);

	sa.sa_family = AF_INET;
	memcpy_s(sa.sa_data, sizeof(sa.sa_data), &s, sizeof(sa.sa_data));

	WCHAR hostname[NI_MAXHOST];
	WCHAR servInfo[NI_MAXSERV];

	DWORD dwRetval = GetNameInfoW((struct sockaddr*)&sa,
		sizeof(struct sockaddr),
		hostname,
		NI_MAXHOST, servInfo, NI_MAXSERV, NI_NUMERICSERV);

	if (dwRetval == 0) {
		*name = hostname;
		return true;
	} 
	else
		return false;
}
bool NetUtils::GetMacByIP(LPCWSTR ipAddr, ULONG srcIp, unsigned char* mac)
{
	ULONG mac_address[2];
	ULONG mac_address_len = 6;

	struct in_addr s;
	InetPton(AF_INET, ipAddr, &s);

	SendARP((IPAddr)s.s_addr, (IPAddr)srcIp, mac_address, &mac_address_len);

	if (mac_address_len)
	{
		BYTE* mac_address_buffer = (BYTE*)&mac_address;
		for (int i = 0; i < (int)mac_address_len; i++)
			mac[i] = (unsigned char)mac_address_buffer[i];
	}
	else
		return false;
	return true;
}

std::string NetUtils::IPToStringA(sockaddr_in* addr)
{
	char sendBuf[100] = { '\0' };
	inet_ntop(AF_INET, (void*)&addr->sin_addr, sendBuf, 100);
	return std::string(sendBuf);
}
std::wstring NetUtils::IPToStringW(sockaddr_in* addr)
{
	wchar_t sendBuf[100] = { L'\0' };
	InetNtop(AF_INET, (void*)&addr->sin_addr, sendBuf, 100);
	return std::wstring(sendBuf);
}
