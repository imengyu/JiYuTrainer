#pragma once

#ifdef _DEBUG
#define PART_MD5_UI L"4092161afeec0e2f7af799f83cce8bcf"
#define PART_MD5_HOOKS L"6c517ddd2136aa15c6b5259d453f4db2"
#define PART_MD5_DRIVER L"c537cee49a6e4faad5ae2153a79b7190"
#define PART_MD5_UPDATER L"fae9a867ece7c34e1c1c319e849e563a"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#else
#define PART_MD5_UI L"3abf9849ad7b41097c0d7d671c3114d1"
#define PART_MD5_HOOKS L"15c44e934a31c17ec27263fbf1844983"
#define PART_MD5_DRIVER L"8d8209086f7a5296c72ec81d7d38146e"
#define PART_MD5_UPDATER L"4099e9dca05ba1a5efc7610197b68dd3"
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