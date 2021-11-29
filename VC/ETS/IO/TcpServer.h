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
#include "IDL/ITCPServer.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "Handler/TcpHandler.h"

namespace ets
{
    namespace io
    {
        class CTcpServer:
            public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
            public ITcpServer
        {
            BEGIN_DEFINE_MAP(CTcpServer)
                SIMPLE_INTERFACE(IDispatch)
                SIMPLE_INTERFACE(ITcpServer)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

            enum {IDL = IDTL_TCPSERVER};

        public:
            CTcpServer(boost::asio::io_service &p_iosService, const std::string &p_sHost, const std::string &p_sService)
                : CDispatch(IDL, &__uuidof(ITcpServer), NULL),
                m_socAcceptor(new boost::asio::ip::tcp::acceptor(p_iosService)), m_sHost(p_sHost), m_sService(p_sService)
            {
            }

            virtual ~CTcpServer()
            {
            }

            HRESULT Init()
            {
                //1.0 解析地址
                boost::asio::ip::tcp::resolver res(m_socAcceptor->get_io_service());
                boost::asio::ip::tcp::resolver::query qry(m_sHost, m_sService);

                boost::system::error_code ec;
                boost::asio::ip::tcp::endpoint ep = *res.resolve(qry, ec);
                if (ec)
                {
                    return E_INVALIDARG;
                }

                //2.0 初始化监听socket
                m_socAcceptor->open(ep.protocol(), ec);
                if (ec)
                {
                    return E_INVALIDARG;
                }

                m_socAcceptor->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
                if (ec)
                {
                    return E_INVALIDARG;
                }

                m_socAcceptor->bind(ep, ec);
                if (ec)
                {
                    return E_INVALIDARG;
                }

                m_socAcceptor->listen(boost::asio::socket_base::max_connections, ec);
                if (ec)
                {
                    return E_INVALIDARG;
                }

                //3.0 监听连接
                boost::shared_ptr<ets::io::CTcpSocket> socConnection(new ets::io::CTcpSocket(m_socAcceptor->get_io_service()));
                socConnection->Accept(m_socAcceptor);

                return S_OK;
            }

        public:
            //ITCPServer实现

        private:
            std::string m_sHost, m_sService;
            boost::shared_ptr<boost::asio::ip::tcp::acceptor> m_socAcceptor;

        };
    }
}
