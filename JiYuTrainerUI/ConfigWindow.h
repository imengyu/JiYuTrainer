#pragma once
#include "stdafx.h"

VOID ShowMoreSettings(HWND hWndMain);

void SaveSettings(HWND hDlg);
void InitSettings(HWND hDlg);
void InitTab(HWND hDlg);


INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsDlgFunc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);