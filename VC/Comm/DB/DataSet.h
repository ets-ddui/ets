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

#include <string>
#include <vector>
#include "../StringHelper.h"

namespace vcl4c
{
    namespace db
    {
        class CBase
        {
        public:
            enum EFieldType {INT_DATA, INT64_DATA, FLOAT_DATA, STRING_DATA, BINARY_DATA};
            enum {NULL_DATA = 0x1};

            class CFieldHelper
            {
            public:
                CFieldHelper(CBase * p_Parent, unsigned int p_iIndex)
                    : m_Parent(p_Parent), m_iIndex(p_iIndex)
                {
                }

                const std::string GetName() const
                {
                    return m_Parent->m_vFields[m_iIndex].m_sName;
                }

                void SetName(const std::string & p_sName)
                {
                    m_Parent->m_vFields[m_iIndex].m_sName = p_sName;
                }

                EFieldType GetType() const
                {
                    return m_Parent->m_vFields[m_iIndex].m_iType;
                }

                unsigned int GetOffset() const
                {
                    return m_Parent->m_vFields[m_iIndex].m_iOffset;
                }

                unsigned int GetSize() const
                {
                    return m_Parent->m_vFields[m_iIndex].m_iSize;
                }

                bool IsNull() const
                {
                    return m_Parent->IsNull(m_iIndex);
                }

                void SetNull()
                {
                    m_Parent->SetNull(m_iIndex);
                }

                void EraseNull()
                {
                    m_Parent->EraseNull(m_iIndex);
                }

                template<class CType>
                const CType Get() const
                {
                    return m_Parent->Get<CType>(m_iIndex);
                }

                template<class CType>
                void Set(const CType & p_Value)
                {
                    m_Parent->Set<CType>(m_iIndex, p_Value);
                }

            private:
                CBase * m_Parent;
                unsigned int m_iIndex;

            };

            virtual void ClearField()
            {
                m_vFields.clear();
            }

            unsigned int GetFieldCount() const
            {
                return m_vFields.size();
            }

            CFieldHelper GetField(int p_iIndex) throw(...)
            {
                if (0 > p_iIndex || m_vFields.size() <= (unsigned int)p_iIndex)
                {
                    throw std::out_of_range("输入索引不合法");
                }

                return CFieldHelper(this, p_iIndex);
            }

            CFieldHelper GetField(const std::string & p_sFieldName)
            {
                return GetField(IndexOfField(p_sFieldName));
            }

            const CFieldHelper GetField(int p_iIndex) const
            {
                return const_cast<CBase *>(this)->GetField(p_iIndex);
            }

            const CFieldHelper GetField(const std::string & p_sFieldName) const
            {
                return const_cast<CBase *>(this)->GetField(p_sFieldName);
            }

            int IndexOfField(const std::string & p_sFieldName) const
            {
                for (unsigned int i = 0; i < m_vFields.size(); ++i)
                {
                    if (p_sFieldName == m_vFields[i].m_sName)
                    {
                        return i;
                    }
                }

                return -1;
            }

            bool IsNull(const int p_iIndex) const
            {
                return 0 != (NULL_DATA & GetFlag(p_iIndex));
            }

            void SetNull(const int p_iIndex)
            {
                SetFlag(p_iIndex, GetFlag(p_iIndex) | NULL_DATA);
            }

            void EraseNull(const int p_iIndex)
            {
                SetFlag(p_iIndex, GetFlag(p_iIndex) & ~NULL_DATA);
            }

            template<class CType>
            const CType Get(const int p_iIndex) const throw(...)
            {
                if (IsNull(p_iIndex))
                {
                    return 0;
                }

                const char * sAddress = GetAddress(p_iIndex);
                if (NULL == sAddress)
                {
                    return StringToValue(GetString(p_iIndex), CType());
                }

                switch (m_vFields[p_iIndex].m_iType)
                {
                case INT_DATA:
                    return (CType)*reinterpret_cast<const int *>(sAddress);
                case INT64_DATA:
                    return (CType)*reinterpret_cast<const __int64 *>(sAddress);
                case FLOAT_DATA:
                    return (CType)*reinterpret_cast<const double *>(sAddress);
                case STRING_DATA:
                    return StringToValue(Get<std::string>(p_iIndex), CType()); //内存中的字符串不一定以NULL结尾，需先用Get<std::string>(p_iIndex)取到正确值后再处理
                case BINARY_DATA:
                    if (sizeof(CType) > m_vFields[p_iIndex].m_iSize)
                    {
                        throw std::runtime_error("无法识别二进制数据的类型");
                    }

                    return *reinterpret_cast<const CType *>(sAddress);
                default:
                    throw std::runtime_error(vcl4c::string::Format("无法识别的数据类型(%d)", m_vFields[p_iIndex].m_iType));
                }
            }

