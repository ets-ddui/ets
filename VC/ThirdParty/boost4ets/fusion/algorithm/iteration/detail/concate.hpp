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
#include <boost/fusion/sequence/intrinsic/begin.hpp>
#include <boost/fusion/sequence/intrinsic/end.hpp>
#include <boost/fusion/iterator/equal_to.hpp>
#include <boost/fusion/iterator/next.hpp>
#include <boost/fusion/iterator/deref.hpp>
#include <boost/fusion/iterator/distance.hpp>
#include <boost/fusion/support/category_of.hpp>
#include <boost/mpl/bool.hpp>

namespace ets { namespace fusion {
namespace detail
{
    template <typename First, typename Next, typename Last, typename F>
    inline void
    concate_linear(First const& first, Next const& next, Last const& last, F const& f)
    {
        f(*first, *next);
        detail::concate_linear(next, boost::fusion::next(next), last, f);
    }

    template <typename First, typename Last, typename F>
    inline void
    concate_linear(First const& first, Last const& next, Last const& last, F const& f)
    {
    }

    template <typename Sequence, typename F>
    inline void
    concate(Sequence& seq, F const& f)
    {
        auto first = boost::fusion::begin(seq);
        auto next = boost::fusion::next(first);
        auto end = boost::fusion::end(seq);

        detail::concate_linear(first, next, end, f);
    }
}}}
