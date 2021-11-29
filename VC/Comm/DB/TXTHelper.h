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
            //文件类型，目前支持两种类型的文件——
            //一种是传入文件名，程序自动读取文件中的数据；
            //一种是传入内存块，程序从内存块中读取需要的数据；
            enum EFileType{ftFileName, ftMemBuffer};
            void Open(const EFileType p_ftFileType, const void *p_ParamValue, const int p_nParamValueLen) throw(...)
            {
                if(ftMemBuffer == p_ftFileType)
                {
                    Close();

                    if(NULL == p_ParamValue)
                    {
                        throw std::invalid_argument("数据缓存无效");
                    }

                    m_pStream = new std::strstream(reinterpret_cast<char *>(const_cast<void *>(p_ParamValue)),
                        p_nParamValueLen, std::ios_base::in);
                }
                else if(ftFileName == p_ftFileType)
                {
                    Close();

                    if(NULL == p_ParamValue)
                    {
                        throw std::invalid_argument("文件名无效");
                    }

                    m_pStream = new std::fstream(reinterpret_cast<char *>(const_cast<void *>(p_ParamValue)),
                        std::ios_base::in);
                }
                else
                {
                    throw std::invalid_argument("文件类型无效");
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

                //1.0 读取1行数据
                std::string sLine = "";
                char sBuffer[MAXCOLWIDTH];
                do
                {
                    m_pStream->getline(sBuffer, sizeof(sBuffer) - 1);

                    //有读出数据，去掉最后可能的'\r'字符
                    if('\0' != sBuffer[0])
                    {
                        int nLen = strlen(sBuffer);
                        if('\r' == sBuffer[nLen - 1]) //如果是Windows系统的换行，最后可能会多一个'\r'字符，直接将其替换为空字符
                        {
                            sBuffer[nLen - 1] = '\0';
                        }

                        sLine += sBuffer;
                    }

                    //如果读到文件末尾，则跳出循环，继续解析读取出来的内容
                    if(m_pStream->eof())
                    {
                        break;
                    }

                    //如果sBuffer被填充满后还未遇到换行符，则继续读取数据
                    if(m_pStream->fail())
                    {
                        //当m_pStream处于读取失败状态时，getline不会读取任何数据，需要清理所有异常状态
                        m_pStream->clear(std::ios_base::goodbit);

                        continue;
                    }

                    break; //成功读取一行数据后，退出循环
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
                    //外部没有定义字段长度，则以第1行的字段长度为准
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
                    //根据外部定义的长度解析每个字段
                    std::vector<std::pair<std::string, std::string>> vFormat;
                    vcl4c::string::SplitString(vFormat, p_sFormat, '=', '&');
                    for (std::vector<std::pair<std::string, std::string>>::const_iterator it = vFormat.begin(); it != vFormat.end(); ++it)
                    {
                        AddField(it->first, atoi(it->second.c_str()));
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
                    throw std::out_of_range("输入索引不合法");
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
                INT64 nLastPos = m_thFile.Tell();

                //文件记录数 = 文件总大小 / 第一行长度
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
                throw std::runtime_error("未实现");
            }

            virtual void Append()
            {
                throw std::runtime_error("未实现");
            }

            virtual void Post()
            {
                throw std::runtime_error("未实现");
            }

        private:
            mutable CTxtHelper m_thFile;
            std::vector<std::string> m_vData;
            char m_cDelimiter;

            void ReadLine(const bool p_bAutoTrim = true)
            {
                m_vData.clear();

                //1.0 读取1行数据
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

                //2.0 以'|'为分隔符，将每个字段解析到m_vData中
                vcl4c::string::SplitString(m_vData, sLine, m_cDelimiter);

                //3.0 去除每个字段首尾的空格
                if (p_bAutoTrim)
                {
                    std::for_each(m_vData.begin(), m_vData.end(), vcl4c::string::Trim);
                }
            }

        };
    }
}
