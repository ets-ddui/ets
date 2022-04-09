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
#include <vector>
#include "Hook.h"
#include "Debug.h"

namespace vcl4c
{
    namespace debugger
    {
        CCodeMemory * CCodeMemory::Alloc()
        {
            //1.0 �����ڴ�
            CCodeMemory *pHeader = reinterpret_cast<CCodeMemory *>(VirtualAlloc(NULL, 4096, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE));
            if(NULL == pHeader)
            {
                return NULL;
            }

            //2.0 ��ʼ���ڴ������Ϣ
            pHeader->m_pRoot = reinterpret_cast<CCodeMemory::CFreeList *>(reinterpret_cast<BYTE *>(pHeader) + 16);
            pHeader->m_pNextCodeMemory = NULL;

            //3.0 ��ʼ�����������еĵ�1�����п�
            pHeader->m_pRoot->m_pNext = NULL;
            pHeader->m_pRoot->m_iSize = 4096 - 16;

            return pHeader;
        }

        void CCodeMemory::Free(CCodeMemory *p_pCodeMemory)
        {
            CCodeMemory * pCurrMemory = NULL, * pNextMemory = NULL;

            pCurrMemory = p_pCodeMemory;
            while(NULL != pCurrMemory)
            {
                pNextMemory = pCurrMemory->m_pNextCodeMemory;
                VirtualFree(pCurrMemory, 0, MEM_RELEASE);
                pCurrMemory = pNextMemory;
            }
        }

        CHookCode * CCodeMemory::GetMemory(const DWORD p_iSize)
        {
            //��p_iSize�������뵽16�ı���
            //�����p_iSize��sizeof(int)��Ŀ����Ϊ��ſ��С���ڴ�����ռ䣬���ڴ��ڷ����ڴ�-4��λ�ã���ʹ���߲��ɼ�
            DWORD iSize = (p_iSize + sizeof(int) + 16 - 1) & ~0xF;

            CFreeList *pPrevFree = reinterpret_cast<CFreeList *>(this);
            CFreeList *pFree = m_pRoot;
            while(NULL != pFree && iSize > pFree->m_iSize)
            {
                pPrevFree = pFree;
                pFree = pFree->m_pNext;
            }

            //���ҵ����ʵĿ��п�����ȴӺ��и�����ɱ�֤�Կ�������ͷ��Ϣ��������
            int *pResult = NULL;
            if(NULL == pFree)
            {
                return NULL;
            }
            else if(iSize == pFree->m_iSize)
            {
                //TODO: ����Ҫע�⣬��������ǵ�1�����п�ʱ��pPrevFreeָ�����this����this����CFreeList *���ͣ�
                //���������Ϊm_pNext�ֶ��Ǽ��ݵģ����δ�������ж�
                //if(pPrevFree == this)
                pPrevFree->m_pNext = pFree->m_pNext;
                pResult = reinterpret_cast<int *>(pFree);
            }
            else
            {
                pFree->m_iSize -= iSize;
                pResult = reinterpret_cast<int *>(reinterpret_cast<BYTE *>(pFree) + pFree->m_iSize);
            }

            memset(pResult, 0, iSize);
            *pResult = iSize;
            return reinterpret_cast<CHookCode *>(reinterpret_cast<BYTE *>(pResult) + sizeof(int));
        }

        void CCodeMemory::FreeMemory(CHookCode *p_pHookCode)
        {
            if(NULL == p_pHookCode)
            {
                return;
            }

            CFreeList *pFree = reinterpret_cast<CFreeList *>(reinterpret_cast<BYTE *>(p_pHookCode) - sizeof(int));
            DWORD iSize = *reinterpret_cast<int *>(pFree);
            pFree->m_pNext = m_pRoot;
            pFree->m_iSize = iSize;
            m_pRoot = pFree;
            return;
        }

        CCriticalSection CHook::m_csLock;
        CCodeMemory *CHook::m_pCodeMemory = NULL;

        int CHook::Translate(void *p_pCodeDest, bool &p_bNeedTranslate, const void *p_pCodeSrc, const CDasmDataEx & p_ddDasmData)
        {
            p_bNeedTranslate = false;

            //TODO: ��Ҫ��jzz��call������Ҫ�����Ĵ������ж�
            if(NULL != p_pCodeDest)
            {
                memcpy(p_pCodeDest, p_pCodeSrc, p_ddDasmData.m_iCodeSize);
            }
            return p_ddDasmData.m_iCodeSize;
        }

