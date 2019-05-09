#pragma once
#include "Driver.h"

ULONG_PTR KxSearchFeatureCodeForAddress(ULONG_PTR StartAddress, PUCHAR FeatureCode, int FeatureCodeSize, int Search_MaxLength);
ULONG_PTR KxGetPspTerminateThreadByPointerAddressX_7Or8Or10(UCHAR FeatureCode0);
ULONG_PTR KxGetPspExitThread_32_64();