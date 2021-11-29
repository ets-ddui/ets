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

#include <stdio.h>

namespace vcl4c
{
    namespace other
    {
        //array_auto_ptr是对标准库中的auto_ptr的扩展，由于标准库中的auto_ptr只能处理单个对象，无法处理数组，
        //因此，增加array_auto_ptr类，用于对数组对象的管理，两者最大的差异仅在析构时释放内存的方式上不一样
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
