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

#include <stdarg.h>
#include <vector>
#include <string>
#include <map>

#if defined(WIN32)
    #include <Windows.h>
#else
    #include <iconv.h>
#endif

#include "array_auto_ptr.h"

namespace vcl4c
{
    namespace string
    {
#if defined(WIN32)
        inline int Format(std::string & p_sResult, const char * p_sFormat, const va_list & p_Arguments)
#else
        inline int Format(std::string & p_sResult, const char * p_sFormat, va_list p_Arguments)
#endif
        {
#if defined(WIN32)
            va_list vArguments = p_Arguments;
            int nSize = _vscprintf(p_sFormat, p_Arguments); //����MSDN���ĵ����壬������ڴ治��ʱ��vsnprintf����-1����Linux����ʵ����Ҫ���ֽ���
                                                            //��ˣ���Windows�У�ʹ��_vscprintf���д���
#else
            va_list vArguments;
            va_copy(vArguments, p_Arguments); //GCCÿ�ε�����vsnprintf֮�󣬻��޸�p_Arguments��ֵ�����µ�2�ε��ó��ֶδ���
                                              //�����Ʋ�ÿ�ε��ú�ָ��ƫ�Ƶ���һ��������λ������
            int nSize = vsnprintf(NULL, 0, p_sFormat, p_Arguments);
#endif

            if(0 > nSize)
            {
                return -1;
            }

            nSize += sizeof(char); //_vscprintf���ص�ֵ���������Ŀ���ֹ��
            other::array_auto_ptr<char> sResult(new char[nSize]);
            int nRetCode = vsnprintf(sResult.get(), nSize, p_sFormat, vArguments);
            if(0 > nRetCode)
            {
                return -1;
            }
            p_sResult = sResult.get();

            return 0;
        }

        inline int Format(std::string & p_sResult, const char * p_sFormat, ...)
        {
            va_list vArg;
            va_start(vArg, p_sFormat);

            return Format(p_sResult, p_sFormat, vArg);
        }

        inline const std::string Format(const char * p_sFormat, ...)
        {
            std::string sResult;

            va_list vArg;
            va_start(vArg, p_sFormat);
            Format(sResult, p_sFormat, vArg);

            return sResult;
        }

        //�ַ����и�������ڽ�ָ���ָ���(����",")�ָ����ַ����б���Ϊ��������
        //    p_vResult       ���ص��ַ�������
        //    p_sDelimitText  Ҫ�������ַ��������Ԫ��֮���÷ָ���(����",")����
        //    p_cDelimiter    �ָ���
        inline void SplitString(std::vector<std::string>& p_vResult, const std::string& p_sDelimitText, const char p_cDelimiter)
        {
            std::string::size_type nBeginPos = 0, nEndPos = 0;
            p_vResult.clear();

            if(p_sDelimitText.size() == 0)
                return;

            nEndPos = p_sDelimitText.find_first_of(p_cDelimiter, nBeginPos);
            while(nEndPos != std::string::npos)
            {
                p_vResult.push_back(p_sDelimitText.substr(nBeginPos, nEndPos - nBeginPos));
                nBeginPos = nEndPos + 1;
                nEndPos = p_sDelimitText.find_first_of(p_cDelimiter, nBeginPos);
            }
            p_vResult.push_back(p_sDelimitText.substr(nBeginPos));
        }

        //�ַ����и�������ڽ���ֵ����ʽ���ַ������Ϊmap����
        inline void Insert(std::map<std::string, std::string> &p_Result, std::pair<std::string, std::string> & p_Value)
        {
            p_Result.insert(p_Value);
        }

        inline void Insert(std::vector<std::pair<std::string, std::string>> &p_Result, std::pair<std::string, std::string> & p_Value)
        {
            p_Result.push_back(p_Value);
        }

