#include "stdafx.h"
#include "ConfigWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/StringHlp.h"
#include "resource.h"
#include <CommCtrl.h>
#include <windowsx.h>

extern JTApp* currentApp;
extern Logger * currentLogger;

extern HINSTANCE hInst;

SettingHlp *currentSettings;

VOID ShowMoreSettings(HWND hWndMain)
{
	currentLogger = currentApp->GetLogger();
	currentSettings = currentApp->GetSettings();
	DialogBox(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWndMain, SettingsDlgFunc);
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

		CheckDlgButton(hDlg, IDC_CHECK_INI_0, currentSettings->GetSettingBool(L"DisableDriver", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_1, currentSettings->GetSettingBool(L"SelfProtect", true) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_2, currentSettings->GetSettingBool(L"BandAllRunOp", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_3, currentSettings->GetSettingBool(L"AlwaysCheckUpdate", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_4, currentSettings->GetSettingBool(L"DoNotShowVirusWindow", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_5, currentSettings->GetSettingBool(L"ForceInstallInCurrentDir", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_6, currentSettings->GetSettingBool(L"ForceDisableWatchDog", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_7, currentSettings->GetSettingBool(L"InjectMasterHelper", false) ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hDlg, IDC_CHECK_INI_8, currentSettings->GetSettingBool(L"InjectProcHelper64", false) ? BST_CHECKED : BST_UNCHECKED);

		std::wstring *setKillProcess = currentSettings->GetSettingStrPtr(L"KillProcess", L"NtTerminateProcess");
		if(StrEqual(setKillProcess->c_str(), L"TerminateProcess"))  Button_SetCheck(GetDlgItem(hDlg, IDC_KILLPROC_TP), BST_CHECKED);
		else if (StrEqual(setKillProcess->c_str(), L"NtTerminateProcess")) Button_SetCheck(GetDlgItem(hDlg, IDC_KILLPROC_NTP), BST_CHECKED);
		else if (StrEqual(setKillProcess->c_str(), L"KernelMode"))  Button_SetCheck(GetDlgItem(hDlg, IDC_KILLPROC_PPTP_APC), BST_CHECKED);
		FreeStringPtr(setKillProcess);

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
		switch (wParam)
		{
		case IDC_CHECK_INI_0:
		case IDC_CHECK_INI_1:
		case IDC_CHECK_INI_2:
		case IDC_CHECK_INI_3:
		case IDC_CHECK_INI_4:
		case IDC_CHECK_INI_5:
		case IDC_CHECK_INI_6:
		case IDC_CHECK_INI_7:
		case IDC_CHECK_INI_8:
			WCHAR key[32]; GetDlgItemText(hDlg, wParam, key, 32);
			currentSettings->SetSettingBool(key, IsDlgButtonChecked(hDlg, wParam));
			break;
		case IDC_KILLPROC_TP:
			currentSettings->SetSettingStr(L"KillProcess", L"TerminateProcess");
			break;
		case IDC_KILLPROC_NTP:
			currentSettings->SetSettingStr(L"KillProcess", L"NtTerminateProcess");
			break;
		case IDC_KILLPROC_PPTP_APC:
			currentSettings->SetSettingStr(L"KillProcess", L"KernelMode");
			break;
		case IDC_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
			break;
		default:
			break;
		}
		break;
	}
	case WM_CLOSE: {
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	case WM_NOTIFY: {
		NMHDR*pNmhdr = (LPNMHDR)lParam;
		if (pNmhdr->idFrom == IDC_COMBO_INJECT_MODE) {
			if (pNmhdr->code == CBN_SELCHANGE) {
				int sel = SendMessage(pNmhdr->hwndFrom, CB_GETCURSEL, 0, 0);
				if(sel == 0)currentSettings->SetSettingStr(L"InjectMode", L"RemoteThread");
				else if (sel == 1)currentSettings->SetSettingStr(L"InjectMode", L"HookDllStub");
			}
		}
		break;
	}
	default:
		break;
	}
	return FALSE;
}
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
	}
	break;
	case WM_CLOSE:
	{
		EndDialog(hDlg, wParam);
		return TRUE;
	}
	break;
	}
	return FALSE;
}



