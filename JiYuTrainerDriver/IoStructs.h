#pragma once

#ifndef JIYUDRIVER
#include <Windows.h>
#endif 

typedef struct tag_JDRV_INITPARAM {
	BOOLEAN IsWin7;
	BOOLEAN IsWinXP;
	ULONG systemVersion;
}JDRV_INITPARAM,*PJDRV_INITPARAM;