        template<class CResult>
        inline void SplitString(CResult &p_Result,
            const std::string &p_sDelimitText, const char p_cFieldDelimiter, const char p_cRowDelimiter)
        {
            std::string::size_type iBeginPos = 0, iRowPos = 0;
            p_Result.clear();

            if(p_sDelimitText.size() == 0)
                return;

            while (true)
            {
                std::string::size_type iFieldPos = p_sDelimitText.find_first_of(p_cFieldDelimiter, iBeginPos);
                if (std::string::npos == iFieldPos)
                {
                    break;
                }

                iRowPos = p_sDelimitText.find_first_of(p_cRowDelimiter, iFieldPos + 1);
                if (std::string::npos == iRowPos)
                {
                    Insert(p_Result, std::make_pair(
                        p_sDelimitText.substr(iBeginPos, iFieldPos - iBeginPos),
                        p_sDelimitText.substr(iFieldPos + 1)));
                    break;
                }
                else
                {
                    Insert(p_Result, std::make_pair(
                        p_sDelimitText.substr(iBeginPos, iFieldPos - iBeginPos),
                        p_sDelimitText.substr(iFieldPos + 1, iRowPos - iFieldPos - 1)));
                    iBeginPos = iRowPos + 1;
                }
            }
        }

        //ȥ����β�Ŀո�
        inline std::string &Trim(std::string &p_str)
        {
            std::string::size_type pos = p_str.find_last_not_of(' ');
            if(pos == std::string::npos)
            {
                p_str.erase(p_str.begin(), p_str.end());
                return p_str;
            }

            p_str.erase(pos + 1);
            pos = p_str.find_first_not_of(' ');
            if(pos != std::string::npos)
            {
                p_str.erase(0, pos);
            }

            return p_str;
        };

        enum ECodePage {cpAscii, cpGbk, cpUtf8};

        inline bool IsLeadByte(unsigned char p_cValue)
        {
#if defined(WIN32)
            return TRUE == ::IsDBCSLeadByte(p_cValue);
#else
            return p_cValue >= 0x80; //TODO: Linux���ʵ���д�����(��ο�stdlib.h��mbstowcs���ຯ����ʵ��)
#endif
        }

        //��ȡ�ַ�����ĳ��λ���ϵ�ָ�����ȵ����ַ���
        inline std::string SubStr(const std::string &p_sString, const UINT p_nIndex, const UINT p_nCount,
            const ECodePage p_nCodePage = cpAscii)
        {
            if(p_nIndex >= p_sString.length())
            {
                return "";
            }

            UINT nIndex = 0, nReturnCount = 0, nLength = 0;

            switch(p_nCodePage)
            {
            case cpGbk: //��GBK������ַ������н�ȡ
                nIndex = p_nIndex;
                nLength = (p_nIndex + p_nCount < p_sString.size()) ? (p_nIndex + p_nCount) : p_sString.size();

                nReturnCount = 0;
                while(nIndex < nLength)
                {
                    if(IsLeadByte(p_sString[nIndex])) //���ֽ��ַ�������Ϊ2
                    {
                        if(nIndex + 1 < nLength) //���ֽڵĵ�2�ֽ��ڽ�ȡ��Χ��
                        {
                            nReturnCount += 2;
                            nIndex += 2;
                        }
                        else //���ֽ��ַ����ضϣ�ֱ���˳�
                        {
                            break;
                        }
                    }
                    else //���ֽ��ַ�������Ϊ1
                    {
                        ++nReturnCount;
                        ++nIndex;
                    }
                }

                break;
            case cpUtf8: //��UTF8������ַ������н�ȡ
                if (0 == p_nCount)
                {
                    return "";
                }
                else if(p_nIndex + p_nCount >= p_sString.size())
                {
                    nReturnCount = p_nCount;
                }
                else
                {
                    nIndex = p_nIndex + p_nCount - 1;

                    //�����һ���ַ�Ϊascii�ַ����10xxxxxxb�ַ�����ǰ�ַ�Ϊ����utf8�ַ�
                    if(0 == (p_sString[nIndex + 1] & 0x80)
                        || 0x80 != (p_sString[nIndex + 1] & 0xC0))
                    {
                        nReturnCount = p_nCount;
                    }
                    else
                    {
                        //��ǰ����������10xxxxxxb�ַ�
                        while(nIndex >= p_nIndex)
                        {
                            //����ascii�ַ������ص�ǰλ��
                            if(0 == (p_sString[nIndex] & 0x80))
                            {
                                break;
                            }

                            if (0 == nIndex)
                            {
                                return "";
                            }

                            //����utf8��1�ֽ��ַ�������ǰһ��λ��
                            if(0x80 != (p_sString[nIndex] & 0xC0))
                            {
                                --nIndex;
                                break;
                            }

                            --nIndex;
                        }

                        //����ַ������쳣��δ�ҵ�һ��������utf8�ַ����򷵻ؿ�
                        if(nIndex < p_nIndex)
                        {
                            return "";
                        }

                        nReturnCount = nIndex - p_nIndex + 1;
                    }
                }

                break;
            default: //���ֽڷ�ʽ��ȡ
                nReturnCount = p_nCount;

                break;
            }

            return p_sString.substr(p_nIndex, nReturnCount);
        };

