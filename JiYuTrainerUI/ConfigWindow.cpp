#include "stdafx.h"
#include "ConfigWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/StringHlp.h"
#include "resource.h"
#include <CommCtrl.h>
#include <windowsx.h>

extern JTApp* currentApp;
extern Logger * currentLogger;

SettingHlp *currentSettings;

VOID ShowMoreSettings(HWND hWndMain)
{
	currentLogger = currentApp->GetLogger();
	currentSettings = currentApp->GetSettings();
	DialogBox(currentApp->GetInstance() , MAKEINTRESOURCE(IDD_SETTINGS), GetDesktopWindow(), SettingsDlgFunc);
}

INT_PTR CALLBACK SettingsDlgFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {
		//IDC_COMBO1
		HWND cbInjectMode = GetDlgItem(hDlg, IDC_COMBO_INJECT_MODE);
		SendMessage(cbInjectMode, CB_ADDSTRING, NULL, (WPARAM)L"RemoteThread");
		SendMessage(cbInjectMode, CB_ADDSTRING, NULL, (WPARAM)L"HookDllStub");
		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_APP)));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_APP)));

		if (SysHlp::Is64BitOS())
		{
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_INI_0), FALSE);
			EnableWindow(GetDlgItem(hDlg, IDC_CHECK_INI_1), FALSE);
			CheckDlgButton(hDlg, IDC_CHECK_INI_0, BST_CHECKED);
		}
		else
		{
			CheckDlgButton(hDlg, IDC_CHECK_INI_0, currentSettings->GetSettingBool(L"DisableDriver", false) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hDlg, IDC_CHECK_INI_1, currentSettings->GetSettingBool(L"SelfProtect", true) ? BST_CHECKED : BST_UNCHECKED);
		}
		CheckDlgButton(hDlg, IDC_CHECK_INI_2, currentSettings->GetSettingBool(L"BandAllRunOp", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_3, currentSettings->GetSettingBool(L"AlwaysCheckUpdate", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_4, currentSettings->GetSettingBool(L"DoNotShowVirusWindow", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_5, currentSettings->GetSettingBool(L"ForceInstallInCurrentDir", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_6, currentSettings->GetSettingBool(L"ForceDisableWatchDog", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_7, currentSettings->GetSettingBool(L"InjectMasterHelper", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_8, currentSettings->GetSettingBool(L"InjectProcHelper64", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_9, currentSettings->GetSettingBool(L"ProhibitKillProcess", true) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_10, currentSettings->GetSettingBool(L"ProhibitCloseWindow", true) ? BST_CHECKED : BST_UNCHECKED);

		std::wstring *setKillProcess = currentSettings->GetSettingStrPtr(L"KillProcess", L"NtTerminateProcess");
		if(StrEqual(setKillProcess->c_str(), L"TerminateProcess"))  Button_SetCheck(GetDlgItem(hDlg, IDC_KILLPROC_TP), BST_CHECKED);
		else if (StrEqual(setKillProcess->c_str(), L"NtTerminateProcess")) Button_SetCheck(GetDlgItem(hDlg, IDC_KILLPROC_NTP), BST_CHECKED);
		else if (StrEqual(setKillProcess->c_str(), L"KernelMode"))  Button_SetCheck(GetDlgItem(hDlg, IDC_KILLPROC_PPTP_APC), BST_CHECKED);
		FreeStringPtr(setKillProcess);

		std::wstring *setCKInterval = currentSettings->GetSettingStrPtr(L"CKInterval", L"3100");
		int ckInterval = _wtoi((*setCKInterval).c_str());
		if (ckInterval < 1000 || ckInterval>10000)
			ckInterval = 3100;
		WCHAR ckIntervalStrBuffer[16];
		swprintf_s(ckIntervalStrBuffer, L"%d", ckInterval);
		SetDlgItemText(hDlg, IDC_CKINTERVAL, ckIntervalStrBuffer);
		FreeStringPtr(setCKInterval);

		std::wstring *setInjectMode = currentSettings->GetSettingStrPtr(L"InjectMode", L"RemoteThread");
		SendMessage(cbInjectMode, CB_SELECTSTRING, -1, (LPARAM)(*setInjectMode).c_str());
		FreeStringPtr(setInjectMode);

		return TRUE;
	}
	case WM_COMMAND: {
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_CHECK_INI_0) {
			if (IsDlgButtonChecked(hDlg, IDC_CHECK_INI_0)) {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_INI_1), FALSE);
				CheckDlgButton(hDlg, IDC_CHECK_INI_1, BST_UNCHECKED);
			}
			else {
				EnableWindow(GetDlgItem(hDlg, IDC_CHECK_INI_1), TRUE);
				CheckDlgButton(hDlg, IDC_CHECK_INI_1, currentSettings->GetSettingBool(L"SelfProtect", true) ? BST_CHECKED : BST_UNCHECKED);
			}
		}
		if (LOWORD(wParam) == IDC_SAVE) {
			if (IsDlgButtonChecked(hDlg, IDC_KILLPROC_TP))
				currentSettings->SetSettingStr(L"KillProcess", L"TerminateProcess");
			else if (IsDlgButtonChecked(hDlg, IDC_KILLPROC_NTP))
				currentSettings->SetSettingStr(L"KillProcess", L"NtTerminateProcess");
			else if (IsDlgButtonChecked(hDlg, IDC_KILLPROC_PPTP_APC))
				currentSettings->SetSettingStr(L"KillProcess", L"KernelMode");

			if (!SysHlp::Is64BitOS()) 
			{
				currentSettings->SetSettingBool(L"DisableDriver", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_0));
				currentSettings->SetSettingBool(L"SelfProtect", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_1));
			}
			currentSettings->SetSettingBool(L"BandAllRunOp", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_2));
			currentSettings->SetSettingBool(L"AlwaysCheckUpdate", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_3));
			currentSettings->SetSettingBool(L"DoNotShowVirusWindow", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_4));
			currentSettings->SetSettingBool(L"ForceInstallInCurrentDir", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_5));
			currentSettings->SetSettingBool(L"ForceDisableWatchDog", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_6));
			currentSettings->SetSettingBool(L"InjectMasterHelper", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_7));
			currentSettings->SetSettingBool(L"InjectProcHelper64", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_8));
			currentSettings->SetSettingBool(L"ProhibitKillProcess", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_9));
			currentSettings->SetSettingBool(L"ProhibitCloseWindow", IsDlgButtonChecked(hDlg, IDC_CHECK_INI_10));

			WCHAR ckIntervalStrBuffer[16];
			GetDlgItemText(hDlg, IDC_CKINTERVAL, ckIntervalStrBuffer, 16);
			int ckInterval = _wtoi(ckIntervalStrBuffer);
			if (ckInterval < 1000 || ckInterval>10000) wcscpy_s(ckIntervalStrBuffer, L"3100");
			currentSettings->SetSettingStr(L"CKInterval", ckIntervalStrBuffer);

			int sel = SendMessage(GetDlgItem(hDlg, IDC_COMBO_INJECT_MODE), CB_GETCURSEL, 0, 0);
			if (sel == 0)currentSettings->SetSettingStr(L"InjectMode", L"RemoteThread");
			else if (sel == 1)currentSettings->SetSettingStr(L"InjectMode", L"HookDllStub");

			currentApp->GetTrainerWorker()->InitSettings();
		}
		return TRUE;
	}
	case WM_CLOSE: {
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	default:
		break;
	}
	return FALSE;
}

