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
#include "../Buffer.h"
#include <boost/asio.hpp>
#ifndef BOOST_NO_CXX11_NOEXCEPT
#include <boost/process/async_pipe.hpp>
#include <boost/process/child.hpp>
#include <boost/process/io.hpp>
#include <boost/process/env.hpp>
#include <boost/process/windows.hpp>
#include <boost/process/start_dir.hpp>
#else
#include <process/async_pipe.hpp>
#include <process/child.hpp>
#include <process/io.hpp>
#include <process/env.hpp>
#include <process/windows.hpp>
#include <process/start_dir.hpp>
#endif
#include "StringHelper.h"
#include <map>

namespace ets
{
    namespace io
    {
#ifndef BOOST_NO_CXX11_NOEXCEPT
#define ETS_PREFIX boost
#else
#define ETS_PREFIX ets
#endif

        class CPsHandler
            : public CHandler
        {
        public:
            explicit CPsHandler(boost::asio::io_service &p_iosService)
                : m_apWrite(p_iosService), m_apRead(p_iosService),
                m_bWritting(false), m_bReading(false)
            {
            }

            CPsHandler(CPsHandler &&p_phHandler)
                : m_apWrite(std::move(p_phHandler.m_apWrite)), m_apRead(std::move(p_phHandler.m_apRead)),
                m_spChild(std::move(p_phHandler.m_spChild)),
                m_bWritting(false), m_bReading(false) //这里不拷贝缓存数据(此构造函数一般只在初始化时使用，若已触发读写操作，m_apPipe上绑定的回调事件无法拷贝，这时的构造动作会引发问题)
            {
            }

            ~CPsHandler()
            {
                if (m_spChild && m_spChild->joinable())
                {
                    m_spChild->terminate();
                    m_spChild.reset();
                }
            }

            bool Start(const std::string &p_sExecute, const std::string &p_sWorkDir, const std::string &p_sEnvironment)
            {
                if (m_spChild && m_spChild->joinable())
                {
                    m_spChild->wait();
                    m_spChild.reset();
                }

                auto env = ETS_PREFIX::process::environment();
                if ("" != p_sEnvironment)
                {
                    std::map<std::string, std::string> mapEnv;
                    vcl4c::string::SplitString(mapEnv, p_sEnvironment, '=', '|');

                    for (auto it = mapEnv.begin(); it != mapEnv.end(); ++it)
                        env[it->first] = it->second;
                }
                else
                {
                    env = ETS_PREFIX::this_process::environment();
                }

                std::error_code ec;
                m_spChild.reset(new ETS_PREFIX::process::child(ec, env, p_sExecute,
                    ETS_PREFIX::process::windows::hide,
                    ETS_PREFIX::process::std_in < m_apWrite, //在child创建成功后，会调用on_success事件，而async_pipe_out、async_pipe_in会将另一个管道关闭，因此，需要将读写管道分开管理
                    ETS_PREFIX::process::std_out > m_apRead));
                if (ec)
                {
                    return false;
                }

                InnerWrite();
                InnerRead();

                return true;
            }

            void Write(const unsigned char *p_sData, std::size_t p_iLen)
            {
                while (p_iLen > 0)
                {
                    std::size_t iSize = DoPush(p_sData, p_iLen);
                    p_sData += iSize;
                    p_iLen -= iSize;

                    /* TODO: 多线程如何处理？ */
                    if (p_iLen > 0)
                    {
                        m_apWrite.sink().get_io_service().run_one();
                    }
                }
            }

        public:
            //组件链接口实现
            std::size_t DoPush(const unsigned char *p_sData, std::size_t p_iLen)
            {
                std::size_t iSize = m_rbWriteBuffer.Write(p_sData, p_iLen);
                if (0 < iSize)
                {
                    InnerWrite();
                }

                return iSize;
            }

            bool DoClose()
            {
                if (!m_rbWriteBuffer.IsEmpty())
                {
                    return false;
                }

                boost::system::error_code ec;
                m_apWrite.close(ec);
                return !ec;
            }

            bool DoPull()
            {
                DoOnData();

                if (!m_apRead.is_open() && m_rbReadBuffer.IsEmpty())
                {
                    return NextClose();
                }

                return true;
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

                    std::size_t Call(const unsigned char *p_sBuffer, const std::size_t p_iSize)
                    {
                        return m_onCallBack(p_sBuffer, p_iSize);
                    }

                private:
                    COnCallback m_onCallBack;
                };

                m_spOnData.reset(new CCallback(p_onCallback));
            }

        private:
            struct IOnData
            {
                virtual std::size_t Call(const unsigned char *p_sBuffer, const std::size_t p_iSize) = 0;
            };

            const static std::size_t c_iSize = 512;

            boost::shared_ptr<ETS_PREFIX::process::child> m_spChild;
            ETS_PREFIX::process::async_pipe m_apWrite;
            ETS_PREFIX::process::async_pipe m_apRead;
            CRingBuffer<c_iSize> m_rbWriteBuffer;
            CRingBuffer<c_iSize> m_rbReadBuffer;
            bool m_bWritting, m_bReading;
            boost::shared_ptr<IOnData> m_spOnData;

            void InnerWrite()
            {
                if (m_bWritting)
                {
                    return;
                }

                if (!m_apWrite.sink().is_open())
                {
                    return;
                }

                std::vector<boost::asio::const_buffer> vData(m_rbWriteBuffer.GetDate());
                if (vData.empty())
                {
                    PreviousPull();
                    return;
                }

                m_apWrite.async_write_some(vData,
                    boost::bind(&CPsHandler::DoWrite, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

                m_bWritting = true;
            }

            void DoWrite(const boost::system::error_code &p_eError, std::size_t p_iSize)
            {
                m_bWritting = false;

                if (p_eError)
                {
                    return;
                }

                m_rbWriteBuffer.Erase(p_iSize);
                InnerWrite();
            }

            void InnerRead()
            {
                if (m_bReading)
                {
                    return;
                }

                if (!m_apRead.source().is_open())
                {
                    return;
                }

                std::vector<boost::asio::mutable_buffer> vBuffer(m_rbReadBuffer.GetBuffer());
                if (vBuffer.empty())
                {
                    return;
                }

                m_apRead.async_read_some(vBuffer,
                    boost::bind(&CPsHandler::DoRead, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

                m_bReading = true;
            }

            void DoRead(const boost::system::error_code &p_eError, std::size_t p_iSize)
            {
                m_bReading = false;

                if (p_eError)
                {
                    return;
                }

                m_rbReadBuffer.Append(p_iSize);
                InnerRead();

                DoOnData();
            }

            bool DoOnData()
            {
                if (m_spOnData)
                {
                    std::size_t iSize = 0;
                    std::vector<boost::asio::const_buffer> vData(m_rbReadBuffer.GetDate());
                    for (auto it = vData.begin(); it != vData.end(); ++it)
                    {
                        std::size_t iRealSize = m_spOnData->Call(boost::asio::buffer_cast<const unsigned char *>(*it),
                            boost::asio::buffer_size(*it));
                        iSize += iRealSize;

                        if (iRealSize < boost::asio::buffer_size(*it))
                        {
                            break;
                        }
                    }

                    if (0 < iSize)
                    {
                        m_rbReadBuffer.Erase(iSize);
                        InnerRead();
                    }
                }

                return true;
            }

        };
    }
}
