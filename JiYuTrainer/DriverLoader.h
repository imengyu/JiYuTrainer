#pragma once
#include "stdafx.h"

//加载驱动
//    lpszDriverName：驱动的服务名
//    driverPath：驱动的完整路径
//    lpszDisplayName：nullptr
EXPORT_CFUNC(BOOL) LoadKernelDriver(const wchar_t* lpszDriverName, const wchar_t* driverPath, const wchar_t* lpszDisplayName);
//卸载驱动
//    szSvrName：服务名
EXPORT_CFUNC(BOOL) UnLoadKernelDriver(const wchar_t* szSvrName);
//打开驱动
EXPORT_CFUNC(BOOL) OpenDriver();
//返回驱动是否加载
EXPORT_CFUNC(BOOL) DriverLoaded();

EXPORT_CFUNC(BOOL) XLoadDriver();

EXPORT_CFUNC(BOOL) XUnLoadDriver();
