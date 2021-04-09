#include "stdafx.h"
#include "CAutoLock.h"

CAutoLock::CAutoLock()
{
    InitializeCriticalSection(&m_Section);
    //Lock();如果是用的时候只定义锁对象，可以不手动进入临界区和退出临界区
}
CAutoLock::~CAutoLock()
{
    DeleteCriticalSection(&m_Section);
    //UnLock();
}
void CAutoLock::Lock()
{
    EnterCriticalSection(&m_Section);
}
void CAutoLock::UnLock()
{
    LeaveCriticalSection(&m_Section);
}