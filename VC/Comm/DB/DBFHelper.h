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

                    //m_dbfData.GetFieldNumber()返回的字段数可能比实际的多，通过判断字段长度来识别
                    if (0 == fld->Width)
                    {
                        break;
                    }

                    switch (fld->Type)
                    {
                    case 'C': //字符型
                    case 'N': //数值型
                    case 'F': //Float
                    case 'L': //逻辑
                        AddField(fld->Name, fld->Width + 1);
                        break;
                    case 'Y': //货币型
                    case 'B': //Double
                        AddField(fld->Name, 21);
                        break;
                    case 'D': //Date
                        AddField(fld->Name, 11);
                        break;
                    case 'T': //DateTime
                        AddField(fld->Name, 23);
                        break;
                    case 'I': //整型
                        AddField(fld->Name, 11);
                        break;
                    default:
                        throw std::runtime_error("不支持二进制类型的字段");
                    }
                }
            }

        protected:
            //CBase接口实现
            virtual std::string GetString(const int p_iIndex) const throw(...)
            {
                if (0 > p_iIndex || (int)GetFieldCount() <= p_iIndex)
                {
                    throw std::out_of_range("输入索引不合法");
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
                    throw std::out_of_range("输入索引不合法");
                }
            }

            virtual BYTE GetFlag(const int p_iIndex) const
            {
                return 0;
            }

            virtual void SetFlag(const int p_iIndex, const BYTE p_iFlag)
            {
                //空实现
            }

            virtual char * GetAddress(const int p_iIndex) const
            {
                return NULL;
            }

        public:
            //CDataSetBase接口实现
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
