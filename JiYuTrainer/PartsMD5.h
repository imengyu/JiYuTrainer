#pragma once

#ifdef _DEBUG
#define PART_MD5_UI L"4092161afeec0e2f7af799f83cce8bcf"
#define PART_MD5_HOOKS L"6c517ddd2136aa15c6b5259d453f4db2"
#define PART_MD5_DRIVER L"c537cee49a6e4faad5ae2153a79b7190"
#define PART_MD5_UPDATER L"fae9a867ece7c34e1c1c319e849e563a"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#else
#define PART_MD5_UI L"df9e01c0c4b37ed8a3d350bf4d1e96b6"
#define PART_MD5_HOOKS L"eb505f3d422ab3ca95ad529000b3edd1"
#define PART_MD5_DRIVER L"bda75f7ab3b23770bf66da7c9d024289"
#define PART_MD5_UPDATER L"332d51dd2a3c63ede2576cd16918f3f5"
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