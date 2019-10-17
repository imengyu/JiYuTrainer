#pragma once
#include "stdafx.h"

BOOL MRegDeleteKey(HKEY hRootKey, LPWSTR path);
BOOL MRegForceDeleteServiceRegkey(LPWSTR lpszDriverName);
BOOL MRegReadKeyString64And32(HKEY hRootKey, LPCWSTR path32, LPCWSTR path64, LPCWSTR keyName, LPWSTR buffer, SIZE_T count);
BOOL MRegReadKeyString(HKEY hRootKey, LPCWSTR path, LPCWSTR keyName, LPWSTR buffer, SIZE_T count);
BOOL MRegCheckUninstallItemExists(LPCWSTR keyName);
