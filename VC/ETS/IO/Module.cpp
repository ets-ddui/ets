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
#define WIN32_LEAN_AND_MEAN //��Windowsͷ�ļ����ų�����ʹ�õ���Ϣ
#include <windows.h>

#pragma warning( disable : 4996 ) //������strncpy_s��صľ���
#define BOOST_ASIO_HAS_MOVE 1 //boost::asio����ֵ���õİ汾ʶ���ƺ������⣬VC2010����֧�֣������ֶ�����

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
    //IModuleʵ��
    STDMETHOD(Run)(VARIANT_BOOL p_bDealMessageLoop)
    {
        m_iosService.reset(); //�����ִ��ʱ��CModule���ᱻ�ͷ�(DLL������)���ϴ�ִ�н����Ա���ִ�в���Ӱ�죬����������״̬

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
    const static int c_iMessageTime = 100; //������Ϣѭ���ļ��ʱ��(����)
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