            template<>
            const std::string Get(const int p_iIndex) const throw(...)
            {
                if (IsNull(p_iIndex))
                {
                    return "";
                }

                const char * sAddress = GetAddress(p_iIndex);
                if (NULL == sAddress)
                {
                    return GetString(p_iIndex);
                }

                switch (m_vFields[p_iIndex].m_iType)
                {
                case INT_DATA:
                    return ValueToString(*reinterpret_cast<const int *>(sAddress));
                case INT64_DATA:
                    return ValueToString(*reinterpret_cast<const __int64 *>(sAddress));
                case FLOAT_DATA:
                    return ValueToString(*reinterpret_cast<const double *>(sAddress));
                case STRING_DATA:
                    if ('\0' == sAddress[m_vFields[p_iIndex].m_iSize - 1])
                    {
                        return std::string(sAddress);
                    }
                    else
                    {
                        return std::string(sAddress, m_vFields[p_iIndex].m_iSize);
                    }
                case BINARY_DATA:
                    return std::string(sAddress, m_vFields[p_iIndex].m_iSize); //按二进制方式处理，中间有空字符也会返回
                default:
                    throw std::runtime_error(vcl4c::string::Format("无法识别的数据类型(%d)", m_vFields[p_iIndex].m_iType));
                }
            }

            template<class CType>
            void Set(const int p_iIndex, const CType & p_Value) throw(...)
            {
                char * sAddress = GetAddress(p_iIndex);
                if (NULL == sAddress)
                {
                    SetString(p_iIndex, ValueToString(p_Value));
                    return;
                }

                switch (m_vFields[p_iIndex].m_iType)
                {
                case INT_DATA:
                    *reinterpret_cast<int *>(sAddress) = p_Value;
                    break;
                case INT64_DATA:
                    *reinterpret_cast<__int64 *>(sAddress) = p_Value;
                    break;
                case FLOAT_DATA:
                    *reinterpret_cast<double *>(sAddress) = p_Value;
                    break;
                case STRING_DATA:
                    strncpy(sAddress, ValueToString(p_Value).c_str(), m_vFields[p_iIndex].m_iSize);
                    break;
                case BINARY_DATA:
                    if (sizeof(CType) > m_vFields[p_iIndex].m_iSize)
                    {
                        throw std::runtime_error("无法识别二进制数据的类型");
                    }

                    *reinterpret_cast<CType *>(sAddress) = p_Value;
                    break;
                default:
                    throw std::runtime_error(vcl4c::string::Format("无法识别的数据类型(%d)", m_vFields[p_iIndex].m_iType));
                }

                EraseNull(p_iIndex);
            }

            template<>
            void Set(const int p_iIndex, const std::string & p_Value) throw(...)
            {
                char * sAddress = GetAddress(p_iIndex);
                if (NULL == sAddress)
                {
                    SetString(p_iIndex, p_Value);
                    return;
                }

                switch (m_vFields[p_iIndex].m_iType)
                {
                case INT_DATA:
                    *reinterpret_cast<int *>(sAddress) = atoi(p_Value.c_str());
                    break;
                case INT64_DATA:
                    *reinterpret_cast<__int64 *>(sAddress) = _atoi64(p_Value.c_str());
                    break;
                case FLOAT_DATA:
                    *reinterpret_cast<double *>(sAddress) = atof(p_Value.c_str());
                    break;
                case STRING_DATA:
                    strncpy(sAddress, p_Value.c_str(), m_vFields[p_iIndex].m_iSize);
                    break;
                case BINARY_DATA:
                    if ((int)p_Value.size() > m_vFields[p_iIndex].m_iSize)
                    {
                        memcpy(sAddress, p_Value.c_str(), m_vFields[p_iIndex].m_iSize);
                    }
                    else
                    {
                        memcpy(sAddress, p_Value.c_str(), p_Value.size());
                        memset(sAddress + p_Value.size(), 0, m_vFields[p_iIndex].m_iSize - p_Value.size());
                    }
                    break;
                default:
                    throw std::runtime_error(vcl4c::string::Format("无法识别的数据类型(%d)", m_vFields[p_iIndex].m_iType));
                }

                EraseNull(p_iIndex);
            }

            template<class CType>
            void AddField(const std::string & p_sName)
            {
                DoAddField(p_sName, GetType(CType()), sizeof(CType));
            }

            void AddField(const std::string & p_sName, const int p_iSize)
            {
                DoAddField(p_sName, STRING_DATA, p_iSize);
            }

            void AddBinaryField(const std::string & p_sName, const int p_iSize)
            {
                DoAddField(p_sName, BINARY_DATA, p_iSize);
            }

        protected:
            virtual std::string GetString(const int p_iIndex) const
            {
                return "";
            }

            virtual void SetString(const int p_iIndex, const std::string & p_sValue)
            {
            }

            virtual BYTE GetFlag(const int p_iIndex) const
            {
                return *reinterpret_cast<const BYTE *>(GetAddress(p_iIndex) - sizeof(BYTE));
            }

            virtual void SetFlag(const int p_iIndex, const BYTE p_iFlag)
            {
                *reinterpret_cast<BYTE *>(GetAddress(p_iIndex) - sizeof(BYTE)) = p_iFlag;
            }

