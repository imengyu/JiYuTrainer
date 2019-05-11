// JiYuTrainerUI.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "MainWindow.h"
#include "../JiYuTrainer/AppPublic.h"
#include "../JiYuTrainer/JiYuTrainer.h"
#include "JiYuTrainerUI.h"

JTApp* appCurrent = nullptr;
MainWindow *appCurrentMainWindow = nullptr;

UIEXPORT_CFUNC(void*) JTUI_GetMainWindow()
{
	return appCurrentMainWindow;
}
UIEXPORT_CFUNC(int) JTUI_RunMain()
{
	appCurrent = (JTApp*)JTGetCurrentApp();

	SciterSetOption(NULL, SCITER_SET_SCRIPT_RUNTIME_FEATURES, ALLOW_FILE_IO | ALLOW_SOCKET_IO | ALLOW_EVAL | ALLOW_SYSINFO);
	SciterSetOption(NULL, SCITER_SET_DEBUG_MODE, TRUE);
	SciterClassName();

	appCurrentMainWindow = new MainWindow();
	int result = appCurrentMainWindow->RunLoop();
	delete appCurrentMainWindow;
	return result;
}
UIEXPORT_CFUNC(int) JTUI_RunUpdate() 
{
	appCurrent = (JTApp*)JTGetCurrentApp();



	return 0;
}
