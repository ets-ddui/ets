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

        //CTCPSocketBase֧�ֶ�������ߣ�һ�������ߵ�ģ�ͣ������շ��������ǵ��̴߳���ģ�
        //��CTCPSocketBase�У�����һ���ڲ����棬���ڻ���socket�ķ�������(�յ��������Ǽ�ʱ�����)
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

                    /* TODO: ���߳���δ��� */
                    if (p_iLen > 0)
                    {
                        m_socSocket.get_io_service().run_one();
                    }
                }
            }

        public:
            //������ӿ�ʵ��
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

                /* TODO: Ҫ���ǰ�صĳ��� */
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

                /* TODO: Ҫ���ǰ�صĳ��� */
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

                /* TODO: Ҫ���ǰ�صĳ��� */
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
