#pragma once

#ifdef _DEBUG
#define PART_MD5_UI L"87e0776e9ae7d94b1c5d09246d4be094"
#define PART_MD5_HOOKS L"efb28edc26a5651834ec88ae32c9273d"
#define PART_MD5_DRIVER L"c537cee49a6e4faad5ae2153a79b7190"
#define PART_MD5_UPDATER L"e382f48dc9738ccb4e6880e0c39d59dc"
#define PART_MD5_SCITER L"4efd9dd2bf27ac03fe456077d6c5b261"
#else
#define PART_MD5_UI L"28cd9beea90e4d0173d43e6d25fed326"
#define PART_MD5_HOOKS L"7cce29bbc12cbd1f0124099a4b417391"
#define PART_MD5_DRIVER L"e40033c6b81ebe18dc6b70ec43cb9921"
#define PART_MD5_UPDATER L"83c67ad9840f14198c343d2589fbde7b"
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