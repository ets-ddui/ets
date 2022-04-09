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

#include "../ThirdParty/dbf/TDBF.h"
#include "DataSet.h"

namespace vcl4c
{
    namespace db
    {
        class CDbfDataSet
            : public CDataSetBase
        {
        public:
            CDbfDataSet()
                : m_dbfData(), m_iRecordRec(0)
            {
            }

            void LoadFromFile(const std::string & p_sFileName, int p_iMode = TDBF::ReadOnly | TDBF::ShareOpen) throw(...)
            {
                m_dbfData.Close();
                m_dbfData.Open(p_sFileName.c_str(), p_iMode);

                ClearField();
                for (unsigned int i = 0; i < m_dbfData.GetFieldNumber(); ++i)
                {
                    const TDBFField * fld = m_dbfData.GetFieldInfo(i);

                    //m_dbfData.GetFieldNumber()���ص��ֶ������ܱ�ʵ�ʵĶ࣬ͨ���ж��ֶγ�����ʶ��
                    if (0 == fld->Width)
                    {
                        break;
                    }

                    switch (fld->Type)
                    {
                    case 'C': //�ַ���
                    case 'N': //��ֵ��
                    case 'F': //Float
                    case 'L': //�߼�
                        AddField(fld->Name, fld->Width + 1);
                        break;
                    case 'Y': //������
                    case 'B': //Double
                        AddField(fld->Name, 21);
                        break;
                    case 'D': //Date
                        AddField(fld->Name, 11);
                        break;
                    case 'T': //DateTime
                        AddField(fld->Name, 23);
                        break;
                    case 'I': //����
                        AddField(fld->Name, 11);
                        break;
                    default:
                        throw std::runtime_error("��֧�ֶ��������͵��ֶ�");
                    }
                }
            }

        protected:
            //CBase�ӿ�ʵ��
            virtual std::string GetString(const int p_iIndex) const throw(...)
            {
                if (0 > p_iIndex || (int)GetFieldCount() <= p_iIndex)
                {
                    throw std::out_of_range("�����������Ϸ�");
                }

                char sBuffer[256];
                m_dbfData.GetField(p_iIndex, sBuffer, sizeof(sBuffer));
                if (!m_dbfData)
                {
                    return "";
                }

                return std::string(sBuffer);
            }

            virtual void SetString(const int p_iIndex, const std::string & p_sValue) throw(...)
            {
                m_dbfData.SetField(p_iIndex, p_sValue.c_str());
                if (!m_dbfData)
                {
                    throw std::out_of_range("�����������Ϸ�");
                }
            }

            virtual BYTE GetFlag(const int p_iIndex) const
            {
                return 0;
            }

            virtual void SetFlag(const int p_iIndex, const BYTE p_iFlag)
            {
                //��ʵ��
            }

            virtual char * GetAddress(const int p_iIndex) const
            {
                return NULL;
            }

        public:
            //CDataSetBase�ӿ�ʵ��
            virtual unsigned int GetRecordCount() const
            {
                return const_cast<TDBF *>(&m_dbfData)->GetRecordNumber();
            }

            virtual void First()
            {
                m_iRecordRec = 0;
                Next();
            }

            virtual void Next()
            {
                ++m_iRecordRec;

                if (!IsEof())
                {
                    m_dbfData.ReadRecord(m_iRecordRec);
                }
            }

            virtual bool IsEof() const
            {
                return m_iRecordRec > GetRecordCount();
            }

            virtual void EmptyData()
            {
                m_dbfData.Zap();
            }

            virtual void Append()
            {
                m_dbfData.ClearRecordBuffer();
                m_iRecordRec = GetRecordCount() + 1;
            }

            virtual void Post()
            {
                if (IsEof())
                {
                    m_dbfData.AppendRecord();
                }
                else
                {
                    m_dbfData.WriteRecord(m_iRecordRec);
                }
            }

        private:
            mutable TDBF m_dbfData;
            unsigned int m_iRecordRec;

        };
    }
}
