#pragma once

EXPORT_CFUNC(void*) JTGetCurrentApp();
EXPORT_CFUNC(void) JTForceExit();
EXPORT_CFUNC(void) JTLog(const wchar_t * str, ...);
EXPORT_CFUNC(void) JTLogInfo(const wchar_t * str, ...);
EXPORT_CFUNC(void) JTLogWarn(const wchar_t *str, ...);
EXPORT_CFUNC(void) JTLogError(const wchar_t *str, ...);