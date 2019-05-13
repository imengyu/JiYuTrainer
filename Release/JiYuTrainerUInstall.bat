@echo off
@ping 127.0.0.1 - n 6 > nul
del /F /Q E:\Programming\WinProjects\JiYuTrainer\Release\JiYuTrainer.bat
del /F /Q E:\Programming\WinProjects\JiYuTrainer\Release\JiYuTrainerUI.dll
del /F /Q E:\Programming\WinProjects\JiYuTrainer\Release\JiYuTrainerHooks.dll
del /F /Q E:\Programming\WinProjects\JiYuTrainer\Release\JiYuTrainerDriver.sys
del /F /Q E:\Programming\WinProjects\JiYuTrainer\Release\JiYuTrainerUpdater.dll
del /F /Q E:\Programming\WinProjects\JiYuTrainer\Release\sciter.dll
del %0
