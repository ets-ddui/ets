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
namespace ets { namespace fusion { namespace detail
{
    template <>
    struct append_set<1>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0;
            typedef boost::fusion::set<T0, Element> type;
        };
    };
    template <>
    struct append_set<2>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1;
            typedef boost::fusion::set<T0 , T1, Element> type;
        };
    };
    template <>
    struct append_set<3>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2;
            typedef boost::fusion::set<T0 , T1 , T2, Element> type;
        };
    };
    template <>
    struct append_set<4>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3;
            typedef boost::fusion::set<T0 , T1 , T2 , T3, Element> type;
        };
    };
    template <>
    struct append_set<5>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4, Element> type;
        };
    };
    template <>
    struct append_set<6>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5, Element> type;
        };
    };
    template <>
    struct append_set<7>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6, Element> type;
        };
    };
    template <>
    struct append_set<8>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7, Element> type;
        };
    };
    template <>
    struct append_set<9>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8, Element> type;
        };
    };
    template <>
    struct append_set<10>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9, Element> type;
        };
    };
    template <>
    struct append_set<11>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10, Element> type;
        };
    };
    template <>
    struct append_set<12>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11, Element> type;
        };
    };
    template <>
    struct append_set<13>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12, Element> type;
        };
    };
    template <>
    struct append_set<14>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13; typedef typename boost::fusion::result_of::next<I13>::type I14;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12; typedef typename boost::fusion::result_of::value_of<I13>::type T13;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13, Element> type;
        };
    };
    template <>
    struct append_set<15>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13; typedef typename boost::fusion::result_of::next<I13>::type I14; typedef typename boost::fusion::result_of::next<I14>::type I15;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12; typedef typename boost::fusion::result_of::value_of<I13>::type T13; typedef typename boost::fusion::result_of::value_of<I14>::type T14;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14, Element> type;
        };
    };
    template <>
    struct append_set<16>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13; typedef typename boost::fusion::result_of::next<I13>::type I14; typedef typename boost::fusion::result_of::next<I14>::type I15; typedef typename boost::fusion::result_of::next<I15>::type I16;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12; typedef typename boost::fusion::result_of::value_of<I13>::type T13; typedef typename boost::fusion::result_of::value_of<I14>::type T14; typedef typename boost::fusion::result_of::value_of<I15>::type T15;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15, Element> type;
        };
    };
    template <>
    struct append_set<17>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13; typedef typename boost::fusion::result_of::next<I13>::type I14; typedef typename boost::fusion::result_of::next<I14>::type I15; typedef typename boost::fusion::result_of::next<I15>::type I16; typedef typename boost::fusion::result_of::next<I16>::type I17;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12; typedef typename boost::fusion::result_of::value_of<I13>::type T13; typedef typename boost::fusion::result_of::value_of<I14>::type T14; typedef typename boost::fusion::result_of::value_of<I15>::type T15; typedef typename boost::fusion::result_of::value_of<I16>::type T16;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16, Element> type;
        };
    };
    template <>
    struct append_set<18>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13; typedef typename boost::fusion::result_of::next<I13>::type I14; typedef typename boost::fusion::result_of::next<I14>::type I15; typedef typename boost::fusion::result_of::next<I15>::type I16; typedef typename boost::fusion::result_of::next<I16>::type I17; typedef typename boost::fusion::result_of::next<I17>::type I18;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12; typedef typename boost::fusion::result_of::value_of<I13>::type T13; typedef typename boost::fusion::result_of::value_of<I14>::type T14; typedef typename boost::fusion::result_of::value_of<I15>::type T15; typedef typename boost::fusion::result_of::value_of<I16>::type T16; typedef typename boost::fusion::result_of::value_of<I17>::type T17;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17, Element> type;
        };
    };
    template <>
    struct append_set<19>
    {
        template <typename I0, typename Element>
        struct apply
        {
            typedef typename boost::fusion::result_of::next<I0>::type I1; typedef typename boost::fusion::result_of::next<I1>::type I2; typedef typename boost::fusion::result_of::next<I2>::type I3; typedef typename boost::fusion::result_of::next<I3>::type I4; typedef typename boost::fusion::result_of::next<I4>::type I5; typedef typename boost::fusion::result_of::next<I5>::type I6; typedef typename boost::fusion::result_of::next<I6>::type I7; typedef typename boost::fusion::result_of::next<I7>::type I8; typedef typename boost::fusion::result_of::next<I8>::type I9; typedef typename boost::fusion::result_of::next<I9>::type I10; typedef typename boost::fusion::result_of::next<I10>::type I11; typedef typename boost::fusion::result_of::next<I11>::type I12; typedef typename boost::fusion::result_of::next<I12>::type I13; typedef typename boost::fusion::result_of::next<I13>::type I14; typedef typename boost::fusion::result_of::next<I14>::type I15; typedef typename boost::fusion::result_of::next<I15>::type I16; typedef typename boost::fusion::result_of::next<I16>::type I17; typedef typename boost::fusion::result_of::next<I17>::type I18; typedef typename boost::fusion::result_of::next<I18>::type I19;
            typedef typename boost::fusion::result_of::value_of<I0>::type T0; typedef typename boost::fusion::result_of::value_of<I1>::type T1; typedef typename boost::fusion::result_of::value_of<I2>::type T2; typedef typename boost::fusion::result_of::value_of<I3>::type T3; typedef typename boost::fusion::result_of::value_of<I4>::type T4; typedef typename boost::fusion::result_of::value_of<I5>::type T5; typedef typename boost::fusion::result_of::value_of<I6>::type T6; typedef typename boost::fusion::result_of::value_of<I7>::type T7; typedef typename boost::fusion::result_of::value_of<I8>::type T8; typedef typename boost::fusion::result_of::value_of<I9>::type T9; typedef typename boost::fusion::result_of::value_of<I10>::type T10; typedef typename boost::fusion::result_of::value_of<I11>::type T11; typedef typename boost::fusion::result_of::value_of<I12>::type T12; typedef typename boost::fusion::result_of::value_of<I13>::type T13; typedef typename boost::fusion::result_of::value_of<I14>::type T14; typedef typename boost::fusion::result_of::value_of<I15>::type T15; typedef typename boost::fusion::result_of::value_of<I16>::type T16; typedef typename boost::fusion::result_of::value_of<I17>::type T17; typedef typename boost::fusion::result_of::value_of<I18>::type T18;
            typedef boost::fusion::set<T0 , T1 , T2 , T3 , T4 , T5 , T6 , T7 , T8 , T9 , T10 , T11 , T12 , T13 , T14 , T15 , T16 , T17 , T18, Element> type;
        };
    };
}}}