        HANDLE CHook::Hook(CHookCallbackWithReturn p_pCallbackInner,
            void *p_pAddress, void *p_pCallback, void * p_CustomerParameter, const HANDLE p_RootHookCode)
        {
            //1.0 ����Ҫ�����Ĵ�����ֽڳ���
            BYTE *pAddress = reinterpret_cast<BYTE *>(p_pAddress);
            std::vector<CDasmDataEx> vDasmData;
            int iCodeLen = 0;
            while(iCodeLen < 6)
            {
                CDasmDataEx dd;
                dd.m_iCodeSize = Dasm(&dd, pAddress, false);
                if(dd.m_iCodeSize < 0)
                {
                    if(NULL == p_pAddress)
                    {
                        break;
                    }

                    return NULL;
                }

                vDasmData.push_back(dd);
                iCodeLen += dd.m_iCodeSize;
                pAddress = pAddress + dd.m_iCodeSize;
            }

            //2.0 ����ê�����Ĵ�С��������Ӧ�ռ�
            bool bNeedTranslate = false, bTempNeedTranslate = false;
            CHookCode *pHookCode = NULL;
            int iHookCodeSize = sizeof(CHookCode) - sizeof(pHookCode->m_cOrigCode) + iCodeLen + sizeof(CJmpCode);
            int iTempHookCodeSize = 0;
            pAddress = reinterpret_cast<BYTE *>(p_pAddress);
            for(std::vector<CDasmDataEx>::const_iterator it = vDasmData.begin(); it != vDasmData.end(); ++it)
            {
                iTempHookCodeSize += Translate(NULL, bTempNeedTranslate, pAddress, *it);
                pAddress = pAddress + it->m_iCodeSize;
                bNeedTranslate = bNeedTranslate || bTempNeedTranslate;
            }

            iHookCodeSize += bNeedTranslate ? iTempHookCodeSize : 0;
            pHookCode = GetHookCodeMemory(iHookCodeSize);
            if(NULL == pHookCode)
            {
                return NULL;
            }

            //3.0 ���ê�����
            pHookCode->m_pNewCodeAddress = &pHookCode->m_cAnchor;
            pHookCode->m_pOrigAddress = p_pAddress;
            pHookCode->m_pReturnAddress = reinterpret_cast<BYTE *>(p_pAddress) + iCodeLen;
            pHookCode->m_iSize = iHookCodeSize;
            pHookCode->m_pCustomerParameter = p_CustomerParameter;
            pHookCode->m_pNextHookCode = NULL;
            for(int i = 0; i < sizeof(pHookCode->m_iTlsLocalData) / sizeof(pHookCode->m_iTlsLocalData[0]); ++i)
            {
                pHookCode->m_iTlsLocalData[i] = TLS_OUT_OF_INDEXES;
            }

            //3.1 ����ԭʼ����
            if(NULL != p_pAddress)
            {
                memcpy(pHookCode->m_cOrigCode, p_pAddress, iCodeLen);
            }
            else
            {
                memset(pHookCode->m_cOrigCode, 0x90, sizeof(pHookCode->m_cOrigCode));
            }

            //3.2 ԭʼ��������(��������뿽�����µ�ַ)
            if(bNeedTranslate)
            {
                pHookCode->m_jcJmp.m_iOpcode = 0x25FF;
                pHookCode->m_jcJmp.m_iAddress = reinterpret_cast<long>(pHookCode->m_pOrigAddress) + iCodeLen;

                BYTE *pSrc = reinterpret_cast<BYTE *>(p_pAddress);
                BYTE *pDest = pHookCode->m_cOrigCode + iCodeLen;
                for(std::vector<CDasmDataEx>::const_iterator it = vDasmData.begin(); it != vDasmData.end(); ++it)
                {
                    pDest += Translate(pDest, bTempNeedTranslate, pSrc, *it);
                    pSrc += it->m_iCodeSize;
                }
            }
            else
            {
                memset(&pHookCode->m_jcJmp, 0x90, sizeof(CJmpCode));
            }

            //3.3 ���ê����ڴ���
            memset(&pHookCode->m_cAnchor, 0x90, sizeof(pHookCode->m_cAnchor));
            //ADD ESP, -0x10 //����4���ֲ��������ڴ�ռ䣬Ϊ������2��Hook�����������ص�ֵַ�������
            pHookCode->m_cAnchor.m_acDEC_ESP.m_iOpcode = 0x83;
            pHookCode->m_cAnchor.m_acDEC_ESP.m_iModRM = 0xC4;
            pHookCode->m_cAnchor.m_acDEC_ESP.m_iImmediate = 0xF0;
            //PUSHAD
            pHookCode->m_cAnchor.m_iPUSHAD = 0x60;
            //PUSHFD
            pHookCode->m_cAnchor.m_iPUSHFD = 0x9C;
            //PUSH pHookCode
            pHookCode->m_cAnchor.m_pcPUSH_Argument3.m_iOpcode = 0x68;
            pHookCode->m_cAnchor.m_pcPUSH_Argument3.m_iDisplacement =
                reinterpret_cast<long>(NULL == p_RootHookCode ? pHookCode : p_RootHookCode);
            //LEA EAX, [ESP + 56] //p_pRetAddress
            pHookCode->m_cAnchor.m_lcLEA_Argument2.m_iOpcode = 0x8D;
            pHookCode->m_cAnchor.m_lcLEA_Argument2.m_iModRM = 0x44;
            pHookCode->m_cAnchor.m_lcLEA_Argument2.m_iSIB = 0x24;
            pHookCode->m_cAnchor.m_lcLEA_Argument2.m_iDisplacement = 56;
            //PUSH EAX
            pHookCode->m_cAnchor.m_iPUSH_Argument2 = 0x50;
            //LEA EAX, [ESP + 8] //p_pRegisters
            pHookCode->m_cAnchor.m_lcLEA_Argument1.m_iOpcode = 0x8D;
            pHookCode->m_cAnchor.m_lcLEA_Argument1.m_iModRM = 0x44;
            pHookCode->m_cAnchor.m_lcLEA_Argument1.m_iSIB = 0x24;
            pHookCode->m_cAnchor.m_lcLEA_Argument1.m_iDisplacement = 8;
            //PUSH EAX
            pHookCode->m_cAnchor.m_iPUSH_Argument1 = 0x50;
            //PUSH p_pCallback
            pHookCode->m_cAnchor.m_pcPUSH_Function.m_iOpcode = 0x68;
            pHookCode->m_cAnchor.m_pcPUSH_Function.m_iDisplacement = reinterpret_cast<long>(p_pCallback);
            //CALL p_pCallbackInner
            pHookCode->m_cAnchor.m_ccCALL_Function.m_iOpcode = 0xE8;
            pHookCode->m_cAnchor.m_ccCALL_Function.m_iDisplacement = reinterpret_cast<long>(p_pCallbackInner)
                - reinterpret_cast<long>(&pHookCode->m_cAnchor.m_ccCALL_Function)
                - sizeof(pHookCode->m_cAnchor.m_ccCALL_Function);
            //TEST EAX, EAX
            pHookCode->m_cAnchor.m_tcTEST_EAX.m_iOpcode = 0x85;
            pHookCode->m_cAnchor.m_tcTEST_EAX.m_iModRM = 0xC0;
            //JNE 8(Offset)
            pHookCode->m_cAnchor.m_jcJNE_RET.m_iOpcode = 0x75;
            pHookCode->m_cAnchor.m_jcJNE_RET.m_iImmediate = reinterpret_cast<BYTE *>(&pHookCode->m_cAnchor.m_iPOPFD)
                - reinterpret_cast<BYTE *>(&pHookCode->m_cAnchor.m_jcJNE_RET.m_iImmediate)
                - sizeof(pHookCode->m_cAnchor.m_jcJNE_RET.m_iImmediate);
            //POPFD
            pHookCode->m_cAnchor.m_iPOPFD_RET = 0x9D;
            //POPAD
            pHookCode->m_cAnchor.m_iPOPAD_RET = 0x61;
            //MOV ESP, [ESP - 20] //POPAD�������ESP��ֵ����Ҫ��ESP������������֤��ջƽ��
            pHookCode->m_cAnchor.m_iMOV_ESP.m_iOpcode = 0x8B;
            pHookCode->m_cAnchor.m_iMOV_ESP.m_iModRM = 0x64;
            pHookCode->m_cAnchor.m_iMOV_ESP.m_iSIB = 0x24;
            pHookCode->m_cAnchor.m_iMOV_ESP.m_iDisplacement = -20;
            //RET
            pHookCode->m_cAnchor.m_iRET = 0xC3;
            //POPFD
            pHookCode->m_cAnchor.m_iPOPFD = 0x9D;
            //POPAD
            pHookCode->m_cAnchor.m_iPOPAD = 0x61;
            //ADD ESP, 0x10
            pHookCode->m_cAnchor.m_acADD_ESP.m_iOpcode = 0x83;
            pHookCode->m_cAnchor.m_acADD_ESP.m_iModRM = 0xC4;
            pHookCode->m_cAnchor.m_acADD_ESP.m_iImmediate = 0x10;

            //3.4 ��䷵����ת����
            CJmpCode *jcJmpBack = reinterpret_cast<CJmpCode *>(reinterpret_cast<long>(pHookCode) + iHookCodeSize - sizeof(CJmpCode));
            jcJmpBack->m_iOpcode = 0x25FF;
            jcJmpBack->m_iAddress = reinterpret_cast<long>(&pHookCode->m_pReturnAddress);

            //4.0 �滻Ŀ�����
            if(NULL != p_pAddress)
            {
                CJmpCode *pDest = reinterpret_cast<CJmpCode *>(p_pAddress);
                DWORD iNewProtect = PAGE_EXECUTE_READWRITE, iOldProtect = 0;
                if(VirtualProtect(pDest, sizeof(CJmpCode), iNewProtect, &iOldProtect))
                {
                    pDest->m_iOpcode = 0x25FF;
                    pDest->m_iAddress = reinterpret_cast<long>(pHookCode);
                    VirtualProtect(pDest, sizeof(CJmpCode), iOldProtect, &iNewProtect);
                }
                else
                {
                    ReleaseHookCodeMemory(pHookCode);
                    return NULL;
                }
            }

            return pHookCode;
        }

