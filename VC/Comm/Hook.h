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

#include "Core.h"
#include <Windows.h>
#include "LDasm/LDasm.h"

namespace vcl4c
{
    namespace debugger
    {
        struct CRegisters
        {
            DWORD EFLAGS;
            DWORD EDI;
            DWORD ESI;
            DWORD EBP;
            DWORD ESP;
            DWORD EBX;
            DWORD EDX;
            DWORD ECX;
            DWORD EAX;
        };
        //��������Hook����ԭ�ͣ���Hook����ִ��ǰ������
        //p_pRegisters����ǰ���Ĵ�����ֵ
        //p_pRetAddress���ص�����ִ������´���ִ�е���ʼ��ַ�����δ���ã���ԭ�߼�ִ�У����򣬰�ָ���ĵ�ַ��ʼִ��
        //p_CustomerParameter���Զ���������������������Զ�����;ʹ��
        typedef void (__stdcall * CHookCallback)(CRegisters *p_pRegisters, void * p_pRetAddress, void * p_CustomerParameter);
        //Function��Hook����ԭ�ͣ��ڱ�Hook����ִ��ǰ����ᴥ��
        //p_bAccepted���Ƿ����ԭʼ����(�ڲ���2�з���false�����ٴε���һ��ԭ��������ʱ��Ҫע���������ѭ��)
        //     true - ��ԭʼ����ִ��(Ĭ��)
        //    false - ������ԭʼ���룬���Զ��巽ʽִ��
        //            ��p_iStep�������ʹ�ã��ڲ���1�У�ֱ�ӷ��غ������ã��ڲ���2�У��ظ����ú����������������Ҫע��ջƽ��
        //            �ڲ���2�У������Hook����Ϊ__stdcall����Э�飬�ڱ����ú����ڲ�����ջƽ�⣬�����ڵ���Hook����ʱ��
        //            ��ε�ջ�ռ��п��ܱ��ƻ���(����ζ���4��ʱ����PUSHADָ�������)
        //p_iStep��ִ�в���
        //        1 - ��Hook����ִ��ǰ����
        //        2 - ��Hook����ִ�к����
        //p_pRegisters����ǰ���Ĵ�����ֵ������1��ͨ���޸Ĵ˲���ֵ�ﵽ���Ʊ�Hook����ִ�е�Ŀ�ģ�����2���ڼ�鱻Hook������ִ�н��
        //p_pParamAddress����Hook������ε���ʼ��ַ���޸Ļ�鿴�˲����б�����ֵ���ɴﵽ��p_pRegisters���Ƶ�Ŀ��
        //p_CustomerParameter���Զ���������������������Զ�����;ʹ��
        //����ֵ���壺p_iStepΪ1��p_bAccepted����falseʱ�����壬���ڶ�ջƽ��(��װ����ͨ������PUSHAD�е�ESPֵʵ��)
        typedef DWORD (__stdcall * CHookFunctionCallback)(bool &p_bAccepted, const int p_iStep,
            CRegisters *p_pRegisters, void * p_pParamAddress, void * p_CustomerParameter);

#pragma pack(push, 1)
        struct CAddCode
        {
            BYTE m_iOpcode;
            BYTE m_iModRM;
            BYTE m_iImmediate;
        };
        struct CJmpCode
        {
            WORD m_iOpcode;
            long m_iAddress;
        };
        struct CPushCode
        {
            BYTE m_iOpcode;
            long m_iDisplacement;
        };
        struct CLeaCode
        {
            BYTE m_iOpcode;
            BYTE m_iModRM;
            BYTE m_iSIB;
            BYTE m_iDisplacement;
        };
        struct CMovCode
        {
            BYTE m_iOpcode;
            BYTE m_iModRM;
            BYTE m_iSIB;
            BYTE m_iDisplacement;
        };
        struct CCallCode
        {
            BYTE m_iOpcode;
            long m_iDisplacement;
        };
        struct CTestCode
        {
            BYTE m_iOpcode;
            BYTE m_iModRM;
        };
        struct CJneCode
        {
            BYTE m_iOpcode;
            char m_iImmediate;
        };
        struct CHookCode
        {
            void *m_pNewCodeAddress; //Ҫִ�д������ʼ��ַ����ֵ�̶����m_cAnchor�ĵ�ַ
            void *m_pOrigAddress; //��Hook��ԭʼ�����ַ��UnHook�ô˾�����ԭ��Ŀ��
            void *m_pReturnAddress; //ê�����ִ�����Ҫ���صĴ����ַ(������m_pOrigAddress����Ĵ���)
            DWORD m_iSize; //CHookCode��ʵ�ʴ�С�����������m_cNewCode�Լ�m_jcJmpBack
            void *m_pCustomerParameter;
            CHookCode *m_pNextHookCode;
            enum{tldRetAddress = 0, tldCallCount = 1};
            DWORD m_iTlsLocalData[2]; //����0���溯���ķ��ص�ַ������1����Hook�������ô���
                                      //���ڶ���̺߳��������룬���ֶζ���ΪTLS����(�ֲ߳̾��洢)��HookFunction�г�ʼ��
            struct
            {
                CAddCode m_acDEC_ESP;
                BYTE m_iPUSHAD;
                BYTE m_iPUSHFD;
                CPushCode m_pcPUSH_Argument3;
                CLeaCode m_lcLEA_Argument2;
                BYTE m_iPUSH_Argument2;
                CLeaCode m_lcLEA_Argument1;
                BYTE m_iPUSH_Argument1;
                CPushCode m_pcPUSH_Function;
                CCallCode m_ccCALL_Function;
                CTestCode m_tcTEST_EAX;
                CJneCode m_jcJNE_RET;
                BYTE m_iPOPFD_RET;
                BYTE m_iPOPAD_RET;
                CMovCode m_iMOV_ESP;
                BYTE m_iRET;
                BYTE m_iPOPFD;
                BYTE m_iPOPAD;
                CAddCode m_acADD_ESP;
            } m_cAnchor;
            CJmpCode m_jcJmp;
            BYTE m_cOrigCode[sizeof(CJmpCode)]; //��Hook��ԭʼ���룬��FF25 [m_pNewCodeAddress�ĵ�ַ]���m_pOrigAddress�����ݣ�
                                 //����ԭʼ���ݿ��������ֶΣ�ʵ�ʿ������ݿɴ���6����UnHookֻ�ָ�6�ֽ�
            /*BYTE m_cNewCode[0];*/ //���������Ҫ��������������Ĵ����ŵ��˱���(���ַ���̶���������m_cOrigCode��ʵ�ʳ��Ⱦ���)��
                                    //����m_cJmp����Ϊָ�������ַ
            /*CJmpCode m_jcJmpBack;*/
        };
        //CCodeMemory���������Ǹ�����������ͷ��16�ֽ�Ϊ������Ϣ
        //ÿ�����п�Ҳ��16�ֽڣ���1��4�ֽڴ����һ�����п�ĵ�ַ����2��4�ֽڴ�ŵ�ǰ��Ĵ�С
        //�����Ƿ��̰߳�ȫ�ģ�ʹ��ǰ��Ҫ���ӱ�������
        //ͨ��GetMemory������ڴ��У���-4�ֽڵ��ڴ��д�ſ�Ĵ�С����ʹ���߲��ɼ���FreeMemory�ݴ��ƶ�Ҫ�ͷŵ��ڴ��С
        class CCodeMemory
        {
        private:
            struct CFreeList
            {
                CFreeList * m_pNext;
                DWORD m_iSize;
            };
            CFreeList * m_pRoot;
            CCodeMemory * m_pNextCodeMemory; //��һ���ڴ����ʼ��ַ�����4096�ֽڲ�����ʱ�������µ��ڴ棬Ŀǰ�ݲ����ã��̶���NULL
        public:
            static CCodeMemory * Alloc();
            static void Free(CCodeMemory *p_pCodeMemory);
            CHookCode * GetMemory(const DWORD p_iSize);
            void FreeMemory(CHookCode *p_pHookCode);
        };
#pragma pack(pop)
        struct CDasmDataEx: public DasmData
        {
            int m_iCodeSize;
        };
        class CHook
        {
        private:
            static CCriticalSection m_csLock;
            //��תָ��ͳһת��ΪFF25 [32λ��ַ]����ʽ�����Ӧ���Ϊjmp dword ptr [[32λ��ַ]]
            //����ȡ��ַ����һ���ж����Ҫ����ˣ�������͵�ַ�Ĵ洢���з���
            //���ٵĿռ��Կ�������ķ�ʽ���й����ʼ��16�ֽڱ������ã�ʵ�ֶ���ڴ�����������
            static CCodeMemory *m_pCodeMemory; //һ��������󳤶�Ϊ15�ֽڣ������������뵽16�ֽ�
                                               //��16�ֽ�Ϊ��λ���з���(�����Է���255����Ԫ��255 = (4096 - 16) / 16)

