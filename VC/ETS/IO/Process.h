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

#include "resource.h"
#include "IDL/IProcess.h"
#include <boost/fusion/container/vector.hpp>
#include <fusion/algorithm/iteration/concate.hpp>
#include "Handler/PsHandler.h"
#include "Handler/LineHandler.h"
#include "Handler/JsHandler.h"

namespace ets
{
    namespace io
    {
        class CProcess:
            public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
            public IProcess
        {
            BEGIN_DEFINE_MAP(CProcess)
                SIMPLE_INTERFACE(IDispatch)
                SIMPLE_INTERFACE(IProcess)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

            enum {IDL = IDTL_PROCESS};

        public:
            CProcess(boost::asio::io_service &p_iosService)
                : CDispatch(IDL, &__uuidof(IProcess), NULL),
                m_vHandler(CPsHandler(p_iosService), CLineHandler(), CJsHandler())
            {
            }

            virtual ~CProcess()
            {
            }

            HRESULT Init()
            {
                ets::fusion::concate(m_vHandler, CBind());

                return S_OK;
            }

        public:
            //IProcess实现
            STDMETHOD(Start)(BSTR p_sExecute, BSTR p_sWorkDir, BSTR p_sEnvironment, IDispatch* p_itfCallback)
            {
                HRESULT hRes = S_OK;

                std::string sExecute;
                hRes = vcl4c::itf::W2String(sExecute, p_sExecute);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                std::string sWorkDir;
                hRes = vcl4c::itf::W2String(sWorkDir, p_sWorkDir);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                std::string sEnvironment;
                hRes = vcl4c::itf::W2String(sEnvironment, p_sEnvironment);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                if (!boost::fusion::at_c<0>(m_vHandler).Start(sExecute, sWorkDir, sEnvironment))
                {
                    return E_FAIL;
                }

                boost::fusion::at_c<2>(m_vHandler).Bind(p_itfCallback);

                return S_OK;
            }

            STDMETHOD(Write)(BSTR p_sData)
            {
                HRESULT hRes = S_OK;

                std::string sData;
                hRes = vcl4c::itf::W2String(sData, p_sData);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                boost::fusion::at_c<0>(m_vHandler).Write(reinterpret_cast<const unsigned char*>(sData.c_str()), sData.size());

                return S_OK;
            }

        private:
            boost::fusion::vector<CPsHandler, CLineHandler, CJsHandler> m_vHandler;

        };
    }
}
