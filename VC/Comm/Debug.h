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

#if defined(WIN32)
#include <Windows.h>
#include <tchar.h>
#include <fstream>
#include "StringHelper.h"
#include "File.h"

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp")
#pragma comment(lib, "shlwapi")

#if (_WIN32_WINNT < 0x0502)
    typedef LONG InterlockedType;
    #define InterlockedIncrement64 InterlockedIncrement
#else
    typedef LONGLONG InterlockedType;
#endif

EXTERN_C DECLSPEC_IMPORT BOOL STDAPICALLTYPE PathFileExistsA(__in LPCSTR pszPath);
extern "C" IMAGE_DOS_HEADER __ImageBase;

namespace vcl4c
{
    namespace logger
    {
        inline void WriteView(const char *p_sMessage)
        {
            ::OutputDebugStringA(p_sMessage);
        }

        inline void WriteView(const std::string &p_sMessage)
        {
            WriteView(p_sMessage.c_str());
        }

        inline void WriteLog(const char *p_sMessage)
        {
            file::CreateFilePath("./Dump/");
            std::ofstream ofs(string::Format("./Dump/Log_%d_%d.txt", ::GetCurrentProcessId(), ::GetCurrentThreadId()),
                std::ios_base::app | std::ios_base::out);
            ofs << p_sMessage;
        }

        inline void WriteLogLn(const char *p_sMessage)
        {
            WriteLog(p_sMessage);
            WriteLog("\n");
        }
    }

    namespace debugger
    {
        inline void CreateDump(PEXCEPTION_POINTERS p_Exception, MINIDUMP_TYPE p_DumpType = MiniDumpNormal, const char *p_sFileName = NULL)
        {
            if(NULL == p_Exception)
            {
                __try
                {
                    ::DebugBreak();
                }
                __except(CreateDump(GetExceptionInformation(), p_DumpType, p_sFileName), EXCEPTION_EXECUTE_HANDLER)
                {

                }

                return;
            }

            file::CreateFilePath("./Dump/");

            TCHAR szPath[MAX_PATH] = {0};
            if(NULL == p_sFileName)
            {
                static InterlockedType cSeq = 0;
                INT64 iSeq = ::InterlockedIncrement64(&cSeq);
                _sntprintf(szPath, sizeof(szPath), _T("./Dump/%d_%d_%I64d.dmp"), ::GetCurrentProcessId(), ::GetCurrentThreadId(), iSeq);
            }
            else
            {
                _sntprintf(szPath, sizeof(szPath), _T("./Dump/%s"), p_sFileName);
            }

            HANDLE fp = ::CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
            if(INVALID_HANDLE_VALUE == fp)
            {
                logger::WriteLog("Dump文件初始化失败");
            }
            else
            {
                MINIDUMP_EXCEPTION_INFORMATION mei = {::GetCurrentThreadId(), p_Exception, TRUE};
                ::MiniDumpWriteDump(::GetCurrentProcess(),
                    ::GetCurrentProcessId(),
                    fp,
                    p_DumpType,
                    &mei,
                    NULL,
                    NULL);

                ::CloseHandle(fp);
            }
        }

        template<MINIDUMP_TYPE p_DumpType>
        inline long CALLBACK DoVectoredHandler(PEXCEPTION_POINTERS p_ExceptionInfo)
        {
            switch(p_ExceptionInfo->ExceptionRecord->ExceptionCode)
            {
            case DBG_PRINTEXCEPTION_C: //OutputDebugString触发的异常
            case EXCEPTION_BREAKPOINT: //断点
            case EXCEPTION_SINGLE_STEP: //单步执行
                return EXCEPTION_CONTINUE_SEARCH; //不生成Dump，继续查找结构化异常处理
            }

            if(EXCEPTION_NONCONTINUABLE != p_ExceptionInfo->ExceptionRecord->ExceptionFlags)
            {
                //::TerminateProcess(::GetCurrentProcess(), -1);
            }

            CreateDump(p_ExceptionInfo, p_DumpType);

            return EXCEPTION_CONTINUE_SEARCH;
        }

        inline void RegVectoredExceptionHandler(bool p_bEnabled, bool p_bFullDump)
        {
            static void *s_VectoredExceptionHandler = NULL;

            if (p_bEnabled)
            {
                if (NULL == s_VectoredExceptionHandler)
                {
                    //AddVectoredExceptionHandler的第1个入参感觉是控制多个向量化异常的执行顺序，但向量化异常永远是在结构化异常之前执行
                    if (p_bFullDump)
                        s_VectoredExceptionHandler = ::AddVectoredExceptionHandler(TRUE, DoVectoredHandler<MiniDumpWithFullMemory>);
                    else
                        s_VectoredExceptionHandler = ::AddVectoredExceptionHandler(TRUE, DoVectoredHandler<MiniDumpNormal>);
                }
            }
            else
            {
                if(NULL != s_VectoredExceptionHandler)
                {
                    void *pTemp = s_VectoredExceptionHandler;
                    s_VectoredExceptionHandler = NULL;
                    ::RemoveVectoredExceptionHandler(pTemp);
                }
            }
        }

        inline void DllInit(const DWORD p_dwReason, bool p_bFullDump)
        {
            switch(p_dwReason)
            {
            case DLL_PROCESS_ATTACH:
                RegVectoredExceptionHandler(true, p_bFullDump);
                break;
            case DLL_PROCESS_DETACH:
                RegVectoredExceptionHandler(false, p_bFullDump);
                break;
            }
        }

        inline HINSTANCE GetCurrentModuleHandle()
        {
            return reinterpret_cast<HINSTANCE>(&__ImageBase);
        }

        inline const std::string GetCurrentModuleFileName()
        {
            char str[MAX_PATH];
            if (0 == ::GetModuleFileName(GetCurrentModuleHandle(), str, sizeof(str)))
            {
                return "";
            }

            return str;
        }
    }
}
#endif //defined(WIN32)
