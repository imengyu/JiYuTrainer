// JiYuTrainerUpdater.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuTrainerUpdater.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "../JiYuTrainer/StringHlp.h"
#include "../JiYuTrainer/PathHelper.h"
#include "../JiYuTrainer/SysHlp.h"
#include <time.h>
#include <Wininet.h>
#include "NetUtils.h"

using namespace std;

#define UPDATE_HOST "http://imengyu.top/services/update/JiYuTrainer/" 
//#define UPDATE_HOST "http://localhost/JiYuTrainer/" 

JTApp* appCurrent = nullptr;
Logger * appLogger = nullptr;

WCHAR newVer[64];

UPEXPORT_CFUNC(BOOL) JUpdater_CheckInternet()
{
	return InternetGetConnectedState(NULL, 0);
}
UPEXPORT_CFUNC(BOOL) JUpdater_CheckUpdate(bool byUser)
{
	appCurrent = (JTApp*)GetApp();
	appLogger = appCurrent->GetLogger();

	if (!byUser && CheckLastUpdateDate(appCurrent->GetPartFullPath(PART_INI)))
		return UPDATE_STATUS_CHECKED;

	return CheckServerForUpdateInfo(byUser);
}
UPEXPORT_CFUNC(BOOL) JUpdater_GetUpdateNew(LPCSTR buffer, size_t maxCount) {
	return CheckServerForUpdateNew(buffer, maxCount);
}
UPEXPORT_CFUNC(LPCWSTR) JUpdater_GetUpdateNewVer() {
	CheckServerForUpdateVer();
	return newVer;
}

int CheckServerForUpdateInfo(bool byUser) {
	// test get requery
	string getUrlStr = string(UPDATE_HOST) + string("?checkupdate=") + CURRENT_VERSION;
	string getResponseStr;
	auto res = curl_get_req(getUrlStr, getResponseStr);
	if (res != CURLE_OK) {
		appLogger->LogError(L"检测更新错误：curl_easy_perform() failed:  %S", curl_easy_strerror(res));
		return UPDATE_STATUS_COULD_NOT_CONNECT;
	}
	else {
		if (getResponseStr == "newupdate")
			return UPDATE_STATUS_HAS_UPDATE;
		else if (getResponseStr == "latest") {
			return UPDATE_STATUS_LATEST;
		}
		else {
			appLogger->LogError(L"检测更新错误：update service return bad result :  %S", getResponseStr.c_str());
			return UPDATE_STATUS_NOT_SUPPORT;
		}
	}
	return false;
}
BOOL CheckServerForUpdateNew(LPCSTR buffer, size_t maxCount) {
	// test get requery
	string getUrlStr = string(UPDATE_HOST) + string("?getupdateinfo");
	string getResponseStr;
	auto res = curl_get_req(getUrlStr, getResponseStr);
	if (res != CURLE_OK) {
		appLogger->LogError(L"获取更新内容错误：curl_easy_perform() failed:  %S", curl_easy_strerror(res));
		return FALSE;
	}
	else {
		strncpy_s((char*)buffer, maxCount, getResponseStr.c_str(), maxCount);
		return TRUE;
	}
}
BOOL CheckServerForUpdateVer() {
	string getUrlStr = string(UPDATE_HOST) + string("?getnewver");
	string getResponseStr;
	auto res = curl_get_req(getUrlStr, getResponseStr);
	if (res != CURLE_OK) {
		appLogger->LogError(L"获取更新内容错误：curl_easy_perform() failed:  %S", curl_easy_strerror(res));
		return FALSE;
	}
	else {
		LPCWSTR getResponseStrW = StringHlp::AnsiToUnicode(getResponseStr.c_str());
		wcscpy_s(newVer, 64, getResponseStrW);
		FreeStringPtr(getResponseStrW);
		return TRUE;
	}
}
bool CheckLastUpdateDate(LPCWSTR iniPath) {

	if (Path::Exists(iniPath))
	{
		WCHAR w[32];
		WCHAR w2[32];
		WCHAR wnow[32];
		GetPrivateProfileString(L"JTSettings", L"LastCheckUpdateTime", L"00/00", w, 32, iniPath);
		GetPrivateProfileString(L"JTSettings", L"AlwaysCheckUpdate", L"FALSE", w2, 32, iniPath);

		time_t tt = time(NULL);//这句返回的只是一个时间cuo
		tm t;
		localtime_s(&t, &tt);
		swprintf_s(wnow, L"%00d/%00d", t.tm_mon + 1, t.tm_mday);

		if (StrEqual(w, wnow)) {
			if (StrEqual(w2, L"TRUE")) return false;
			return true;
		}
		else WritePrivateProfileString(L"JTSettings", L"LastCheckUpdateTime", wnow, iniPath);
	}
	return false;
}

HANDLE updateThread;
bool updateing = false;
bool updateCanceled = false;
UpdateDownloadCallback callBack = nullptr;
LPARAM callBackLparam = NULL;
WCHAR updateFilePath[MAX_PATH];
string downloadFileUrl;
FILE *updateFile = NULL;