        //��GBK������ַ���ת��ΪUTF-8��ʽ
        inline std::string GbkToUtf8(const std::string &p_str)
        {
            int nLen = 0;

            other::array_auto_ptr<wchar_t> strWide(new wchar_t[p_str.size()]);
            nLen = ::MultiByteToWideChar(CP_ACP, 0, p_str.c_str(), p_str.size(), strWide.get(), p_str.size());
            if(0 == nLen)
            {
                return "";
            }

            other::array_auto_ptr<char> strUtf8(new char[nLen * 4]);
            nLen = ::WideCharToMultiByte(CP_UTF8, 0, strWide.get(), nLen, strUtf8.get(), nLen * 4, NULL, NULL);
            if(0 == nLen)
            {
                return "";
            }

            return std::string(strUtf8.get(), strUtf8.get() + nLen);
        };

        //��UTF-8������ַ���ת��ΪGBK��ʽ
        inline std::string Utf8ToGbk(const std::string &p_str)
        {
            int nLen = 0;

            other::array_auto_ptr<wchar_t> strWide(new wchar_t[p_str.size()]);
            nLen = ::MultiByteToWideChar(CP_UTF8, 0, p_str.c_str(), p_str.size(), strWide.get(), p_str.size());
            if(0 == nLen)
            {
                return "";
            }

            other::array_auto_ptr<char> strGbk(new char[nLen * 4]);
            nLen = ::WideCharToMultiByte(CP_ACP, 0, strWide.get(), nLen, strGbk.get(), nLen * 4, NULL, NULL);
            if(0 == nLen)
            {
                return "";
            }

            return std::string(strGbk.get(), strGbk.get() + nLen);
        };

        //��BASE64������ַ���ת��Ϊ�ڴ���
        //    Base64ToMemInternalΪ�ڲ�ʵ�֣��������ⲿʹ��
        //    Base64ToMemDirect�������µ��ڴ�ռ䣬ֱ�ӽ�ת����Ľ��������������ڴ��У����ƻ�ԭʼ����
        //    Base64ToMem�����µ��ڴ�ռ��Ž�����ݣ����ƻ�ԭʼ����
        inline int DecodeBase64(const char* p_sSrc, unsigned char* p_sDest, int p_nSrcLen)
        {
            const char c_sBase64Table[] =
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                62,        // '+'
                0, 0, 0,
                63,        // '/'
                52, 53, 54, 55, 56, 57, 58, 59, 60, 61,        // '0'-'9'
                0, 0, 0, 0, 0, 0, 0,
                0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
                13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,        // 'A'-'Z'
                0, 0, 0, 0, 0, 0,
                26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
                39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,        // 'a'-'z'
            };

            int nDestLen = 0;

