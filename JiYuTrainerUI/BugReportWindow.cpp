#include "stdafx.h"
#include "BugReportWindow.h"
#include "resource.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/StringHlp.h"
#include "../JiYuTrainer/SysHlp.h"
#include "../JiYuTrainerUpdater/JiYuTrainerUpdater.h"

extern JTApp* currentApp;

void ShowBugReportWindow() 
{
	DialogBox(currentApp->GetInstance(), MAKEINTRESOURCE(IDD_BUGREPORT), NULL, BugReportDlgFunc);
}

INT_PTR CALLBACK BugReportDlgFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG: {

		SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_BUG)));
		SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(currentApp->GetInstance(), MAKEINTRESOURCE(IDI_BUG)));
		SetDlgItemText(hDlg, IDC_BUGREPORT_CONTENT, L"无法获取程序错误日志");
		CheckDlgButton(hDlg, IDC_REBOOT, BST_CHECKED);

		int  appArgCount = currentApp->GetCommandLineArraySize();
		LPWSTR *appArgList = currentApp->GetCommandLineArray();
		int argFIndex = currentApp->FindArgInCommandLine(appArgList, appArgCount, L"bugfile");
		if (argFIndex >= 0 && (argFIndex + 1) < appArgCount) {
			LPCWSTR bugFilePath = appArgList[argFIndex + 1];
			CHAR szTest[1000];
			memset(szTest, 0, sizeof(szTest));
			std::string bugReportContent;
			int len = 0;
			FILE *fp = NULL;
			_wfopen_s(&fp, bugFilePath, L"r");
			if (NULL != fp)
			{
				while (fgets(szTest, 1000, fp) != NULL)
				{
					bugReportContent += szTest;
					memset(szTest, 0, sizeof(szTest));
				}
				fclose(fp);

				SetDlgItemTextA(hDlg, IDC_BUGREPORT_CONTENT, bugReportContent.c_str());
			}
		}

		return TRUE;
	}
	case WM_COMMAND: {
		switch (wParam)
		{
		case IDOK: {
			if (JUpdater_CheckInternet()) {
				Sleep(2000);
				MessageBox(hDlg, L"您的错误报告已成功提交，感谢您的支持！", L"JiYuTrainer", MB_ICONINFORMATION);
				EndDialog(hDlg, LOWORD(wParam));
			}
			else {
				MessageBox(hDlg, L"无法提交错误报告，可能是网络未连接？", L"JiYuTrainer", MB_ICONEXCLAMATION);
			}
			if (IsDlgButtonChecked(hDlg, IDC_REBOOT))
				SysHlp::RunApplication(currentApp->GetFullPath(), L"-r3");
			return TRUE;
		}
		case IDCANCEL: {
			if (IsDlgButtonChecked(hDlg, IDC_REBOOT))
				SysHlp::RunApplication(currentApp->GetFullPath(), L"-r3");
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}		   
		default:
			break;
		}
		break;
	}
	default:
		break;
	}
	return FALSE;
}