        static int __stdcall DoHookCode(void *p_pHookCallback,
            CRegisters *p_pRegisters, void * p_pRetAddress, CHookCode *p_hcHookCode)
        {
            reinterpret_cast<CHookCallback>(p_pHookCallback)(p_pRegisters, p_pRetAddress, p_hcHookCode->m_pCustomerParameter);
            return TRUE;
        }

        HANDLE CHook::HookCode(void *p_pAddress, CHookCallback p_pCallback, void * p_CustomerParameter)
        {
            return Hook(DoHookCode, p_pAddress, p_pCallback, p_CustomerParameter, NULL);
        }

        static int __stdcall DoHookFunctionEnter(void *p_pHookCallback,
            CRegisters *p_pRegisters, void * p_pRetAddress, CHookCode *p_hcHookCode)
        {
            bool bAccepted = true;
            int iEspBalance = reinterpret_cast<CHookFunctionCallback>(p_pHookCallback)(bAccepted, 1, p_pRegisters,
                reinterpret_cast<BYTE *>(p_pRetAddress) + sizeof(long), p_hcHookCode->m_pCustomerParameter);

            TlsSetValue(p_hcHookCode->m_iTlsLocalData[CHookCode::tldCallCount], reinterpret_cast<void *>(1));

            if(bAccepted)
            {
                TlsSetValue(p_hcHookCode->m_iTlsLocalData[CHookCode::tldRetAddress],
                    reinterpret_cast<void *>(*reinterpret_cast<long *>(p_pRetAddress)));
                *reinterpret_cast<long *>(p_pRetAddress) = reinterpret_cast<long>(&p_hcHookCode->m_pNextHookCode->m_cAnchor);
            }
            else
            {
                p_pRegisters->ESP += 0x10 + iEspBalance;
                *reinterpret_cast<long *>(p_pRegisters->ESP) = *reinterpret_cast<long *>(p_pRetAddress);
            }

            return bAccepted;
        }