UPEXPORT_CFUNC(BOOL) JUpdater_DownLoadUpdateFile(UpdateDownloadCallback callBack, LPARAM lpararm)
{
	::callBack = callBack;
	callBackLparam = lpararm;

	updateCanceled = false;

	wcsncpy_s(updateFilePath, appCurrent->GetCurrentDir(), MAX_PATH);
	wcsncat_s(updateFilePath, L"\\JiYuTrainerUpdater.exe", MAX_PATH);

	// test get requery
	string getUrlStr = string(UPDATE_HOST) + string("?getupdate");
	string getResponseStr;
	auto res = curl_get_req(getUrlStr, getResponseStr);
	if (res != CURLE_OK) {
		appLogger->LogError(L"获取更新错误 : curl_easy_perform() failed:  %S", curl_easy_strerror(res));
		callBack(0, callBackLparam, UPDATE_STATUS_COULD_NOT_CONNECT);
	}
	else if (getResponseStr != "")
	{
		downloadFileUrl = string(UPDATE_HOST)  + getResponseStr;
		updateThread = CreateThread(NULL, NULL, UpdateDownloadThread, NULL, NULL, NULL);
		return true;
	}
	else {
		appLogger->LogError(L"获取更新错误，空返回值");
		callBack(0, callBackLparam, UPDATE_STATUS_COULD_NOT_CONNECT);
	}

	return false;
}
UPEXPORT_CFUNC(BOOL) JUpdater_Updatering()
{
	return updateing;
}
UPEXPORT_CFUNC(BOOL) JUpdater_CancelDownLoadUpdateFile()
{
	if (updateing) {

		if (updateThread) {
			TerminateThread(updateThread, 0);
			updateThread = NULL;
		}

		updateCanceled = true;

		if (updateFile) {
			fclose(updateFile);
			updateFile = NULL;
		}
		if (Path::Exists(updateFilePath))
			DeleteFileW(updateFilePath);

		return TRUE;
	}
	return FALSE;
}
UPEXPORT_CFUNC(BOOL) JUpdater_RunInstallion()
{
	if (Path::Exists(updateFilePath)) {
		//强制卸载已注入的病毒
		if(appCurrent->GetTrainerWorker())
			appCurrent->GetTrainerWorker()->RunOperation(TrainerWorkerOpForceUnLoadVirus);
		//运行更新
		if (SysHlp::RunApplicationPriviledge(updateFilePath, appCurrent->MakeFromSourceArg(L"-install-full")) && GetLastError() == ERROR_CANCELLED) {
			MessageBox(nullptr, L"您取消了更新", L"", MB_ICONEXCLAMATION);
			return FALSE;
		}else return TRUE;
	}
	return FALSE;
}

DWORD WINAPI UpdateDownloadThread(LPVOID lpThreadParameter)
{
	LPCSTR url = downloadFileUrl.c_str();

	updateing = true;
	Sleep(1300);
	// init curl
	CURL *curl = curl_easy_init();
	// res code
	CURLcode res;
	if (curl)
	{
		if (Path::Exists(updateFilePath))
			DeleteFileW(updateFilePath);


		errno_t err = _wfopen_s(&updateFile, updateFilePath, L"wb");
		if (!updateFile) {
			appLogger->LogError(L"创建更新文件错误 : fopen:  %d", err);
			
			callBack(0, callBackLparam, UPDATE_STATUS_COULD_NOT_CREATE_FILE);

			updateing = false;
			updateThread = NULL;
			return 0;
		}

		curl_easy_setopt(curl, CURLOPT_URL, url); // url
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); // set transport and time out time
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, updateFile);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, UpdateProgressFunc);
		//curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)1638400);
		// start req
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			appLogger->LogError(L"下载更新文件错误 %s 失败 :  %S", url, curl_easy_strerror(res));

			callBack(0, callBackLparam, UPDATE_STATUS_COULD_NOT_CONNECT);

			updateing = false;
			fclose(updateFile);
			updateThread = NULL;
			return 0;
		}

		fclose(updateFile);
	}
	else {
		appLogger->LogError(L"下载更新文件错误 : curl failed ");
		callBack(0, callBackLparam, UPDATE_STATUS_NOT_SUPPORT);
		updateing = false;
	}

	// release curl
	curl_easy_cleanup(curl);

	callBack(0, callBackLparam, UPDATE_STATUS_FINISHED);

	updateing = false;
	updateThread = NULL;

	return 0;
}

int UpdateProgressFunc(void* ptr, double TotalToDownload, double NowDownloaded, double TotalToUpload, double NowUploaded)
{
	wchar_t updateProgressPrect[10];
	double fractiondownloaded = NowDownloaded / TotalToDownload;
	if (_isnan(fractiondownloaded))
		return 0;

	if (fractiondownloaded >= 1)
		wcscpy_s(updateProgressPrect, L"100%");
	else
		swprintf_s(updateProgressPrect, L"%3.0f%%", fractiondownloaded * 100);

	callBack(updateProgressPrect, callBackLparam, UPDATE_STATUS_DWONLAODING);

	return 0;
}
