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
#if FUSION_MAX_SET_SIZE <= 10
#include <fusion/container/set/detail/cpp03/preprocessed/append_set10.hpp>
#elif FUSION_MAX_SET_SIZE <= 20
#include <fusion/container/set/detail/cpp03/preprocessed/append_set20.hpp>
#elif FUSION_MAX_SET_SIZE <= 30
#include <fusion/container/set/detail/cpp03/preprocessed/append_set30.hpp>
#elif FUSION_MAX_SET_SIZE <= 40
#include <fusion/container/set/detail/cpp03/preprocessed/append_set40.hpp>
#elif FUSION_MAX_SET_SIZE <= 50
#include <fusion/container/set/detail/cpp03/preprocessed/append_set50.hpp>
#else
#error "FUSION_MAX_SET_SIZE out of bounds for preprocessed headers"
#endif
