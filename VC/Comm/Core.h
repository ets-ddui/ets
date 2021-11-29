/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)������չ���߼���

    ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
    �����˿��Ŀ����ϣ�������ã��������κα�֤��
    ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

    ��Դ��ַ: https://github.com/ets-ddui/ets
    ��ԴЭ��: The MIT License (MIT)
    ��������: xinghun87@163.com
    �ٷ����ͣ�https://blog.csdn.net/xinghun61
*/
#pragma once

#include <Windows.h>
#include <stdexcept>
#include <process.h>

class NonCopyable
{
protected:
    NonCopyable()
    {

    }

    ~NonCopyable()
    {

    }

private:
    NonCopyable(const NonCopyable &); //��֧�ֿ������캯��
    NonCopyable & operator =(const NonCopyable &); //��֧�ָ�ֵ�����
};

class CSyncObject
    : NonCopyable
{
public:
    virtual BOOL Lock(DWORD dwTimeout = INFINITE) = 0;
    virtual BOOL Unlock() = 0;
};

class CSemaphore
    : public CSyncObject
{
public:
    explicit CSemaphore(LONG lInitialCount = 1, LONG lMaxCount = 1,
        LPCTSTR pstrName = NULL, LPSECURITY_ATTRIBUTES lpsaAttributes = NULL)
    {
        m_hObject = ::CreateSemaphore(lpsaAttributes, lInitialCount, lMaxCount, pstrName);
    }

    virtual ~CSemaphore()
    {
        if (NULL == m_hObject)
        {
            ::CloseHandle(m_hObject);
            m_hObject = NULL;
        }
    }

    virtual BOOL Lock(DWORD dwTimeout = INFINITE)
    {
        DWORD dwRet = ::WaitForSingleObject(m_hObject, dwTimeout);
        if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
            return TRUE;
        else
            return FALSE;
    }

    virtual BOOL Unlock()
    {
        return Unlock(1, NULL);
    }

    BOOL Unlock(LONG lCount, LPLONG lpPrevCount = NULL)
    {
        return ::ReleaseSemaphore(m_hObject, lCount, lpPrevCount);
    }

private:
    HANDLE m_hObject;

};

class CMutex : public CSyncObject
{
public:
    explicit CMutex(BOOL bInitiallyOwn = FALSE, LPCTSTR pstrName = NULL,
        LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
    {
        m_hObject = ::CreateMutex(lpsaAttribute, bInitiallyOwn, pstrName);
    }

    virtual ~CMutex()
    {
        if (NULL == m_hObject)
        {
            ::CloseHandle(m_hObject);
            m_hObject = NULL;
        }
    }

    virtual BOOL Lock(DWORD dwTimeout = INFINITE)
    {
        DWORD dwRet = ::WaitForSingleObject(m_hObject, dwTimeout);
        if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
            return TRUE;
        else
            return FALSE;
    }

    virtual BOOL Unlock()
    {
        return ::ReleaseMutex(m_hObject);
    }

private:
    HANDLE m_hObject;

};

class CEvent
    : public CSyncObject
{
public:
    explicit CEvent(BOOL bInitiallyOwn = FALSE, BOOL bManualReset = FALSE,
        LPCTSTR pstrName = NULL, LPSECURITY_ATTRIBUTES lpsaAttribute = NULL)
    {
        m_hObject = ::CreateEvent(lpsaAttribute, bManualReset, bInitiallyOwn, pstrName);
    }

    virtual ~CEvent()
    {
        if (NULL == m_hObject)
        {
            ::CloseHandle(m_hObject);
            m_hObject = NULL;
        }
    }

    BOOL SetEvent()
    {
        return ::SetEvent(m_hObject);
    }

    BOOL PulseEvent()
    {
        return ::PulseEvent(m_hObject);
    }

    BOOL ResetEvent()
    {
        return ::ResetEvent(m_hObject);
    }

    virtual BOOL Lock(DWORD dwTimeout = INFINITE)
    {
        DWORD dwRet = ::WaitForSingleObject(m_hObject, dwTimeout);
        if (dwRet == WAIT_OBJECT_0 || dwRet == WAIT_ABANDONED)
            return TRUE;
        else
            return FALSE;
    }

    virtual BOOL Unlock()
    {
        return TRUE;
    }

private:
    HANDLE m_hObject;

};

class CCriticalSection
    : public CSyncObject
{
public:
    CCriticalSection()
    {
        HRESULT hr = Init();

        if (FAILED(hr))
        {
            //AtlThrow(hr);
            throw hr;
        }
    }

    virtual ~CCriticalSection()
    {
        ::DeleteCriticalSection(&m_sect);
    }

    virtual BOOL Lock(DWORD /*dwTimeout*/)
    {
        ::EnterCriticalSection(&m_sect);
        return TRUE;
    }

    virtual BOOL Unlock()
    {
        ::LeaveCriticalSection(&m_sect);
        return TRUE;
    }

private:
    CRITICAL_SECTION m_sect;

    HRESULT Init()
    {
        if (!InitializeCriticalSectionAndSpinCount(&m_sect, 0))
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }

        return S_OK;
    }

};

