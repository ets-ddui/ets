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

#include <OAIdl.h>
#include "Handler.h"
#include "ATL/Utility.h"

namespace ets
{
    namespace io
    {
        class CJsHandler
            : public CHandler
        {
        public:
            CJsHandler()
                : m_itfCallback(nullptr)
            {
            }

            explicit CJsHandler(IDispatch *p_itfCallback)
                : m_itfCallback(nullptr)
            {
                Bind(p_itfCallback);
            }

            ~CJsHandler()
            {
                Clear();
            }

            void Bind(IDispatch *p_itfCallback)
            {
                if (m_itfCallback == p_itfCallback)
                {
                    return;
                }

                Clear();

                m_itfCallback = p_itfCallback;
                if (nullptr != m_itfCallback)
                {
                    m_itfCallback->AddRef();
                }
            }

            void Clear()
            {
                if (nullptr != m_itfCallback)
                {
                    m_itfCallback->Release();
                    m_itfCallback = nullptr;
                }
            }

        public:
            //组件链接口实现
            bool DoPush(const std::string &p_sLine)
            {
                if (nullptr != m_itfCallback)
                {
                    CComVariant vResult;
                    return SUCCEEDED(vcl4c::itf::CDispatchHelper::DispatchInvoke(m_itfCallback,
                        0L, DISPATCH_METHOD, &vResult, "s", p_sLine.c_str()));
                }

                return true;
            }

            bool DoClose()
            {
                return true;
            }

            bool DoPull()
            {
                return true;
            }

        private:
            IDispatch *m_itfCallback;

        };
    }
}
