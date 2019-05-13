#pragma once

#ifdef _DEBUG
#define PART_MD5_UI L"eb0458ca81710d0802afac75b74d8579"
#define PART_MD5_HOOKS L"5f6c9f775da4f3a79b6373df85f06121"
#define PART_MD5_DRIVER L"c537cee49a6e4faad5ae2153a79b7190"
#define PART_MD5_UPDATER L"55ceae816f59df4248e2b847136454bf"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#else
#define PART_MD5_UI L"a483ca8ba6397a8e75d250476cf25341"
#define PART_MD5_HOOKS L"df5cf3b5f4934146e5e0ae50090e12fb"
#define PART_MD5_DRIVER L"bda75f7ab3b23770bf66da7c9d024289"
#define PART_MD5_UPDATER L"0dd562170d151a95b3cb6dfadaf5c472"
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