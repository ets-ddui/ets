/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)，可扩展工具集。

    本工具软件是开源自由软件，您可以遵照 MIT 协议，修改和发布此程序。
    发布此库的目的是希望其有用，但不做任何保证。
    如果将本库用于商业项目，由于本库中的Bug，而引起的任何风险及损失，本作者不承担任何责任。

    开源地址: https://github.com/ets-ddui/ets
              https://gitee.com/ets-ddui/ets
    开源协议: The MIT License (MIT)
    作者邮箱: xinghun87@163.com
    官方博客：https://blog.csdn.net/xinghun61
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
    //IModule实现

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
