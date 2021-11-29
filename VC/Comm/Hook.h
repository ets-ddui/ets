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
        //常规代码的Hook函数原型，在Hook代码执行前被调用
        //p_pRegisters：当前各寄存器的值
        //p_pRetAddress：回调函数执行完后，新代码执行的起始地址，如果未设置，则按原逻辑执行，否则，按指定的地址开始执行
        //p_CustomerParameter：自定义参数，供宿主程序做自定义用途使用
        typedef void (__stdcall * CHookCallback)(CRegisters *p_pRegisters, void * p_pRetAddress, void * p_CustomerParameter);
        //Function的Hook函数原型，在被Hook函数执行前后均会触发
        //p_bAccepted：是否调用原始代码(在步骤2中返回false，可再次调用一次原函数，这时需要注意避免无限循环)
        //     true - 按原始流程执行(默认)
        //    false - 不调用原始代码，按自定义方式执行
        //            与p_iStep参数配合使用，在步骤1中，直接返回函数调用，在步骤2中，重复调用函数，两种情况都需要注意栈平衡
        //            在步骤2中，如果被Hook函数为__stdcall调用协议，在被调用函数内部会做栈平衡，导致在调用Hook函数时，
        //            入参的栈空间有可能被破坏掉(当入参多于4个时，被PUSHAD指令给覆盖)
        //p_iStep：执行步骤
        //        1 - 被Hook函数执行前调用
        //        2 - 被Hook函数执行后调用
        //p_pRegisters：当前各寄存器的值，步骤1中通过修改此参数值达到控制被Hook函数执行的目的，步骤2用于检查被Hook函数的执行结果
        //p_pParamAddress：被Hook函数入参的起始地址，修改或查看此参数中变量的值，可达到与p_pRegisters类似的目的
        //p_CustomerParameter：自定义参数，供宿主程序做自定义用途使用
        //返回值含义：p_iStep为1且p_bAccepted返回false时有意义，用于堆栈平衡(包装函数通过调整PUSHAD中的ESP值实现)
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
            void *m_pNewCodeAddress; //要执行代码的起始地址，其值固定填充m_cAnchor的地址
            void *m_pOrigAddress; //被Hook的原始代码地址，UnHook用此决定还原的目标
            void *m_pReturnAddress; //锚点代码执行完后，要返回的代码地址(紧接在m_pOrigAddress后面的代码)
            DWORD m_iSize; //CHookCode的实际大小，包含后面的m_cNewCode以及m_jcJmpBack
            void *m_pCustomerParameter;
            CHookCode *m_pNextHookCode;
            enum{tldRetAddress = 0, tldCallCount = 1};
            DWORD m_iTlsLocalData[2]; //索引0保存函数的返回地址，索引1保存Hook函数调用次数
                                      //由于多个线程函数可重入，此字段定义为TLS对象(线程局部存储)，HookFunction中初始化
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
            BYTE m_cOrigCode[sizeof(CJmpCode)]; //被Hook的原始代码，用FF25 [m_pNewCodeAddress的地址]填充m_pOrigAddress的内容，
                                 //并将原始内容拷贝到此字段，实际拷贝内容可大于6，但UnHook只恢复6字节
            /*BYTE m_cNewCode[0];*/ //如果代码需要修正，则将修正后的代码存放到此变量(其地址不固定，具体由m_cOrigCode的实际长度决定)，
                                    //并将m_cJmp调整为指向这个地址
            /*CJmpCode m_jcJmpBack;*/
        };
        //CCodeMemory，本质上是个空闲链表，开头的16字节为管理信息
        //每个空闲块也有16字节，第1个4字节存放下一个空闲块的地址，第2个4字节存放当前块的大小
        //此类是非线程安全的，使用前需要增加保护机制
        //通过GetMemory分配的内存中，在-4字节的内存中存放块的大小，对使用者不可见，FreeMemory据此推断要释放的内存大小
        class CCodeMemory
        {
        private:
            struct CFreeList
            {
                CFreeList * m_pNext;
                DWORD m_iSize;
            };
            CFreeList * m_pRoot;
            CCodeMemory * m_pNextCodeMemory; //下一块内存的起始地址，解决4096字节不够用时，申请新的内存，目前暂不启用，固定填NULL
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
            //跳转指令统一转换为FF25 [32位地址]的形式，其对应汇编为jmp dword ptr [[32位地址]]
            //由于取地址操作一般有对齐的要求，因此，将代码和地址的存储进行分离
            //开辟的空间以空闲链表的方式进行管理，最开始的16字节保留自用，实现多个内存块的链表连接
            static CCodeMemory *m_pCodeMemory; //一条汇编的最大长度为15字节，这里四舍五入到16字节
                                               //以16字节为单位进行分配(最多可以分配255个单元，255 = (4096 - 16) / 16)

            //对原代码进行转译，用于修正对jzz、call这类带跳转地址的指令，修正后的代码存放到p_pCodeDest中
            //p_pCodeDest可以为NULL，这种情况下只会计算转换后代码的大小
            static int Translate(void *p_pCodeDest, bool &p_bNeedTranslate, const void *p_pCodeSrc, const CDasmDataEx & p_ddDasmData);
            static CHookCode *GetHookCodeMemory(const DWORD p_iSize);
            static void ReleaseHookCodeMemory(CHookCode *p_pHookCode);
        public:
            static void Init();
            static void UnInit();
        private:
            //不提供构造函数及析构函数的实现
            CHook(CHook &);
            CHook & operator =(CHook &);
            CHook(void *p_pAddress);
            ~CHook();

            typedef int (__stdcall * CHookCallbackWithReturn)(void *p_pHookCallback,
                CRegisters *p_pRegisters, void *p_pRetAddress, CHookCode *p_hcHookCode);
            static HANDLE Hook(CHookCallbackWithReturn p_pCallbackInner,
                void *p_pAddress, void *p_pCallback, void *p_CustomerParameter, const HANDLE p_RootHookCode);
        public:
            //Hook的返回值作为UnHook的入参，本质上是个地址指针，指向Hook中动态分配的程序地址
            //Hook为普通代码Hook
            //HookFunction为函数Hook，被Hook函数被执行前后均会触发回调函数，p_pAddress建议设置为被Hook函数的起始地址，
            //通过修改返回地址的值，达到对函数调用完成事件的跟踪
            static HANDLE HookCode(void *p_pAddress, CHookCallback p_pCallback, void * p_CustomerParameter);
            static HANDLE HookFunction(void *p_pAddress, CHookFunctionCallback p_pCallback, void * p_CustomerParameter);
            static bool UnHook(const HANDLE p_hHandle);
        };
    }
}
