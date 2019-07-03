// JiYuTrainerUI.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MainWindow.h"
#include "UpdaterWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "JiYuTrainerUI.h"

JTApp* currentApp = nullptr;
MainWindow *currentMainWindow = nullptr;

UIEXPORT_CFUNC(void*) JTUI_GetMainWindow()
{
	return currentMainWindow;
}
UIEXPORT_CFUNC(int) JTUI_RunMain()
{
	currentApp = (JTApp*)JTGetApp();

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
UIEXPORT_CFUNC(int) JTUI_RunUpdate() 
{
	currentApp = (JTApp*)JTGetApp();
	UpdaterWindow updaterWindow(NULL);
	return updaterWindow.RunLoop();
}
