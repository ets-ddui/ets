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

#include "resource.h"
#include "IDL/IScript.h"

namespace ets
{
    namespace v8
    {
        class CV8:
            public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
            public IScript
        {
            BEGIN_DEFINE_MAP(CV8)
                SIMPLE_INTERFACE(IDispatch)
                SIMPLE_INTERFACE(IScript)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

            enum {IDL = IDTL_SCRIPT};

        public:
            CV8()
                : CDispatch(IDL, &__uuidof(IScript), NULL)
            {
            }

            virtual ~CV8()
            {
            }

            HRESULT Init()
            {
                return S_OK;
            }

        public:
            //IScriptʵ��
            STDMETHOD(RegContainer)(IDispatch* p_itfContainer)
            {
                // TODO: �ڴ����ʵ�ִ���

                return S_OK;
            }

            STDMETHOD(RegFrame)(IDispatch* p_itfFrame)
            {
                // TODO: �ڴ����ʵ�ִ���

                return S_OK;
            }

            STDMETHOD(RunModule)(BSTR p_sFileName, BSTR p_sEntryFunction)
            {
                // TODO: �ڴ����ʵ�ִ���

                return S_OK;
            }

            STDMETHOD(RunCode)(BSTR p_sCode)
            {
                // TODO: �ڴ����ʵ�ִ���

                return S_OK;
            }

        };

    }
}
