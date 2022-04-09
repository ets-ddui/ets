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

#include <stdio.h>

namespace vcl4c
{
    namespace other
    {
        //array_auto_ptr�ǶԱ�׼���е�auto_ptr����չ�����ڱ�׼���е�auto_ptrֻ�ܴ����������޷��������飬
        //��ˣ�����array_auto_ptr�࣬���ڶ��������Ĺ����������Ĳ����������ʱ�ͷ��ڴ�ķ�ʽ�ϲ�һ��
        template<class _Ty>
        class array_auto_ptr
        {
        public:
            explicit array_auto_ptr(_Ty *p_Value = NULL)
                : m_Value(p_Value)
            {
            }
            array_auto_ptr(array_auto_ptr<_Ty> &p_Value)
                : m_Value(p_Value.release())
            {
            }
            virtual ~array_auto_ptr(void)
            {
                delete []m_Value;
            }
            array_auto_ptr<_Ty> &operator=(array_auto_ptr<_Ty> &p_Value)
            {
                reset(p_Value.release());
                return *this;
            }
            _Ty &operator[](int p_iIndex) const
            {
                return m_Value[p_iIndex];
            }
            bool operator!() const
            {
                return NULL == m_Value;
            }
            _Ty *get() const
            {
                return m_Value;
            }
            _Ty *release()
            {
                _Ty *ptTemp = m_Value;
                m_Value = NULL;
                return ptTemp;
            }
            void reset(_Ty *p_Value = NULL)
            {
                if(p_Value != m_Value)
                    delete []m_Value;
                m_Value = p_Value;
            }
        private:
            _Ty *m_Value;
        };
    }
}
