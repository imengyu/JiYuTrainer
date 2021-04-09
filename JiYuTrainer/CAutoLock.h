#ifndef CAUTO_LOCK_H__
#define CAUTO_LOCK_H__

class CAutoLock
{
public:
    CAutoLock();
    ~CAutoLock();

    void Lock();
    void UnLock();

private:
    CRITICAL_SECTION m_Section;
};

#endif