        static int __stdcall DoHookFunctionReturn(void *p_pHookCallback,
            CRegisters *p_pRegisters, void * p_pRetAddress, CHookCode *p_hcHookCode)
        {
            bool bAccepted = true;
            reinterpret_cast<CHookFunctionCallback>(p_pHookCallback)(bAccepted, 2, p_pRegisters,
                reinterpret_cast<BYTE *>(p_pRetAddress), p_hcHookCode->m_pCustomerParameter);

            long i = reinterpret_cast<long>(TlsGetValue(p_hcHookCode->m_iTlsLocalData[CHookCode::tldCallCount]));
            ++i;
            if(5 == i)
            {
                vcl4c::logger::WriteView("DoHookFunctionReturn���ô�������");
            }
            TlsSetValue(p_hcHookCode->m_iTlsLocalData[CHookCode::tldCallCount], reinterpret_cast<void *>(i));

            if(bAccepted)
            {
                //��Hook����ִ����󣬷��ص�ַ�Ѿ����������������½�ԭ���ص�ַѹ���ջ��
                //Ȼ�󣬺����������FALSE�������Ϳ��Խ��ú����RETָ��ص�Ŀ�����
                //��DoHookFunctionEnter�Աȣ����ﲻ��Ҫ��ջƽ�⣬��Ϊ��Hook�����Ѿ������
                p_pRegisters->ESP = p_pRegisters->ESP + 0x10 - sizeof(long);
                *reinterpret_cast<long *>(p_pRegisters->ESP) =
                    reinterpret_cast<long>(TlsGetValue(p_hcHookCode->m_iTlsLocalData[CHookCode::tldRetAddress]));
            }
            else
            {
                //����ִ��һ��ԭ�������ã���ê�����������תָ�ʼִ��(����ԭʼ���뿪ʼִ�У�����DoHookFunctionEnter)
                p_pRegisters->ESP = p_pRegisters->ESP + 0x10 - 2 * sizeof(long);
                *reinterpret_cast<long *>(p_pRegisters->ESP) = reinterpret_cast<long>(&p_hcHookCode->m_jcJmp);
                *reinterpret_cast<long *>(p_pRegisters->ESP + sizeof(long)) =
                    reinterpret_cast<long>(&p_hcHookCode->m_pNextHookCode->m_cAnchor);
            }

            return FALSE;
        }

