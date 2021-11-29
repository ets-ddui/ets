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

#include "Handler.h"
#include <string>

namespace ets
{
    namespace io
    {
        class CLineHandler
            : public CHandler
        {
        public:
            CLineHandler()
                : m_bSkipN(false)
            {
            }

        public:
            //组件链接口实现
            std::size_t DoPush(const unsigned char *p_sData, std::size_t p_iLen)
            {
                for (const unsigned char *sBegin = p_sData, *sEnd = p_sData + p_iLen; sBegin != sEnd; ++sBegin)
                {
                    switch (*sBegin)
                    {
                    case '\n':
                        if (m_bSkipN)
                        {
                            m_bSkipN = false;

                            continue;
                        }

                        if (!DoOnData())
                        {
                            return sBegin - p_sData;
                        }

                        break;
                    case '\r':
                        if (!DoOnData())
                        {
                            return sBegin - p_sData;
                        }

                        m_bSkipN = true;
                        
                        break;
                    default:
                        m_bSkipN = false;
                        m_sLine.push_back(*sBegin);

                        break;
                    }
                }

                return p_iLen;
            }

            bool DoClose()
            {
                if (!m_sLine.empty() && !DoOnData())
                {
                    return false;
                }

                return NextClose();
            }

            bool DoPull()
            {
                return PreviousPull();
            }

            template<typename COnCallback>
            void BindPush(const COnCallback p_onCallback)
            {
                class CCallback
                    : public IOnData
                {
                public:
                    CCallback(const COnCallback p_onCallBack)
                        : m_onCallBack(p_onCallBack)
                    {
                    }

                    bool Call(const std::string &p_sLine)
                    {
                        return m_onCallBack(p_sLine);
                    }

                private:
                    COnCallback m_onCallBack;
                };

                m_spOnData.reset(new CCallback(p_onCallback));
            }

        private:
            struct IOnData
            {
                virtual bool Call(const std::string &p_sLine) = 0;
            };

            std::string m_sLine;
            bool m_bSkipN;
            boost::shared_ptr<IOnData> m_spOnData;

            bool DoOnData()
            {
                if (m_spOnData)
                {
                    if (!m_spOnData->Call(m_sLine))
                    {
                        return false;
                    }
                }

                m_sLine.clear();

                return true;
            }

        };
    }
}
