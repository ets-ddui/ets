/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)������չ���߼���

    ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
    �����˿��Ŀ����ϣ�������ã��������κα�֤��
    ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

    ��Դ��ַ: https://github.com/ets-ddui/ets
              https://gitee.com/ets-ddui/ets
    ��ԴЭ��: The MIT License (MIT)
    ��������: xinghun87@163.com
    �ٷ����ͣ�https://blog.csdn.net/xinghun61
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
                logger::WriteLog("Dump�ļ���ʼ��ʧ��");
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
            case DBG_PRINTEXCEPTION_C: //OutputDebugString�������쳣
            case EXCEPTION_BREAKPOINT: //�ϵ�
            case EXCEPTION_SINGLE_STEP: //����ִ��
                return EXCEPTION_CONTINUE_SEARCH; //������Dump���������ҽṹ���쳣����
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
                    //AddVectoredExceptionHandler�ĵ�1����θо��ǿ��ƶ���������쳣��ִ��˳�򣬵��������쳣��Զ���ڽṹ���쳣֮ǰִ��
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
