/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)，可扩展工具集。

    本工具软件是开源自由软件，您可以遵照 MIT 协议，修改和发布此程序。
    发布此库的目的是希望其有用，但不做任何保证。
    如果将本库用于商业项目，由于本库中的Bug，而引起的任何风险及损失，本作者不承担任何责任。

    开源地址: https://github.com/ets-ddui/ets
    开源协议: The MIT License (MIT)
    作者邮箱: xinghun87@163.com
    官方博客：https://blog.csdn.net/xinghun61
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
    NonCopyable(const NonCopyable &); //不支持拷贝构造函数
    NonCopyable & operator =(const NonCopyable &); //不支持赋值运算符
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

    //注意，这里的参数含义有调整，基类表示超时时间，这里表示循环次数
    virtual BOOL Lock(DWORD p_iLoopCount = INFINITE) throw(...)
    {
        //如果是嵌套调用，直接加1，防止死锁
        DWORD iCurrentThreadID = ::GetCurrentThreadId();
        if (iCurrentThreadID == m_iThreadID)
        {
            ++m_iNestingCount;
            return TRUE;
        }

        for (int iLoop = 0; iLoop < RETRY_COUNT; ++iLoop) //防止自旋锁过多的占用CPU，增加循环次数限制
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

        throw std::runtime_error("自旋锁等待超时");
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
            throw std::runtime_error("UnLock调用次数不匹配");
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

        if (0 < (long)::ResumeThread(m_hThread)) //如果返回值为0，表示当前线程已经是执行状态，挂起数不用再处理
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
