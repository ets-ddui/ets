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
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include "../Buffer.h"

namespace ets
{
    namespace io
    {
        enum ESocketState {ssNotConnect, ssError, ssOK};

        //CTCPSocketBase支持多个生产者，一个消费者的模型，数据收发本质上是单线程处理的，
        //在CTCPSocketBase中，包含一个内部缓存，用于缓存socket的发送数据(收到的数据是即时处理的)
        template<int c_iWriteSize, int c_iReadSize>
        class CTcpHandler
            : public CHandler
        {
        public:
            explicit CTcpHandler(boost::asio::io_service &p_iosService)
                : m_socSocket(p_iosService), m_ssState(ssNotConnect),
                m_bWritting(false), m_bReading(false)
            {
            }

            const ESocketState GetState() const
            {
                return m_ssState;
            }

            void Connect(const boost::asio::ip::tcp::resolver::query &p_qryAddress)
            {
                if (m_socSocket.get_io_service().stopped())
                {
                    return;
                }

                boost::shared_ptr<boost::asio::ip::tcp::resolver> res(new boost::asio::ip::tcp::resolver(m_socSocket.get_io_service()));
                res->async_resolve(p_qryAddress,
                    boost::bind(&CTcpHandler::DoResolve, this,
                        boost::asio::placeholders::error, boost::asio::placeholders::iterator, res));
            }

            void Close()
            {
                boost::system::error_code ecError;
                m_socSocket.close(ecError);
            }

            void Accept(const boost::shared_ptr<boost::asio::ip::tcp::acceptor> &p_socAcceptor)
            {
                if (p_socAcceptor->get_io_service().stopped())
                {
                    return;
                }

                p_socAcceptor->async_accept(m_socSocket,
                    boost::bind(&CTcpHandler::DoAccept, this,
                        boost::asio::placeholders::error, p_socAcceptor));
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
                        m_socSocket.get_io_service().run_one();
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
                m_socSocket.shutdown(boost::asio::socket_base::shutdown_send, ec);
                return !ec;
            }

            bool DoPull()
            {
                DoOnData();

                /* TODO: 要考虑半关的场景 */
                if (!m_socSocket.is_open() && m_rbReadBuffer.IsEmpty())
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

            boost::asio::ip::tcp::socket m_socSocket;
            boost::mutex m_muLock;
            ESocketState m_ssState;
            CRingBuffer<c_iWriteSize> m_rbWriteBuffer;
            CRingBuffer<c_iReadSize> m_rbReadBuffer;
            bool m_bWritting, m_bReading;
            boost::shared_ptr<IOnData> m_spOnData;

            void InnerWrite()
            {
                if (m_bWritting)
                {
                    return;
                }

                /* TODO: 要考虑半关的场景 */
                if (!m_socSocket.is_open())
                {
                    return;
                }

                std::vector<boost::asio::const_buffer> vData(m_rbWriteBuffer.GetDate());
                if (vData.empty())
                {
                    PreviousPull();
                    return;
                }

                m_socSocket.async_write_some(vData,
                    boost::bind(&CTcpHandler::DoWrite, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

                m_bWritting = true;
            }

            void DoWrite(const boost::system::error_code &p_eError, std::size_t p_iSize)
            {
                m_bWritting = false;

                if (p_eError)
                {
                    m_ssState = ssError;
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

                /* TODO: 要考虑半关的场景 */
                if (!m_socSocket.is_open())
                {
                    return;
                }

                std::vector<boost::asio::mutable_buffer> vBuffer(m_rbReadBuffer.GetBuffer());
                if (vBuffer.empty())
                {
                    return;
                }

                m_socSocket.async_read_some(vBuffer,
                    boost::bind(&CTcpHandler::DoRead, this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));

                m_bReading = true;
            }

            void DoRead(const boost::system::error_code &p_eError, std::size_t p_iSize)
            {
                m_bReading = false;

                if (p_eError)
                {
                    m_ssState = ssError;
                    return;
                }

                m_rbReadBuffer.Append(p_iSize);
                InnerRead();

                DoOnData();
            }

            void DoAccept(const boost::system::error_code &p_eError,
                const boost::shared_ptr<boost::asio::ip::tcp::acceptor> &p_socAcceptor)
            {
                boost::shared_ptr<CTcpHandler> socConnection(new CTcpHandler(p_socAcceptor->get_io_service()));
                socConnection->Accept(p_socAcceptor);

                if (p_eError)
                {
                    m_ssState = ssError;
                    return;
                }

                InnerWrite();
                InnerRead();

                m_ssState = ssOK;
            }

            void DoResolve(const boost::system::error_code &p_eError,
                boost::asio::ip::tcp::resolver::iterator p_itEndPoint,
                boost::shared_ptr<boost::asio::ip::tcp::resolver> /*p_resResolver*/)
            {
                if (p_eError)
                {
                    m_ssState = ssError;
                    return;
                }

                boost::asio::async_connect(m_socSocket, p_itEndPoint,
                    boost::bind(&CTcpHandler::DoConnected, this,
                        boost::asio::placeholders::error));
            }

            void DoConnected(const boost::system::error_code &p_eError)
            {
                if (p_eError)
                {
                    m_ssState = ssError;
                    return;
                }

                InnerWrite();
                InnerRead();

                m_ssState = ssOK;
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

        typedef CTcpHandler<4096, 512> CTcpSocket;
    }
}
