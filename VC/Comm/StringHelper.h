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
            int nSize = _vscprintf(p_sFormat, p_Arguments); //根据MSDN的文档定义，当结果内存不足时，vsnprintf返回-1，而Linux返回实际需要的字节数
                                                            //因此，在Windows中，使用_vscprintf进行代替
#else
            va_list vArguments;
            va_copy(vArguments, p_Arguments); //GCC每次调用完vsnprintf之后，会修改p_Arguments的值，导致第2次调用出现段错误
                                              //个人推测每次调用后，指针偏移到下一个参数的位置上了
            int nSize = vsnprintf(NULL, 0, p_sFormat, p_Arguments);
#endif

            if(0 > nSize)
            {
                return -1;
            }

            nSize += sizeof(char); //_vscprintf返回的值不包含最后的空终止符
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

        //字符串切割函数，用于将指定分隔符(例如",")分隔的字符串列表拆分为向量数组
        //    p_vResult       返回的字符串数组
        //    p_sDelimitText  要解析的字符串，其各元素之间用分隔符(例如",")隔开
        //    p_cDelimiter    分隔符
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

        //字符串切割函数，用于将键值对形式的字符串拆解为map对象
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

        //去掉首尾的空格
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
            return p_cValue >= 0x80; //TODO: Linux如何实现有待考虑(需参考stdlib.h中mbstowcs这类函数的实现)
#endif
        }

        //截取字符串中某个位置上的指定长度的子字符串
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
            case cpGbk: //按GBK编码的字符串进行截取
                nIndex = p_nIndex;
                nLength = (p_nIndex + p_nCount < p_sString.size()) ? (p_nIndex + p_nCount) : p_sString.size();

                nReturnCount = 0;
                while(nIndex < nLength)
                {
                    if(IsLeadByte(p_sString[nIndex])) //多字节字符，长度为2
                    {
                        if(nIndex + 1 < nLength) //多字节的第2字节在截取范围内
                        {
                            nReturnCount += 2;
                            nIndex += 2;
                        }
                        else //多字节字符被截断，直接退出
                        {
                            break;
                        }
                    }
                    else //单字节字符，长度为1
                    {
                        ++nReturnCount;
                        ++nIndex;
                    }
                }

                break;
            case cpUtf8: //按UTF8编码的字符串进行截取
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

                    //如果下一个字符为ascii字符或非10xxxxxxb字符，则当前字符为完整utf8字符
                    if(0 == (p_sString[nIndex + 1] & 0x80)
                        || 0x80 != (p_sString[nIndex + 1] & 0xC0))
                    {
                        nReturnCount = p_nCount;
                    }
                    else
                    {
                        //向前搜索，跳过10xxxxxxb字符
                        while(nIndex >= p_nIndex)
                        {
                            //碰到ascii字符，返回当前位置
                            if(0 == (p_sString[nIndex] & 0x80))
                            {
                                break;
                            }

                            if (0 == nIndex)
                            {
                                return "";
                            }

                            //碰到utf8第1字节字符，返回前一个位置
                            if(0x80 != (p_sString[nIndex] & 0xC0))
                            {
                                --nIndex;
                                break;
                            }

                            --nIndex;
                        }

                        //如果字符串有异常，未找到一个完整的utf8字符，则返回空
                        if(nIndex < p_nIndex)
                        {
                            return "";
                        }

                        nReturnCount = nIndex - p_nIndex + 1;
                    }
                }

                break;
            default: //按字节方式截取
                nReturnCount = p_nCount;

                break;
            }

            return p_sString.substr(p_nIndex, nReturnCount);
        };

        //将GBK编码的字符串转码为UTF-8格式
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

        //将UTF-8编码的字符串转码为GBK格式
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

        //将BASE64编码的字符串转码为内存流
        //    Base64ToMemInternal为内部实现，不建议外部使用
        //    Base64ToMemDirect不开辟新的内存空间，直接将转换后的结果保存在输入的内存中，会破坏原始数据
        //    Base64ToMem开辟新的内存空间存放结果数据，不破坏原始数据
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
            //内存分配失败，直接退出
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

        //功能描述：将内存流转码为BASE64编码的字符串
        inline std::string MemToBase64(const char *p_sSrc, const int p_nSrcLen)
        {
            static const char c_sBase64Tab[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

            std::string sResult;

            unsigned char c1 = 0, c2 = 0, c3 = 0; //输入缓冲区读出3个字节
            int nDiv = p_nSrcLen / 3; //输入数据长度除以3得到的倍数
            int nMod = p_nSrcLen % 3; //输入数据长度除以3得到的余数

            //每次取3个字节，编码成4个字符
            for(int n = 0; n < nDiv; ++n)
            {
                //取3个字节
                c1 = *p_sSrc++;
                c2 = *p_sSrc++;
                c3 = *p_sSrc++;

                //编码成4个字符
                sResult.push_back(c_sBase64Tab[c1 >> 2]);
                sResult.push_back(c_sBase64Tab[((c1 << 4) | (c2 >> 4)) & 0x3f]);
                sResult.push_back(c_sBase64Tab[((c2 << 2) | (c3 >> 6)) & 0x3f]);
                sResult.push_back(c_sBase64Tab[c3 & 0x3f]);
            }

            //编码余下的字节
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
