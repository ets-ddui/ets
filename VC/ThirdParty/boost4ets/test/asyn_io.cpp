/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)������չ���߼���

    ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
    �����˿��Ŀ����ϣ�������ã��������κα�֤��
    ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

    ��Դ��ַ: https://github.com/ets-ddui/ets
              https://gitee.com/ets-ddui/ets
    ��ԴЭ��: The MIT License (MIT)
    ��������: xinghun87@163.com
    �ٷ����ͣ�https://blog.csdn.net/xinghun61
*/
#define BOOST_ASIO_HAS_MOVE 1 //boost::asio����ֵ���õİ汾ʶ���ƺ������⣬VC2010����֧�֣������ֶ�����

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
