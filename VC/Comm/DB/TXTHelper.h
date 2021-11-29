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

#include <ios>
#include <strstream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "DataSet.h"
#include "../StringHelper.h"

namespace vcl4c
{
    namespace db
    {
        class CTxtHelper
        {
        public:
            CTxtHelper()
                : m_pStream(NULL)
            {
            }

            ~CTxtHelper()
            {
                Close();
            }

        public:
            //�ļ����ͣ�Ŀǰ֧���������͵��ļ�����
            //һ���Ǵ����ļ����������Զ���ȡ�ļ��е����ݣ�
            //һ���Ǵ����ڴ�飬������ڴ���ж�ȡ��Ҫ�����ݣ�
            enum EFileType{ftFileName, ftMemBuffer};
            void Open(const EFileType p_ftFileType, const void *p_ParamValue, const int p_nParamValueLen) throw(...)
            {
                if(ftMemBuffer == p_ftFileType)
                {
                    Close();

                    if(NULL == p_ParamValue)
                    {
                        throw std::invalid_argument("���ݻ�����Ч");
                    }

                    m_pStream = new std::strstream(reinterpret_cast<char *>(const_cast<void *>(p_ParamValue)),
                        p_nParamValueLen, std::ios_base::in);
                }
                else if(ftFileName == p_ftFileType)
                {
                    Close();

                    if(NULL == p_ParamValue)
                    {
                        throw std::invalid_argument("�ļ�����Ч");
                    }

                    m_pStream = new std::fstream(reinterpret_cast<char *>(const_cast<void *>(p_ParamValue)),
                        std::ios_base::in);
                }
                else
                {
                    throw std::invalid_argument("�ļ�������Ч");
                }
            }

            void Close()
            {
                if(m_pStream)
                {
                    delete m_pStream;
                    m_pStream = NULL;
                }
            }

            void Seek(const INT64 p_nPos = 0)
            {
                m_pStream->seekg(p_nPos);
            }

            void Seek(const int p_nOffset, const std::ios_base::seekdir p_nWay)
            {
                m_pStream->seekg(p_nOffset, p_nWay);
            }

            INT64 Tell() const
            {
                return m_pStream->tellg();
            }

            const bool Eof() const
            {
                return m_pStream->eof();
            }

            const std::string ReadLine()
            {
                if(m_pStream->eof())
                {
                    return "";
                }

                //1.0 ��ȡ1������
                std::string sLine = "";
                char sBuffer[MAXCOLWIDTH];
                do
                {
                    m_pStream->getline(sBuffer, sizeof(sBuffer) - 1);

                    //�ж������ݣ�ȥ�������ܵ�'\r'�ַ�
                    if('\0' != sBuffer[0])
                    {
                        int nLen = strlen(sBuffer);
                        if('\r' == sBuffer[nLen - 1]) //�����Windowsϵͳ�Ļ��У������ܻ��һ��'\r'�ַ���ֱ�ӽ����滻Ϊ���ַ�
                        {
                            sBuffer[nLen - 1] = '\0';
                        }

                        sLine += sBuffer;
                    }

                    //��������ļ�ĩβ��������ѭ��������������ȡ����������
                    if(m_pStream->eof())
                    {
                        break;
                    }

                    //���sBuffer���������δ�������з����������ȡ����
                    if(m_pStream->fail())
                    {
                        //��m_pStream���ڶ�ȡʧ��״̬ʱ��getline�����ȡ�κ����ݣ���Ҫ���������쳣״̬
                        m_pStream->clear(std::ios_base::goodbit);

                        continue;
                    }

                    break; //�ɹ���ȡһ�����ݺ��˳�ѭ��
                } while (true);

                return sLine;
            }

        private:
            enum {MAXCOLWIDTH = 255};
            std::iostream *m_pStream;

        };

        class CTxtDataSet
            : public CDataSetBase
        {
        public:
            CTxtDataSet()
                : m_thFile(), m_vData(), m_cDelimiter('|')
            {
            }

            void LoadFromFile(const std::string & p_sFileName, const std::string & p_sFormat, const char p_cDelimiter)
            {
                m_cDelimiter = p_cDelimiter;

                m_thFile.Close();
                m_thFile.Open(CTxtHelper::ftFileName, p_sFileName.c_str(), p_sFileName.size());

                ClearField();

                if (p_sFormat.empty())
                {
                    //�ⲿû�ж����ֶγ��ȣ����Ե�1�е��ֶγ���Ϊ׼
                    ReadLine(false);
                    if (0 == m_vData.size())
                    {
                        return;
                    }

                    for (unsigned int i = 0; i < m_vData.size(); ++i)
                    {
                        AddField(vcl4c::string::Format("%d", i), m_vData[i].size());
                    }

                    m_thFile.Seek(0);
                }
                else
                {
                    //�����ⲿ����ĳ��Ƚ���ÿ���ֶ�
                    std::vector<std::pair<std::string, std::string>> vFormat;
                    vcl4c::string::SplitString(vFormat, p_sFormat, '=', '&');
                    for (std::vector<std::pair<std::string, std::string>>::const_iterator it = vFormat.begin(); it != vFormat.end(); ++it)
                    {
                        AddField(it->first, atoi(it->second.c_str()));
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

                if ((int)m_vData.size() <= p_iIndex)
                {
                    return "";
                }

                return m_vData[p_iIndex];
            }

            virtual void SetString(const int p_iIndex, const std::string & p_sValue) throw(...)
            {
                if (0 > p_iIndex || (int)GetFieldCount() <= p_iIndex)
                {
                    throw std::out_of_range("�����������Ϸ�");
                }

                if ((int)m_vData.size() <= p_iIndex)
                {
                    return;
                }

                m_vData[p_iIndex] = p_sValue;
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
                INT64 nLastPos = m_thFile.Tell();

                //�ļ���¼�� = �ļ��ܴ�С / ��һ�г���
                m_thFile.Seek(0);
                m_thFile.ReadLine();
                INT64 nRowLen = m_thFile.Tell();
                m_thFile.Seek(0, std::ios_base::end);
                INT64 nTotalLen = m_thFile.Tell();

                m_thFile.Seek(nLastPos);

                return 0 != nRowLen ? int(nTotalLen / nRowLen) : 0;
            }

            virtual void First()
            {
                m_thFile.Seek(0);
                Next();
            }

            virtual void Next()
            {
                ReadLine();
            }

            virtual bool IsEof() const
            {
                return 0 == m_vData.size();
            }

            virtual void EmptyData()
            {
                throw std::runtime_error("δʵ��");
            }

            virtual void Append()
            {
                throw std::runtime_error("δʵ��");
            }

            virtual void Post()
            {
                throw std::runtime_error("δʵ��");
            }

        private:
            mutable CTxtHelper m_thFile;
            std::vector<std::string> m_vData;
            char m_cDelimiter;

            void ReadLine(const bool p_bAutoTrim = true)
            {
                m_vData.clear();

                //1.0 ��ȡ1������
                std::string sLine = "";
                while(!m_thFile.Eof())
                {
                    sLine = m_thFile.ReadLine();
                    if("" != sLine)
                    {
                        break;
                    }
                };

                if("" == sLine)
                {
                    return;
                }

                //2.0 ��'|'Ϊ�ָ�������ÿ���ֶν�����m_vData��
                vcl4c::string::SplitString(m_vData, sLine, m_cDelimiter);

                //3.0 ȥ��ÿ���ֶ���β�Ŀո�
                if (p_bAutoTrim)
                {
                    std::for_each(m_vData.begin(), m_vData.end(), vcl4c::string::Trim);
                }
            }

        };
    }
}