class CSpinLock
    : public CSyncObject
{
private:
    enum {RETRY_COUNT = 10};

public:
    explicit CSpinLock()
        : m_iLock(0), m_iNestingCount(0), m_iThreadID(0)
    {

    }

    //ע�⣬����Ĳ��������е����������ʾ��ʱʱ�䣬�����ʾѭ������
    virtual BOOL Lock(DWORD p_iLoopCount = INFINITE) throw(...)
    {
        //�����Ƕ�׵��ã�ֱ�Ӽ�1����ֹ����
        DWORD iCurrentThreadID = ::GetCurrentThreadId();
        if (iCurrentThreadID == m_iThreadID)
        {
            ++m_iNestingCount;
            return TRUE;
        }

        for (int iLoop = 0; iLoop < RETRY_COUNT; ++iLoop) //��ֹ�����������ռ��CPU������ѭ����������
        {
            for (INT64 i = 0; i < p_iLoopCount; ++i)
            {
                if (1 == ::InterlockedIncrement(&m_iLock))
                {
                    ++m_iNestingCount;
                    m_iThreadID = iCurrentThreadID;
                    return TRUE;
                }

                ::InterlockedDecrement(&m_iLock);
            }

            if (INFINITE == p_iLoopCount)
            {
                ::Sleep(100);
                continue;
            }
            else
            {
                return FALSE;
            }
        }

        throw std::runtime_error("�������ȴ���ʱ");
    }

    virtual BOOL Unlock()
    {
        int iNestingCount = --m_iNestingCount;

        if (0 == iNestingCount)
        {
            m_iThreadID = 0;
            ::InterlockedDecrement(&m_iLock);
        }

        if (0 > iNestingCount)
        {
            throw std::runtime_error("UnLock���ô�����ƥ��");
        }

        return TRUE;
    }

private:
    long volatile m_iLock;
    int m_iNestingCount;
    DWORD m_iThreadID;

};

class CSingleLock
{
public:
    explicit CSingleLock(CSyncObject* pObject, BOOL bInitialLock = FALSE)
    {
        m_pObject = pObject;
        m_bAcquired = FALSE;

        if (bInitialLock)
            Lock();
    }

    ~CSingleLock()
    {
        Unlock();
    }

    BOOL Lock(DWORD dwTimeOut = INFINITE)
    {
        m_bAcquired = m_pObject->Lock(dwTimeOut);
        return m_bAcquired;
    }

    BOOL Unlock()
    {
        if (m_bAcquired)
            m_bAcquired = !m_pObject->Unlock();

        return !m_bAcquired;
    }

    BOOL IsLocked()
    {
        return m_bAcquired;
    }

protected:
    CSyncObject* m_pObject;
    BOOL    m_bAcquired;
};

class CThread
    : public NonCopyable
{
public:
    explicit CThread(bool p_bSuspended = false)
        : m_hThread(0), m_bTerminated(false), m_bFinished(false), m_iSuspendCount(1)
    {
        m_hThread = reinterpret_cast<HANDLE>(_beginthreadex(NULL, 0, &ThreadProc, this, CREATE_SUSPENDED, NULL));
        if (!p_bSuspended)
        {
            Resume();
        }
    }

    virtual ~CThread()
    {
        if (0 != m_hThread)
        {
            if (!m_bFinished)
            {
                Terminate();
                Resume();
                WaitFor();
            }

            ::CloseHandle(m_hThread);
            m_hThread = 0;
        }
    }

    bool operator !() const
    {
        return 0 == m_hThread;
    }

    void Resume()
    {
        if (0 == m_hThread || m_bFinished)
        {
            return;
        }

        if (0 < (long)::ResumeThread(m_hThread)) //�������ֵΪ0����ʾ��ǰ�߳��Ѿ���ִ��״̬�������������ٴ���
        {
            ::InterlockedDecrement(&m_iSuspendCount);
        }
    }

    void Suspend()
    {
        if (0 == m_hThread || m_bFinished)
        {
            return;
        }

        if (0 <= (long)::SuspendThread(m_hThread))
        {
            ::InterlockedIncrement(&m_iSuspendCount);
        }
    }

    long GetSuspendCount() const
    {
        return m_iSuspendCount;
    }

    void Terminate()
    {
        m_bTerminated = true;
    }

    bool GetTerminated() const
    {
        return m_bTerminated;
    }

    void WaitFor()
    {
        if (!m_bFinished)
        {
            WaitForSingleObject(m_hThread, INFINITE);
        }
    }

protected:
    virtual void Execute() = 0;

private:
    HANDLE m_hThread;
    bool m_bTerminated, m_bFinished;
    long volatile m_iSuspendCount;

private:
    static unsigned int _stdcall ThreadProc(void *p_pParameter)
    {
        CThread * thdThis = reinterpret_cast<CThread *>(p_pParameter);
        if (!thdThis->GetTerminated())
        {
            thdThis->Execute();
        }

        thdThis->m_bFinished = true;

        return 0;
    }

};
