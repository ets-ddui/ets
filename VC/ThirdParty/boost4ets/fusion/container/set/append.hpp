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
