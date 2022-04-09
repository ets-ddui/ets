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
#pragma once

#include <boost/fusion/support/config.hpp>
#include <fusion/container/set/detail/append_set.hpp>
//#include <boost/fusion/container/set/detail/convert_impl.hpp>
#include <boost/fusion/container/set/set.hpp>
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/size.hpp>

namespace ets { namespace fusion
{
    namespace result_of
    {
        template <typename Sequence, typename Element>
        struct append_set
        {
            typedef typename boost::fusion::result_of::has_key<Sequence, Element> has_key;

            typedef typename detail::append_set<boost::fusion::result_of::size<Sequence>::value> gen;

            typedef typename std::conditional<
                has_key::value,
                Sequence,
                typename gen::template apply<typename boost::fusion::result_of::begin<Sequence>::type, Element>::type
            >::type type;
        };
    }
}}
