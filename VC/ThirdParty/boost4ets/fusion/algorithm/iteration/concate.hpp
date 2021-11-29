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

#include <boost/fusion/support/config.hpp>
#include <fusion/algorithm/iteration/detail/concate.hpp>

namespace ets { namespace fusion
{
    namespace result_of
    {
        template <typename Sequence, typename F>
        struct concate
        {
            typedef void type;
        };
    }

    template <typename Sequence, typename F>
    inline void
    concate(Sequence& seq, F const& f)
    {
        detail::concate(seq, f);
    }

    template <typename Sequence, typename F>
    inline void
    concate(Sequence const& seq, F const& f)
    {
        detail::concate(seq, f);
    }
}}
