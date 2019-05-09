// JiYuTrainerUpdater.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "JiYuTrainerUpdater.h"
#include "../JiYuTrainer/App.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "../JiYuTrainer/PathHelper.h"
#include "../JiYuTrainer/StringHlp.h"
#include "../JiYuTrainer/SysHlp.h"
#include <time.h>
#include <Wininet.h>
#include "NetUtils.h"

using namespace std;

//#define CURRENT_VERSION "1.6.509.1031" 
#define CURRENT_VERSION "1.3.0506.9063" 
//#define UPDATE_HOST "http://update.imyzc.com/JiYuTrainer/" 
#define UPDATE_HOST "http://localhost/JiYuTrainer/" 

HINSTANCE hInst;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		hInst = hModule;
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

JTApp* appCurrent = nullptr;

UPEXPORT_CFUNC(BOOL) JUpdater_CheckInternet()
{
	return InternetGetConnectedState(NULL, 0);
}
UPEXPORT_CFUNC(BOOL) JUpdater_CheckUpdate(bool byUser)
{
	appCurrent = (JTApp*)JTGetCurrentApp();

	if (!byUser && CheckLastUpdateDate(appCurrent->GetPartFullPath(PART_INI)))
		return UPDATE_STATUS_CHECKED;

	return CheckServerForUpdateInfo(byUser);
}

int CheckServerForUpdateInfo(bool byUser) {
	// test get requery
	string getUrlStr = string(UPDATE_HOST) + string("?checkupdate=") + CURRENT_VERSION;
	string getResponseStr;
	auto res = curl_get_req(getUrlStr, getResponseStr);
	if (res != CURLE_OK) {
		JTLogError(L"检测更新错误：curl_easy_perform() failed:  %S", curl_easy_strerror(res));
		return UPDATE_STATUS_COULD_NOT_CONNECT;
	}
	else {
		if (getResponseStr == "newupdate")
			return UPDATE_STATUS_HAS_UPDATE;
		else if (getResponseStr == "latest") {
			return UPDATE_STATUS_LATEST;
		}
		else {
			JTLogError(L"检测更新错误：update service return bad result :  %S", getResponseStr.c_str());
			return UPDATE_STATUS_NOT_SUPPORT;
		}
	}
	return false;
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
UpdateDownloadCallback callBack = nullptr;
LPARAM callBackLparam = NULL;
WCHAR updateFilePath[MAX_PATH];

UPEXPORT_CFUNC(BOOL) JUpdater_DownLoadUpdateFile(UpdateDownloadCallback callBack, LPARAM lpararm)
{
	callBack = callBack;
	callBackLparam = lpararm;

	wcsncpy_s(updateFilePath, appCurrent->GetCurrentDir(), MAX_PATH);
	wcsncat_s(updateFilePath, L"\\JiYuTrainerNew.exe", MAX_PATH);

	// test get requery
	string getUrlStr = string(UPDATE_HOST) + string("?getupdate");
	string getResponseStr;
	auto res = curl_get_req(getUrlStr, getResponseStr);
	if (res != CURLE_OK) {
		JTLogError(L"获取更新错误 : curl_easy_perform() failed:  %S", curl_easy_strerror(res));
		MessageBox(0, L"获取更新错误\n具体错误请参照输出日志", L"更新时发生错误", MB_ICONERROR);
	}
	else if (getResponseStr != "")
	{
		string downloadUrl = string(UPDATE_HOST)  + getResponseStr;
		updateThread = CreateThread(NULL, NULL, UpdateDownloadThread, (LPVOID)downloadUrl.c_str(), NULL, NULL);
		return true;
	}
	else {
		JTLogError(L"获取更新错误，空返回值");
		MessageBox(0, L"获取更新错误", L"更新时发生错误", MB_ICONERROR);
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

		if (Path::Exists(updateFilePath))
			DeleteFileW(updateFilePath);

		return TRUE;
	}
	return FALSE;
}

UPEXPORT_CFUNC(BOOL) JUpdater_RunInstallion()
{
	if (Path::Exists(updateFilePath)) {
		if (!SysHlp::RunApplicationPriviledge(L"", L"-install-full") && GetLastError() == ERROR_CANCELLED)
			MessageBox(nullptr, L"您取消了更新", L"", MB_ICONEXCLAMATION);
	}
	return FALSE;
}

DWORD WINAPI UpdateDownloadThread(LPVOID lpThreadParameter)
{
	updateing = true;
	Sleep(2000);
	// init curl
	CURL *curl = curl_easy_init();
	// res code
	CURLcode res;
	if (curl)
	{
		if (Path::Exists(updateFilePath))
			DeleteFileW(updateFilePath);

		FILE *file_param = NULL;
		errno_t err = _wfopen_s(&file_param, updateFilePath, L"wb");
		if (!file_param) {
			JTLogError(L"创建更新文件错误 : fopen:  %d", err);
			
			callBack(0, callBackLparam, UPDATE_STATUS_COULD_NOT_CREATE_FILE);

			updateing = false;
			updateThread = NULL;
			return 0;
		}
		curl_easy_setopt(curl, CURLOPT_URL, (LPCSTR)lpThreadParameter); // url
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false); // if want to use https
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false); // set peer and host verify false
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_HEADER, 0);
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3); // set transport and time out time
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, file_param);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, FALSE);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, UpdateProgressFunc);
		//curl_easy_setopt(curl, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)1638400);
		// start req
		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			JTLogError(L"下载更新文件错误 : curl_easy_perform() failed :  %S", curl_easy_strerror(res));

			callBack(0, callBackLparam, UPDATE_STATUS_COULD_NOT_CONNECT);

			updateing = false;
			fclose(file_param);
			updateThread = NULL;
			return 0;
		}

		fclose(file_param);
	}
	else {
		JTLogError(L"下载更新文件错误 : curl failed ");
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
	if (fractiondownloaded >= 1)
		wcscpy_s(updateProgressPrect, L"100%");
	else
		swprintf_s(updateProgressPrect, L"%3.0f%%", fractiondownloaded * 100);

	callBack(updateProgressPrect, callBackLparam, UPDATE_STATUS_DWONLAODING);

	return 0;
}
