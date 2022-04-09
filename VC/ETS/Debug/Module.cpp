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
#include "stdafx.h"
#include "resource.h"
#include <OAIdl.h>
#include "IDL/ETS.h"
#include "ATL/InterfaceImplement.h"
#include "ATL/Utility.h"
#include "IDL/IModule.h"

class CModule:
    public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
    public IModule
{
    BEGIN_DEFINE_MAP(CModule)
        SIMPLE_INTERFACE(IDispatch)
        SIMPLE_INTERFACE(IModule)
    END_DEFINE_MAP()

    IMPLEMENT_IUNKNOWN()

    enum {IDL = IDTL_MODULE};

public:
    CModule()
        : CDispatch(IDL, &__uuidof(IModule), NULL)
    {
    }

    virtual ~CModule()
    {
    }

    HRESULT Init()
    {
        return S_OK;
    }

public:
    //IModuleʵ��

};

STDAPI GetModule(IManager *p_itfManager, IDispatch **p_itfResult)
{
    vcl4c::itf::CManager::InitManager(p_itfManager);
    return vcl4c::itf::ObjectToDispatch<CModule>(p_itfResult, new (std::nothrow) CModule());
}

BOOL APIENTRY DllMain(HMODULE p_hModule, DWORD  p_iReason, LPVOID p_pReserved)
{
    switch (p_iReason)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        vcl4c::itf::CManager::InitManager(nullptr);
        break;
    }

    return TRUE;
}
