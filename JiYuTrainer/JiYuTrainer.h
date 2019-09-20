#pragma once

EXPORT_CFUNC(LONG) WINAPI AppUnhandledExceptionFilter(PEXCEPTION_POINTERS pExInfo);
EXPORT_CFUNC(VOID) ShowAppStartupFail(int rs);
EXPORT_CFUNC(void*) GetApp();
EXPORT_CFUNC(void) ForceExit();