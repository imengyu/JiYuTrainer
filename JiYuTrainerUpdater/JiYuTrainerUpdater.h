#pragma once
#include "stdafx.h"

#define UPDATE_STATUS_NOT_SUPPORT 0
#define UPDATE_STATUS_LATEST 1
#define UPDATE_STATUS_CHECKED 2
#define UPDATE_STATUS_HAS_UPDATE 3
#define UPDATE_STATUS_COULD_NOT_CONNECT 4
#define UPDATE_STATUS_COULD_NOT_CREATE_FILE 5
#define UPDATE_STATUS_DWONLAODING 6
#define UPDATE_STATUS_FINISHED 7

UPEXPORT_CFUNC(BOOL) JUpdater_CheckInternet();
UPEXPORT_CFUNC(BOOL) JUpdater_CheckUpdate(bool byUser);
UPEXPORT_CFUNC(BOOL) JUpdater_GetUpdateNew(LPCSTR buffer, size_t maxCount);
UPEXPORT_CFUNC(LPCWSTR) JUpdater_GetUpdateNewVer();

int CheckServerForUpdateInfo(bool byUser);
BOOL CheckServerForUpdateNew(LPCSTR buffer, size_t maxCount);
BOOL CheckServerForUpdateVer();
bool CheckLastUpdateDate(LPCWSTR iniPath);
DWORD WINAPI UpdateDownloadThread(LPVOID lpThreadParameter);
int UpdateProgressFunc(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded);

typedef void(*UpdateDownloadCallback)(LPCWSTR precent, LPARAM lParam, int status);

UPEXPORT_CFUNC(BOOL) JUpdater_DownLoadUpdateFile(UpdateDownloadCallback callBack, LPARAM lpararm);
UPEXPORT_CFUNC(BOOL) JUpdater_Updatering();
UPEXPORT_CFUNC(BOOL) JUpdater_CancelDownLoadUpdateFile();
UPEXPORT_CFUNC(BOOL) JUpdater_RunInstallion();