            for(int i = 0; i < p_nSrcLen; i += 4)
            {
                int nValue = c_sBase64Table[int(*p_sSrc++)] << 18;
                nValue += c_sBase64Table[int(*p_sSrc++)] << 12;
                *p_sDest++ = (unsigned char)((nValue & 0x00ff0000) >> 16);
                nDestLen++;

                if (*p_sSrc != '=')
                {
                    nValue += c_sBase64Table[int(*p_sSrc++)] << 6;
                    *p_sDest++ = (nValue & 0x0000ff00) >> 8;
                    nDestLen++;

                    if (*p_sSrc != '=')
                    {
                        nValue += c_sBase64Table[int(*p_sSrc++)];
                        *p_sDest++ =nValue & 0x000000ff;
                        nDestLen++;
                    }

                    break;
                }
            }

            *p_sDest = '\0';

            return nDestLen;
        }

        inline bool Base64ToMemInternal(int &p_nRealDestLen, char *p_pDest, const int /*p_nDestLen*/, const char *p_sSrc, const int p_nSrcLen)
        {
            p_nRealDestLen = DecodeBase64(p_sSrc, reinterpret_cast<unsigned char *>(p_pDest), p_nSrcLen);
            return true;
        }

        inline bool Base64ToMemDirect(int &p_nRealDestLen, char *p_sSrc, const int p_nSrcLen)
        {
            return Base64ToMemInternal(p_nRealDestLen, p_sSrc, p_nSrcLen, p_sSrc, p_nSrcLen);
        }

        inline other::array_auto_ptr<char> Base64ToMem(int &p_nRealDestLen, const char *p_sSrc, const int p_nSrcLen)
        {
            other::array_auto_ptr<char> pBlank;
            other::array_auto_ptr<char> pMem(new char[p_nSrcLen]);
            //�ڴ����ʧ�ܣ�ֱ���˳�
            if(!pMem)
            {
                p_nRealDestLen = 0;
                return pBlank;
            }

            if(Base64ToMemInternal(p_nRealDestLen, pMem.get(), p_nSrcLen, p_sSrc, p_nSrcLen))
                return pMem;

            p_nRealDestLen = 0;
            return pBlank;
        }

        //�������������ڴ���ת��ΪBASE64������ַ���
        inline std::string MemToBase64(const char *p_sSrc, const int p_nSrcLen)
        {
            static const char c_sBase64Tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            std::string sResult;

            unsigned char c1 = 0, c2 = 0, c3 = 0; //���뻺��������3���ֽ�
            int nDiv = p_nSrcLen / 3; //�������ݳ��ȳ���3�õ��ı���
            int nMod = p_nSrcLen % 3; //�������ݳ��ȳ���3�õ�������

            //ÿ��ȡ3���ֽڣ������4���ַ�
            for(int n = 0; n < nDiv; ++n)
            {
                //ȡ3���ֽ�
                c1 = *p_sSrc++;
                c2 = *p_sSrc++;
                c3 = *p_sSrc++;

                //�����4���ַ�
                sResult.push_back(c_sBase64Tab[c1 >> 2]);
                sResult.push_back(c_sBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f]);
                sResult.push_back(c_sBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f]);
                sResult.push_back(c_sBase64Tab[c3 & 0x3f]);
            }

            //�������µ��ֽ�
            if (nMod == 1)
            {
                c1 = *p_sSrc++;
                sResult.push_back(c_sBase64Tab[(c1 & 0xfc) >> 2]);
                sResult.push_back(c_sBase64Tab[((c1 & 0x03) << 4)]);
                sResult.push_back('=');
                sResult.push_back('=');
            }
            else if (nMod == 2)
            {
                c1 = *p_sSrc++;
                c2 = *p_sSrc++;
                sResult.push_back(c_sBase64Tab[(c1 & 0xfc) >> 2]);
                sResult.push_back(c_sBase64Tab[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)]);
                sResult.push_back(c_sBase64Tab[((c2 & 0x0f) << 2)]);
                sResult.push_back('=');
            }

            return sResult;
        }
    }
}