            //��ԭ�������ת�룬����������jzz��call�������ת��ַ��ָ�������Ĵ����ŵ�p_pCodeDest��
            //p_pCodeDest����ΪNULL�����������ֻ�����ת�������Ĵ�С
            static int Translate(void *p_pCodeDest, bool &p_bNeedTranslate, const void *p_pCodeSrc, const CDasmDataEx & p_ddDasmData);
            static CHookCode *GetHookCodeMemory(const DWORD p_iSize);
            static void ReleaseHookCodeMemory(CHookCode *p_pHookCode);
        public:
            static void Init();
            static void UnInit();
        private:
            //���ṩ���캯��������������ʵ��
            CHook(CHook &);
            CHook & operator =(CHook &);
            CHook(void *p_pAddress);
            ~CHook();

            typedef int (__stdcall * CHookCallbackWithReturn)(void *p_pHookCallback,
                CRegisters *p_pRegisters, void *p_pRetAddress, CHookCode *p_hcHookCode);
            static HANDLE Hook(CHookCallbackWithReturn p_pCallbackInner,
                void *p_pAddress, void *p_pCallback, void *p_CustomerParameter, const HANDLE p_RootHookCode);
        public:
            //Hook�ķ���ֵ��ΪUnHook����Σ��������Ǹ���ַָ�룬ָ��Hook�ж�̬����ĳ����ַ
            //HookΪ��ͨ����Hook
            //HookFunctionΪ����Hook����Hook������ִ��ǰ����ᴥ���ص�������p_pAddress��������Ϊ��Hook��������ʼ��ַ��
            //ͨ���޸ķ��ص�ַ��ֵ���ﵽ�Ժ�����������¼��ĸ���
            static HANDLE HookCode(void *p_pAddress, CHookCallback p_pCallback, void * p_CustomerParameter);
            static HANDLE HookFunction(void *p_pAddress, CHookFunctionCallback p_pCallback, void * p_CustomerParameter);
            static bool UnHook(const HANDLE p_hHandle);
        };
    }
}
