#pragma once

#ifdef _DEBUG
#define PART_MD5_UI L"eb0458ca81710d0802afac75b74d8579"
#define PART_MD5_HOOKS L"5f6c9f775da4f3a79b6373df85f06121"
#define PART_MD5_DRIVER L"c537cee49a6e4faad5ae2153a79b7190"
#define PART_MD5_UPDATER L"55ceae816f59df4248e2b847136454bf"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#else
#define PART_MD5_UI L"47bdfee49c000640396954e919495707"
#define PART_MD5_HOOKS L"3335c32d59f731bbc17edd377d6e258a"
#define PART_MD5_DRIVER L"e40033c6b81ebe18dc6b70ec43cb9921"
#define PART_MD5_UPDATER L"288b0ab5840146e5e52be021186ccc3d"
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