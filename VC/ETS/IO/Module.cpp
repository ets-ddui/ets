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
#define WIN32_LEAN_AND_MEAN //从Windows头文件中排除极少使用的信息
#include <windows.h>

#pragma warning( disable : 4996 ) //屏蔽与strncpy_s相关的警告
#define BOOST_ASIO_HAS_MOVE 1 //boost::asio对右值引用的版本识别似乎有问题，VC2010好像支持，这里手动开启

#include "resource.h"
#include <OAIdl.h>
#include "IDL/ETS.h"
#include "ATL/InterfaceImplement.h"
#include "ATL/Utility.h"
#include "IDL/IModule.h"
#include "TCPServer.h"
#include "TCPClient.h"
#include <boost/config.hpp>
#include "Process.h"
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>

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
        : CDispatch(IDL, &__uuidof(IModule), NULL),
        m_iosService()
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
    STDMETHOD(Run)(VARIANT_BOOL p_bDealMessageLoop)
    {
        m_iosService.reset(); //当多次执行时，CModule不会被释放(DLL被缓存)，上次执行结果会对本次执行产生影响，这里先重置状态

        boost::system::error_code ec;
        if (VARIANT_TRUE == p_bDealMessageLoop)
        {
            while (true)
            {
                if (0 < m_iosService.poll_one(ec))
                {
                    continue;
                }

                if (ec)
                {
                    return E_FAIL;
                }

                if (m_iosService.stopped())
                {
                    break;
                }

                DoMessageLoop(ec);

                boost::asio::steady_timer st(m_iosService);
                st.expires_from_now(boost::chrono::milliseconds(c_iMessageTime));
                st.async_wait(boost::bind(DoMessageLoop, boost::asio::placeholders::error));

                m_iosService.run_one(ec);
                if (ec)
                {
                    return E_FAIL;
                }

                st.cancel();
            }
        }
        else
        {
            while (true)
            {
                m_iosService.run(ec);
                if (ec)
                {
                    return E_FAIL;
                }

                if (m_iosService.stopped())
                {
                    break;
                }
            }
        }

        return S_OK;
    }

    STDMETHOD(GetTcpServer)(BSTR p_sHost, BSTR p_sService, IDispatch** p_itfResult)
    {
        HRESULT hRes = S_OK;

        std::string sHost;
        hRes = vcl4c::itf::W2String(sHost, p_sHost);
        if (FAILED(hRes))
        {
            return hRes;
        }

        std::string sService;
        hRes = vcl4c::itf::W2String(sService, p_sService);
        if (FAILED(hRes))
        {
            return hRes;
        }

        return vcl4c::itf::ObjectToDispatch<ets::io::CTcpServer>(p_itfResult, new (std::nothrow) ets::io::CTcpServer(m_iosService, sHost, sService));
    }

    STDMETHOD(GetTcpClient)(BSTR p_sHost, BSTR p_sService, IDispatch** p_itfResult)
    {
        HRESULT hRes = S_OK;

        std::string sHost;
        hRes = vcl4c::itf::W2String(sHost, p_sHost);
        if (FAILED(hRes))
        {
            return hRes;
        }

        std::string sService;
        hRes = vcl4c::itf::W2String(sService, p_sService);
        if (FAILED(hRes))
        {
            return hRes;
        }

        return vcl4c::itf::ObjectToDispatch<ets::io::CTcpClient>(p_itfResult, new (std::nothrow) ets::io::CTcpClient(m_iosService, sHost, sService));
    }

    STDMETHOD(GetProcess)(IDispatch** p_itfResult)
    {
        return vcl4c::itf::ObjectToDispatch<ets::io::CProcess>(p_itfResult, new (std::nothrow) ets::io::CProcess(m_iosService));
    }

private:
    const static int c_iMessageTime = 100; //处理消息循环的间隔时间(毫秒)
    boost::asio::io_service m_iosService;

    static void DoMessageLoop(const boost::system::error_code &p_eError)
    {
        CComBSTR strName(L"MessageLoop");
        CComVariant vMessageLoop;
        if (FAILED(vcl4c::itf::CManager::GetManager()->GetService(strName, &vMessageLoop)))
        {
            return;
        }

        CComVariant vResult;
        if (FAILED(vcl4c::itf::CDispatchHelper::DispatchInvoke(&vMessageLoop, "Execute", DISPATCH_METHOD, &vResult, "")))
        {
            return;
        }
    }

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