        HANDLE CHook::HookFunction(void *p_pAddress, CHookFunctionCallback p_pCallback, void * p_CustomerParameter)
        {
            CHookCode *pHookCode = reinterpret_cast<CHookCode *>(
                Hook(DoHookFunctionEnter, p_pAddress, p_pCallback, p_CustomerParameter, NULL));
            if(NULL == pHookCode)
            {
                return NULL;
            }

            pHookCode->m_pNextHookCode = reinterpret_cast<CHookCode *>(
                Hook(DoHookFunctionReturn, NULL, p_pCallback, p_CustomerParameter, pHookCode));
            if(NULL == pHookCode->m_pNextHookCode)
            {
                UnHook(pHookCode);
                return NULL;
            }

            for(int i = 0; i < sizeof(pHookCode->m_iTlsLocalData) / sizeof(pHookCode->m_iTlsLocalData[0]); ++i)
            {
                pHookCode->m_iTlsLocalData[i] = TlsAlloc();
                if(TLS_OUT_OF_INDEXES == pHookCode->m_iTlsLocalData[i])
                {
                    UnHook(pHookCode);
                    return NULL;
                }
            }

            return pHookCode;
        }

        bool CHook::UnHook(const HANDLE p_hHandle)
        {
            CHookCode *pHookCode = reinterpret_cast<CHookCode *>(p_hHandle);
            if(NULL != pHookCode->m_pNextHookCode)
            {
                UnHook(pHookCode->m_pNextHookCode);
            }

            for(int i = 0; i < sizeof(pHookCode->m_iTlsLocalData) / sizeof(pHookCode->m_iTlsLocalData[0]); ++i)
            {
                if(TLS_OUT_OF_INDEXES != pHookCode->m_iTlsLocalData[i])
                {
                    TlsFree(pHookCode->m_iTlsLocalData[i]);
                    pHookCode->m_iTlsLocalData[i] = TLS_OUT_OF_INDEXES;
                }
            }

            void *pDest = pHookCode->m_pOrigAddress;
            if(NULL == pDest)
            {
                return true;
            }

            DWORD iNewProtect = PAGE_EXECUTE_READWRITE, iOldProtect = 0;
            if(VirtualProtect(pDest, sizeof(pHookCode->m_cOrigCode), iNewProtect, &iOldProtect))
            {
                memcpy(pDest, pHookCode->m_cOrigCode, sizeof(pHookCode->m_cOrigCode));
                VirtualProtect(pDest, sizeof(pHookCode->m_cOrigCode), iOldProtect, &iNewProtect);

                return true;
            }
            else
            {
                return false;
            }
        }

        void CHook::Init()
        {
            CSingleLock slLock(&m_csLock, TRUE);

            if(NULL == m_pCodeMemory)
            {
                m_pCodeMemory = CCodeMemory::Alloc();
            }
        }

        void CHook::UnInit()
        {
            CSingleLock slLock(&m_csLock, TRUE);

            if(NULL != m_pCodeMemory)
            {
                CCodeMemory * pHeader = m_pCodeMemory;
                m_pCodeMemory = NULL;
                CCodeMemory::Free(pHeader);
            }
        }

        CHookCode * CHook::GetHookCodeMemory(const DWORD p_iSize)
        {
            CSingleLock slLock(&m_csLock, TRUE);

            if(NULL == m_pCodeMemory)
            {
                Init();
                if(NULL == m_pCodeMemory)
                {
                    return NULL;
                }
            }

            return reinterpret_cast<CHookCode *>(m_pCodeMemory->GetMemory(p_iSize));
        }

        void CHook::ReleaseHookCodeMemory( CHookCode *p_pHookCode )
        {
            CSingleLock slLock(&m_csLock, TRUE);

            if(NULL == p_pHookCode)
            {
                return;
            }

            m_pCodeMemory->FreeMemory(p_pHookCode);
        }

    }
}
