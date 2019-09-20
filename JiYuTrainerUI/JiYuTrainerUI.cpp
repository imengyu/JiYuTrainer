// JiYuTrainerUI.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MainWindow.h"
#include "UpdaterWindow.h"
#include "ConfigWindow.h"
#include "BugReportWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "JiYuTrainerUI.h"

MainWindow *currentMainWindow = nullptr;
extern JTApp * currentApp;

int JiYuTrainerUIRunMain()
{
	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);
#if _DEBUG
	SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
#endif
	SciterClassName();

	currentMainWindow = new MainWindow();
	int result = currentMainWindow->RunLoop();
	delete currentMainWindow;
	return result;
}
int JiYuTrainerUIRunUpdate()
{
	UpdaterWindow updaterWindow(NULL);
	return updaterWindow.RunLoop();
}
int JiYuTrainerUIRunConfig()
{
	ShowMoreSettings(GetDesktopWindow());
	return 0;
}
int JiYuTrainerUIRunBugReport()
{
	ShowBugReportWindow();
	return 0;
}

UIEXPORT_CFUNC(int) JiYuTrainerUICommonEntry(int i)
{
	SetUnhandledExceptionFilter(AppUnhandledExceptionFilter);
	switch (i)
	{
	case 0: return JiYuTrainerUIRunMain();
	case 1: return JiYuTrainerUIRunUpdate();
	case 2: return JiYuTrainerUIRunConfig();
	case 3: return JiYuTrainerUIRunBugReport();
	case 4: currentApp->RunOperation(AppOperation3);  return 0;
	default: return 0;
	}
}
