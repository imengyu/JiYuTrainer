#pragma once

#ifdef _DEBUG
#define PART_MD5_UI L"eb0458ca81710d0802afac75b74d8579"
#define PART_MD5_HOOKS L"5f6c9f775da4f3a79b6373df85f06121"
#define PART_MD5_DRIVER L"c537cee49a6e4faad5ae2153a79b7190"
#define PART_MD5_UPDATER L"55ceae816f59df4248e2b847136454bf"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#else
#define PART_MD5_UI L"ec76d2e28b0deaaac6d4fe0f4e74dc36"
#define PART_MD5_HOOKS L"204077065b2d8dee6d80670d659718af"
#define PART_MD5_DRIVER L"e40033c6b81ebe18dc6b70ec43cb9921"
#define PART_MD5_UPDATER L"0c6e3d0b929f5246a1fcbf47ef31df8a"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#endif

/*
#ifdef _DEBUG

IDR_DLL_DRIVER          BIN                     "..\\Debug\\JiYuTrainerDriver.sys"

IDR_DLL_HOOKS           BIN                     "..\\Debug\\JiYuTrainerHooks.dll"

IDR_DLL_UI              BIN                     "..\\Debug\\JiYuTrainerUI.dll"

IDR_DLL_UPDATER         BIN                     "..\\Debug\\JiYuTrainerUpdater.dll"

IDR_DLL_SCITER          BIN                     "..\\Debug\\sciter.dll"

#else

IDR_DLL_DRIVER          BIN                     "..\\Release\\JiYuTrainerDriver.sys"

IDR_DLL_HOOKS           BIN                     "..\\Release\\JiYuTrainerHooks.dll"

IDR_DLL_UI              BIN                     "..\\Release\\JiYuTrainerUI.dll"

IDR_DLL_UPDATER         BIN                     "..\\Release\\JiYuTrainerUpdater.dll"

IDR_DLL_SCITER          BIN                     "..\\Release\\sciter.dll"

#endif
*/