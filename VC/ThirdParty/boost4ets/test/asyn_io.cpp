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
#define BOOST_ASIO_HAS_MOVE 1 //boost::asio对右值引用的版本识别似乎有问题，VC2010好像支持，这里手动开启

#include <boost/asio.hpp>
#include <process/async_pipe.hpp>
#include <process/child.hpp>
#include <process/search_path.hpp>
#include <process/io.hpp>

int main(int argc, char* argv[])
{
    boost::asio::io_service ios;
    std::vector<char> buf(100);

    ets::process::async_pipe ap(ios);

    ets::process::child c(ets::process::search_path("cmd"), "/?", ets::process::std_out > ap);

    boost::asio::async_read(ap, boost::asio::buffer(buf),
        [&buf](const boost::system::error_code &ec, std::size_t size){
            std::cout << std::string(buf.begin(), buf.begin() + size);
        });

    ios.run();
    c.wait();
    int result = c.exit_code();

    return result;
}
