#pragma once
#include "stdafx.h"

//加载驱动
//    lpszDriverName：驱动的服务名
//    driverPath：驱动的完整路径
//    lpszDisplayName：nullptr
BOOL LoadKernelDriver(const wchar_t* lpszDriverName, const wchar_t* driverPath, const wchar_t* lpszDisplayName);
//卸载驱动
//    szSvrName：服务名
BOOL UnLoadKernelDriver(const wchar_t* szSvrName);
//打开驱动
BOOL OpenDriver();
//返回驱动是否加载
BOOL DriverLoaded();

BOOL XinitSelfProtect();

BOOL XLoadDriver();

BOOL XCloseDriverHandle();

BOOL XUnLoadDriver();
