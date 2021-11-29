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
#include "IDL/IScript.h"

namespace ets
{
    namespace v8
    {
        class CV8:
            public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
            public IScript
        {
            BEGIN_DEFINE_MAP(CV8)
                SIMPLE_INTERFACE(IDispatch)
                SIMPLE_INTERFACE(IScript)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

            enum {IDL = IDTL_SCRIPT};

        public:
            CV8()
                : CDispatch(IDL, &__uuidof(IScript), NULL)
            {
            }

            virtual ~CV8()
            {
            }

            HRESULT Init()
            {
                return S_OK;
            }

        public:
            //IScript实现
            STDMETHOD(RegContainer)(IDispatch* p_itfContainer)
            {
                // TODO: 在此添加实现代码

                return S_OK;
            }

            STDMETHOD(RegFrame)(IDispatch* p_itfFrame)
            {
                // TODO: 在此添加实现代码

                return S_OK;
            }

            STDMETHOD(RunModule)(BSTR p_sFileName, BSTR p_sEntryFunction)
            {
                // TODO: 在此添加实现代码

                return S_OK;
            }

            STDMETHOD(RunCode)(BSTR p_sCode)
            {
                // TODO: 在此添加实现代码

                return S_OK;
            }

        };

    }
}
