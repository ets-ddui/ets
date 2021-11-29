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
#include <vector>
#include <string>
#include "../Buffer.h"

namespace ets
{
    namespace io
    {
        typedef std::vector<std::pair<std::string, std::string>> CHttpHeader;

        template<int c_iSize>
        class CHttpHandler
            : public CHandler
        {
        public:
            CHttpHandler()
                : m_ntType(ntFirstLine), m_iReturnCount(0), m_bNeedCallback(false)
            {
            }

        public:
            //组件链接口实现
            std::size_t DoPush(const unsigned char *p_sData, std::size_t p_iLen)
            {
                const unsigned char *sBegin = p_sData, *sEnd = p_sData + p_iLen;
                while (sBegin != sEnd)
                {
                    m_bNeedCallback = true;

                    if (ntData == m_ntType)
                    {
                        /* TODO: 增加读取Post数据的逻辑 */

                        if (!DoOnData())
                        {
                            break;
                        }

                        continue;
                    }

                    switch (*sBegin)
                    {
                    case '\r':
                        ++sBegin;
                        if (ntFirstLine == m_ntType)
                        {
                            m_ntType = ntFieldName;
                        }
                        else if (ntFieldName == m_ntType || ntFieldValue == m_ntType)
                        {
                            m_ntType = ntFieldName;
                            m_vHeader.push_back(std::make_pair(m_sFieldName, m_sFieldValue));
                            m_sFieldName.clear();
                            m_sFieldValue.clear();
                        }

                        continue;
                    case '\n':
                        ++sBegin;
                        ++m_iReturnCount;
                        if (2 == m_iReturnCount)
                        {
                            m_ntType = ntData;
                        }

                        continue;
                    case ':':
                        m_iReturnCount = 0;

                        if (ntFieldName == m_ntType)
                        {
                            ++sBegin;
                            m_ntType = ntFieldValue;
                            continue;
                        }

                        break;
                    default:
                        m_iReturnCount = 0;

                        break;
                    }

                    switch (m_ntType)
                    {
                    case ntFirstLine:
                        m_sFirstLine.push_back(*sBegin);
                        break;
                    case ntFieldName:
                        m_sFieldName.push_back(*sBegin);
                        break;
                    case ntFieldValue:
                        m_sFieldValue.push_back(*sBegin);
                        break;
                    }

                    ++sBegin;
                }

                DoOnData();

                return sBegin - p_sData;
            }

            bool DoClose()
            {
                if (!DoOnData())
                {
                    return false;
                }

                return NextClose();
            }

            bool DoPull()
            {
                if (!DoOnData())
                {
                    return false;
                }

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

                    bool Call(const CHttpBase &p_stHttp)
                    {
                        return m_onCallBack(p_stHttp);
                    }

                private:
                    COnCallback m_onCallBack;
                };

                m_spOnData->reset(new CCallback(p_onCallback));
            }

        private:
            struct IOnData
            {
                virtual bool Call(const CHttpBase &p_stHttp) = 0;
            };

            enum ENextType
            {
                ntFirstLine,
                ntFieldName,
                ntFieldValue,
                ntData
            };
            
            boost::shared_ptr<IOnData> m_spOnData;
            std::string m_sFirstLine, m_sFieldName, m_sFieldValue;
            CHttpHeader m_vHeader;
            CRingBuffer<c_iSize> m_rbData;
            ENextType m_ntType;
            std::size_t m_iReturnCount; //用于识别Http头的结束位置
            bool m_bNeedCallback;

            void Reset()
            {
                m_sFirstLine.clear();
                m_sFieldName.clear();
                m_sFieldValue.clear();
                m_vHeader.clear();
                m_rbData.Clear();
                m_ntType = ntFirstLine;
                m_iReturnCount = 0;
            }

            bool DoOnData()
            {
                if (m_bNeedCallback)
                {
                    //如果报文头还未读取完成，则不回调
                    if (ntData != m_ntType)
                    {
                        return true;
                    }

                    if (m_spOnData)
                    {
                        if (!m_spOnData->Call(*this))
                        {
                            return false;
                        }
                    }

                    m_bNeedCallback = false;
                }

                return true;
            }

        };
    }
}
