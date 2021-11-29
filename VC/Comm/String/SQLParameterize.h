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
#include <stdio.h>
#include <string>
#include "FormatString.h"

namespace vcl4c
{
    namespace string
    {
        class CSQLParameterize: public CFormatString<CFormatData>
        {
        private:
            enum{stQuotation = stType + 1};
            typedef CFormatString<CFormatData> CParentClass;
            bool m_bHasParameter;
        protected:
            CSQLParameterize(const char *p_sFormat)
                : CParentClass(p_sFormat), m_bHasParameter(false)
            {
            }

            virtual EState GetState(const EState p_stLastState)
            {
                char cValue = GetValue();
                if('\'' == cValue)
                {
                    return (EState)stQuotation;
                }
                else
                {
                    EState st = CParentClass::GetState(p_stLastState);
                    //�����һ���ַ��ǡ�%�����ҵ�ǰ�ַ�������ͨ�ַ�����˵���Ǳ�������Ŀ�ʼ
                    if(stPercent == p_stLastState && stNormal != st)
                        m_bHasParameter = true;

                    return st;
                }
            }

            int Format(va_list p_valParams)
            {
                return CParentClass::Format(p_valParams);
            }

            virtual bool DoCustomFormat(const EState p_stState, va_list &p_valParams) throw(...)
            {
                if(stQuotation != p_stState)
                    return false;

                m_lstData.push_back(CFormatData(NULL));
                MoveNext();
                return true;
            }

            virtual std::string ToString() const throw(...)
            {
                std::string strResult;

                //1.0 û���Զ����������ԭ��ʽ����SQL��
                if(!m_bHasParameter)
                {
                    for(std::list<CFormatData>::const_iterator it = m_lstData.begin(); it != m_lstData.end(); ++it)
                        if(dtCustom == it->GetDataType())
                            strResult.append(1, '\'');
                        else
                            strResult.append(it->ToString());

                    return strResult;
                }

                //2.0 ���Զ����������SQLת��Ϊ��������ʽ
                std::string strType, strValue;
                int iParamIndex = 0;

                strResult.append("exec sp_executesql N'");
                for(std::list<CFormatData>::const_iterator it = m_lstData.begin(); it != m_lstData.end(); ++it)
                {
                    switch(it->GetDataType())
                    {
                    case dtCustom:
                        {
                            //2.1 �����������Ƿ�����Զ������
                            bool bHasParam = false, bNotChar = false;
                            std::list<CFormatData>::const_iterator itInner = ++it;
                            for(; itInner != m_lstData.end(); ++itInner)
                            {
                                if(dtCustom == itInner->GetDataType())
                                    break;
                                else if(dtText == itInner->GetDataType())
                                {
                                    bNotChar = true;
                                    continue;
                                }
                                else
                                {
                                    if(dtChar != itInner->GetDataType())
                                        bNotChar = true;
                                    bHasParam = true; //�ҵ������󣬼������ҽ�β�ġ�'��
                                }
                            }

                            if(itInner == m_lstData.end())
                                throw "��ʽ����ȷ";

                            //2.2 ��װSQL���
                            if(bHasParam) //���Զ������
                            {
                                //2.2.1 �滻����"@P<Index>"
                                strResult.append("@P");
                                strResult.append(CFormatData::Int64ToString(++iParamIndex));

                                //2.2.2 �������ֵ�б�"<Param1>,<Param2>,..."
                                if(!strValue.empty())
                                    strValue.append(",");
                                strValue.append("'");
                                int iSize = strValue.size();
                                for(; it != itInner; ++it)
                                    strValue.append(it->ToString());
                                iSize = strValue.size() - iSize;
                                iSize = (iSize + 15) & ~0xF; //�ַ�����16�ı������㳤��(���ٲ�ͬ����ֵ֮��Ĳ���)
                                iSize = iSize <= 0 ? 16 : iSize;
                                strValue.append("'");

                                //2.2.3 ����������Ͷ����б�"@P<Index> [varchar(<Size>)|char],@P<Index> [varchar(<Size>)|char],..."
                                if(!strType.empty())
                                    strType.append(",");
                                if(bNotChar)
                                {
                                    strType.append("@P");
                                    strType.append(CFormatData::Int64ToString(iParamIndex));
                                    strType.append(" varchar(");
                                    strType.append(CFormatData::Int64ToString(iSize));
                                    strType.append(")");
                                }
                                else
                                {
                                    strType.append("@P");
                                    strType.append(CFormatData::Int64ToString(iParamIndex));
                                    strType.append(" char");
                                }
                            }
                            else //û���Զ������
                            {
                                strResult.append("''");
                                for(; it != itInner; ++it)
                                    strResult.append(it->ToString());
                                strResult.append("''");
                            }
                        }

                        break;
                    case dtText:
                        strResult.append(it->ToString());
                        break;
                    default:
                        {
                            //dtChar, dtWideChar, dtString, dtWideString, dtDouble, dtInt64
                            EDataType dt = it->GetDataType();

                            strResult.append("@P");
                            strResult.append(CFormatData::Int64ToString(++iParamIndex));

                            if(!strValue.empty())
                                strValue.append(",");
                            if(dtChar == dt || dtWideChar == dt || dtString == dt || dtWideString == dt)
                                strValue.append("'");
                            int iSize = strValue.size();
                            strValue.append(it->ToString());
                            iSize = strValue.size() - iSize;
                            iSize = (iSize + 15) & ~0xF; //�ַ�����16�ı������㳤��(���ٲ�ͬ����ֵ֮��Ĳ���)
                            iSize = iSize <= 0 ? 16 : iSize;
                            if(dtChar == dt || dtWideChar == dt || dtString == dt || dtWideString == dt)
                                strValue.append("'");

                            if(!strType.empty())
                                strType.append(",");
                            if(dtChar == dt)
                            {
                                strType.append("@P");
                                strType.append(CFormatData::Int64ToString(iParamIndex));
                                strType.append(" char");
                            }
                            else if(dtWideChar == dt || dtString == dt || dtWideString == dt)
                            {
                                strType.append("@P");
                                strType.append(CFormatData::Int64ToString(iParamIndex));
                                strType.append(" varchar(");
                                strType.append(CFormatData::Int64ToString(iSize));
                                strType.append(")");
                            }
                            else if(dtInt64 == dt)
                            {
                                strType.append("@P");
                                strType.append(CFormatData::Int64ToString(iParamIndex));
                                strType.append(" bigint");
                            }
                            else //dtDouble
                            {
                                strType.append("@P");
                                strType.append(CFormatData::Int64ToString(iParamIndex));
                                strType.append(" decimal(38,");
                                strType.append(CFormatData::Int64ToString(it->GetPrecision()));
                                strType.append(")");
                            }
                        }
                    }
                }
                strResult.append("', N'");
                strResult.append(strType);
                strResult.append("', ");
                strResult.append(strValue);

                return strResult;
            }
        public:
            static const std::string Format(const char *p_sFormat, ...)
            {
                va_list valParams = NULL;
                va_start(valParams, p_sFormat);

                return Format(p_sFormat, valParams);
            }

            static const std::string Format(const char *p_sFormat, va_list p_valParams) throw(...)
            {
                CSQLParameterize fs(p_sFormat);

                if(0 != fs.Format(p_valParams))
                    throw "��ʽ��ʧ��";

                return fs.ToString();
            }
        };
    }
}