            virtual char * GetAddress(const int p_iIndex) const = 0;

            virtual void DoAddField(const std::string & p_sName, const EFieldType p_iType, const int p_iSize)
            {
                m_vFields.push_back(CField(p_sName, p_iType, GetRecordSize() + sizeof(BYTE), p_iSize));
            }

            unsigned int GetRecordSize() const
            {
                if (m_vFields.empty())
                {
                    return 0;
                }
                else
                {
                    const CField & fld = m_vFields.back();
                    return fld.m_iOffset + fld.m_iSize;
                }
            }

        protected:
            //模板辅助函数
            static int StringToValue(const std::string & p_sValue, int)
            {
                return atoi(p_sValue.c_str());
            }

            static __int64 StringToValue(const std::string & p_sValue, __int64)
            {
                return _atoi64(p_sValue.c_str());
            }

            static double StringToValue(const std::string & p_sValue, double)
            {
                return atof(p_sValue.c_str());
            }

            static const std::string ValueToString(const int p_Value)
            {
                return vcl4c::string::Format("%d", p_Value);
            }

            static const std::string ValueToString(const __int64 p_Value)
            {
                return vcl4c::string::Format("%I64d", p_Value);
            }

            static const std::string ValueToString(const double p_Value)
            {
                return vcl4c::string::Format("%.16g", p_Value);
            }

            static EFieldType GetType(int)
            {
                return INT_DATA;
            }

            static EFieldType GetType(__int64)
            {
                return INT64_DATA;
            }

            static EFieldType GetType(double)
            {
                return FLOAT_DATA;
            }

        private:
            struct CField
            {
                CField(const std::string & p_sName, const EFieldType p_iType, const unsigned int p_iOffset, const unsigned int p_iSize)
                    : m_sName(p_sName), m_iType(p_iType), m_iOffset(p_iOffset), m_iSize(p_iSize)
                {
                }

                std::string m_sName;
                EFieldType m_iType;
                unsigned int m_iOffset;
                unsigned int m_iSize;
            };

            std::vector<CField> m_vFields;

        };

        typedef CBase::CFieldHelper CFieldHelper;

        class CStruct
            : public CBase
        {
        protected:
            virtual char * GetAddress(const int p_iIndex) const
            {
                return const_cast<char *>(m_sBuffer.c_str() + GetField(p_iIndex).GetOffset());
            }

            virtual void DoAddField(const std::string & p_sName, const EFieldType p_iType, const int p_iSize)
            {
                __super::DoAddField(p_sName, p_iType, p_iSize);

                if (GetRecordSize() > m_sBuffer.size())
                {
                    m_sBuffer.resize(GetRecordSize());
                }
            }

        private:
            std::string m_sBuffer;

        };

        class CDataSetBase
            : public CBase
        {
        public:
            virtual unsigned int GetRecordCount() const = 0;

            virtual void First() = 0;

            virtual void Next() = 0;

            virtual bool IsEof() const = 0;

            virtual void EmptyData() = 0;

            virtual void Append() = 0;

            virtual void Post() = 0;

        };

        class CDataSet
            : public CDataSetBase
        {
        public:
            CDataSet()
                : m_iCursorPos(0)
            {
            }

            virtual void ClearField() throw(...)
            {
                if (!m_vDatas.empty())
                {
                    throw std::runtime_error("有数据时不允许变更列字段");
                }

                __super::ClearField();
            }

        public:
            //CDataSetBase接口实现
            virtual unsigned int GetRecordCount() const
            {
                return m_vDatas.size();
            }

            virtual void First()
            {
                m_iCursorPos = 0;
            }

            virtual void Next()
            {
                ++m_iCursorPos;
                if (m_iCursorPos > m_vDatas.size())
                {
                    m_iCursorPos = m_vDatas.size();
                }
            }

            virtual bool IsEof() const
            {
                return m_iCursorPos >= m_vDatas.size();
            }

            virtual void EmptyData()
            {
                m_iCursorPos = 0;
                m_vDatas.clear();
            }

            virtual void Append()
            {
                m_iCursorPos = m_vDatas.size();

                std::string strRecord;
                strRecord.resize(GetRecordSize());
                m_vDatas.push_back(strRecord);
            }

            virtual void Post()
            {
            }

        protected:
            virtual char * GetAddress(const int p_iIndex) const
            {
                if (IsEof())
                {
                    return NULL;
                }

                return const_cast<char *>(m_vDatas[m_iCursorPos].c_str() + GetField(p_iIndex).GetOffset());
            }

            virtual void DoAddField(const std::string & p_sName, const EFieldType p_iType, const int p_iSize) throw(...)
            {
                if (!m_vDatas.empty())
                {
                    throw std::runtime_error("有数据时不允许变更列字段");
                }

                __super::DoAddField(p_sName, p_iType, p_iSize);
            }

            void AppendRawData(const std::string & p_sData)
            {
                Append();
                m_vDatas.back().assign(p_sData);
            }

        private:
            std::vector<std::string> m_vDatas;
            unsigned int m_iCursorPos;

        };
    }
}
