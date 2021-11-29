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

#include <boost/bind.hpp>

namespace ets
{

    // 0

    template<class R, class T>
        auto auto_bind(R (T::*f) (), T *t)
        -> decltype(boost::bind(f, t))
    {
        return boost::bind(f, t);
    }

    template<class R, class T>
        auto auto_bind(R (T::*f) () const, T *t)
        -> decltype(boost::bind(f, t))
    {
        return boost::bind(f, t);
    }

    // 1

    template<class R, class T,
        class A1>
        auto auto_bind(R (T::*f) (A1), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1))
    {
        return boost::bind(f, t, boost::placeholders::_1);
    }

    template<class R, class T,
        class A1>
        auto auto_bind(R (T::*f) (A1) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1))
    {
        return boost::bind(f, t, boost::placeholders::_1);
    }

    // 2

    template<class R, class T,
        class A1, class A2>
        auto auto_bind(R (T::*f) (A1, A2), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2);
    }

    template<class R, class T,
        class A1, class A2>
        auto auto_bind(R (T::*f) (A1, A2) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2);
    }

    // 3

    template<class R, class T,
        class A1, class A2, class A3>
        auto auto_bind(R (T::*f) (A1, A2, A3), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3);
    }

    template<class R, class T,
        class A1, class A2, class A3>
        auto auto_bind(R (T::*f) (A1, A2, A3) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3);
    }

    // 4

    template<class R, class T,
        class A1, class A2, class A3, class A4>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4);
    }

    template<class R, class T,
        class A1, class A2, class A3, class A4>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4);
    }

    // 5

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5);
    }

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5);
    }

    // 6

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6);
    }

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6);
    }

    // 7

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6, class A7>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6, A7), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7);
    }

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6, class A7>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6, A7) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7);
    }

    // 8

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6, A7, A8), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8);
    }

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6, A7, A8) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8);
    }

    // 9

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6, A7, A8, A9), T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8, boost::placeholders::_9))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8, boost::placeholders::_9);
    }

    template<class R, class T,
        class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
        auto auto_bind(R (T::*f) (A1, A2, A3, A4, A5, A6, A7, A8, A9) const, T *t)
        -> decltype(boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8, boost::placeholders::_9))
    {
        return boost::bind(f, t, boost::placeholders::_1, boost::placeholders::_2, boost::placeholders::_3, boost::placeholders::_4, boost::placeholders::_5, boost::placeholders::_6, boost::placeholders::_7, boost::placeholders::_8, boost::placeholders::_9);
    }

} // namespace ets
