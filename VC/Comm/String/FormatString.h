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
#include <string>
#include <list>
#include <algorithm>
#include <locale.h>

namespace vcl4c
{
    namespace string
    {
        enum EFlag
        {
            flSign = 0x00001, flSignsp = 0x00002, flLeft = 0x00004, flLeadZero = 0x00008,
            flLong = 0x00010, flShort = 0x00020, flSigned = 0x00040, flAlternate = 0x00080,
            flNegative = 0x00100, /*flForceoctal = 0x00200,*/ flLongDouble = 0x00400, flWideChar = 0x00800, //flForceoctal表示8进制的整数是否要前补0，可通过flAlternate+Radix检测到，此标志多余
            flLongLong = 0x01000, flI64 = 0x02000, flUpperCase = 0x04000
        };
        enum EDataType{dtText, dtChar, dtWideChar, dtString, dtWideString, dtDouble, dtInt64, dtCustom};
        class CFormatData
        {
            const static int c_iBufferSize = 512;
            EDataType m_dtDataType;
            int m_iFlag, m_iWidth, m_iPrecision;
            union
            {
                char m_cChar;
                wchar_t m_wcChar;
                struct
                {
                    int m_iLen;
                    const char *m_sValue;
                }m_sString;
                struct
                {
                    int m_iLen;
                    const wchar_t *m_wsValue;
                }m_wsString;
                double m_dbDouble;
                struct
                {
                    int m_iRadix;
                    __int64 m_iValue;
                }m_iInt64;
                void *m_pCustom;
            } m_Value;
        public:
            CFormatData(void *p_pCustom)
                : m_dtDataType(dtCustom), m_iFlag(0), m_iWidth(0), m_iPrecision(-1)
            {
                m_Value.m_pCustom = p_pCustom;
            }
            //常规文本(格式化字符串中不带“%”的部分)
            CFormatData(const char *p_sValue, const int p_iLen)
                : m_dtDataType(dtText), m_iFlag(0), m_iWidth(0), m_iPrecision(-1)
            {
                m_Value.m_sString.m_sValue = p_sValue;
                m_Value.m_sString.m_iLen = p_iLen;
            }
            //字符串类型
            CFormatData(const char *p_sValue, const int p_iFlag, const int p_iWidth, const int p_iPrecision)
                : m_dtDataType(dtString), m_iFlag(p_iFlag), m_iWidth(p_iWidth), m_iPrecision(p_iPrecision)
            {
                if(NULL == p_sValue)
                    m_Value.m_sString.m_sValue = "(null)";
                else
                    m_Value.m_sString.m_sValue = p_sValue;

                int iLen = strlen(p_sValue);
                if(m_iPrecision >= 0 && m_iPrecision < iLen)
                    iLen = m_iPrecision;

                m_Value.m_sString.m_iLen = iLen;
            }
            //字符串类型
            CFormatData(const wchar_t *p_wsValue, const int p_iFlag, const int p_iWidth, const int p_iPrecision)
                : m_dtDataType(dtWideString), m_iFlag(p_iFlag), m_iWidth(p_iWidth), m_iPrecision(p_iPrecision)
            {
                if(NULL == p_wsValue)
                    m_Value.m_wsString.m_wsValue = L"(null)";
                else
                    m_Value.m_wsString.m_wsValue = p_wsValue;

                int iLen = wcslen(p_wsValue);
                if(m_iPrecision >= 0 && m_iPrecision < iLen)
                    iLen = m_iPrecision;

                m_Value.m_wsString.m_iLen = iLen;
            }
            //整数类型
            CFormatData(va_list &p_Value, const int p_iRadix, const int p_iFlag, const int p_iWidth, const int p_iPrecision)
                : m_dtDataType(dtInt64), m_iFlag(p_iFlag), m_iWidth(p_iWidth), m_iPrecision(p_iPrecision)
            {
                __int64 iValue = 0;
                if(m_iFlag & (flI64 | flLongLong))
                    iValue = va_arg(p_Value, __int64);
                else if(m_iFlag & flLong)
                    iValue = va_arg(p_Value, int);
                else if(m_iFlag & flShort)
                {
                    if(m_iFlag & flSigned)
                        iValue = (short)(va_arg(p_Value, int));
                    else
                        iValue = (unsigned short)(va_arg(p_Value, int));
                }
                else
                {
                    if(m_iFlag & flSigned)
                        iValue = va_arg(p_Value, int);
                    else
                        iValue = (unsigned int)(va_arg(p_Value, int));
                }

                if((m_iFlag & flSigned) && (iValue < 0))
                {
                    iValue = -iValue;
                    m_iFlag |= flNegative;
                }

                if(0 == (m_iFlag & (flI64 | flLongLong)))
                    iValue &= 0xFFFFFFFF;

                if(m_iPrecision < 0)
                    m_iPrecision = 1;
                else
                {
                    m_iFlag &= ~flLeadZero;
                    if(m_iPrecision > c_iBufferSize)
                        m_iPrecision = c_iBufferSize;
                }

                m_Value.m_iInt64.m_iRadix = p_iRadix;
                m_Value.m_iInt64.m_iValue = iValue;
            }
            //浮点数以及字符类型
            template<typename CType>
            CFormatData(const CType p_Value, const int p_iFlag, const int p_iWidth, const int p_iPrecision)
                : m_iFlag(p_iFlag), m_iWidth(p_iWidth), m_iPrecision(p_iPrecision)
            {
                m_dtDataType = DataType(p_Value);

                *(CType *)&m_Value = p_Value;
            }

            const std::string ToString() const throw(...)
            {
                typedef std::string (CFormatData::*FDataToString)() const;
                //dtText, dtChar, dtWideChar, dtString, dtWideString, dtDouble, dtInt64
                const static FDataToString fnToString[] =
                {
                    &CFormatData::StringToString,
                    &CFormatData::CharToString,
                    &CFormatData::WideCharToString,
                    &CFormatData::StringToString,
                    &CFormatData::WideStringToString,
                    &CFormatData::DoubleToString,
                    &CFormatData::Int64ToString
                };

                return FormatOutput((this->*fnToString[m_dtDataType])());
            }

            virtual const std::string FormatOutput(std::string &p_sValue) const
            {
                char sPrefix[2]; //有iPrefixLen的保障，sPrefix可以不做初始化
                int iPrefixLen = 0;

                //16进制和8进制前缀的填补方法不同，16进制放到这里处理，8进制放到Int64ToString处理
                if(dtInt64 == m_dtDataType && 16 == m_Value.m_iInt64.m_iRadix && 0 != m_Value.m_iInt64.m_iValue)
                {
                    sPrefix[0] = '0';
                    if(m_iFlag & flUpperCase)
                        sPrefix[1] = 'X';
                    else
                        sPrefix[1] = 'x';
                    iPrefixLen = 2;
                }
                else if (m_iFlag & flSigned) //16进制都是无符号的，因此，这里用else if串联没问题
                {
                    if (m_iFlag & flNegative)
                    {
                        sPrefix[0] = '-';
                        iPrefixLen = 1;
                    }
                    else if (m_iFlag & flSign)
                    {
                        sPrefix[0] = '+';
                        iPrefixLen = 1;
                    }
                    else if (m_iFlag & flSignsp)
                    {
                        sPrefix[0] = ' ';
                        iPrefixLen = 1;
                    }
                }

                int iPadding = m_iWidth - p_sValue.size() - iPrefixLen;
                //如果不用对齐，也不用补前缀，直接返回原始值，提升性能
                if(0 >= iPadding)
                {
                    if(0 >= iPrefixLen)
                        return p_sValue;
                    else
                    {
                        std::string strResult;
                        strResult.append(sPrefix, sPrefix + iPrefixLen);
                        strResult.append(p_sValue);
                        return strResult;
                    }
                }

                std::string strResult;
                if(!(m_iFlag & flLeadZero) && !(m_iFlag & flLeft))
                    strResult.append(iPadding, ' ');
                strResult.append(sPrefix, sPrefix + iPrefixLen);
                if((m_iFlag & flLeadZero) && !(m_iFlag & flLeft))
                    strResult.append(iPadding, '0');
                strResult.append(p_sValue);
                if(m_iFlag & flLeft) //原逻辑在WideString转换失败的情况下，这里不会做对齐处理，为保持代码简洁，未对这种场景做特殊处理
                    strResult.append(iPadding, ' ');

                return strResult;
            }

            const EDataType GetDataType() const
            {
                return m_dtDataType;
            }

            const int GetPrecision() const
            {
                return m_iPrecision;
            }
        public:
            static std::string CharToString(const char p_cValue)
            {
                return std::string(1, p_cValue);
            }
            static std::string WideCharToString(const wchar_t p_wcValue)
            {
                int iRet = 0;
                char sBuffer[MB_LEN_MAX + 1];
                sBuffer[0] = '\0';
                errno_t eError = wctomb_s(&iRet, sBuffer, _countof(sBuffer), p_wcValue);
                if(0 != eError || 0 == iRet)
                    return "";
                else
                    return std::string(sBuffer, sBuffer + iRet);
            }
            static std::string StringToString(const char *p_sValue, const int p_iLength)
            {
                return std::string(p_sValue, p_sValue + p_iLength);
            }
            static std::string WideStringToString(const wchar_t *p_wsValue, const int p_iLength)
            {
                std::string strResult;

                const wchar_t *ws = p_wsValue;
                int iLen = p_iLength + 1;

                int iRet = 0;
                char sBuffer[MB_LEN_MAX + 1];
                sBuffer[0] = '\0';
                errno_t eError = 0;

                while(--iLen)
                {
                    eError = wctomb_s(&iRet, sBuffer, _countof(sBuffer), *ws);
                    if(0 != eError || 0 == iRet)
                    {
                        return "";
                    }

                    strResult.append(sBuffer, sBuffer + iRet);
                    ++ws;
                }

                return strResult;
            }
            static std::string DoubleToString(const double p_dbValue, const int p_iPrecision, const bool p_bForceDecimalPoint)
            {
                if(p_iPrecision + _CVTBUFSIZE > c_iBufferSize)
                    throw "输入的数值超过最大范围";

                std::string strResult;
                char sBuffer[c_iBufferSize];
                sBuffer[0] = '\0';
                int iDec = 0, iSign = 0;
                errno_t eError = _fcvt_s(sBuffer, c_iBufferSize, p_dbValue, p_iPrecision, &iDec, &iSign);
                if(0 != eError)
                    throw "数据转换失败";

                //在构造CFormatData之前，已经将m_Value.m_dbDouble转换成正数了，这里不再对iSign的值做处理
                if(iDec <= 0) //小数点在第一个数字之前
                {
                    strResult.append("0.");
                    strResult.append(min(abs(iDec), p_iPrecision), '0');
                    strResult.append(sBuffer);
                }
                else //小数点在第一个数字之后
                {
                    strResult.append(sBuffer, sBuffer + iDec);
                    strResult.append(1, '.');
                    strResult.append(sBuffer + iDec);
                }

                //当m_iPrecision为0时，默认不会有最后的小数点，只有在指定了“#”标志(例如，“%#.0f”)时，才会显示
                if((0 == p_iPrecision) && !p_bForceDecimalPoint)
                    if('.' == strResult.back())
                        strResult.pop_back(); //这里是删除最后的小数点

                return strResult;
            }
            static std::string Int64ToString(const __int64 p_iValue, const int p_iRadix = 10,
                const int p_iPrecision = -1, const bool p_bUpperCase = false, const bool p_bForcePrefix = false)
            {
                std::string strResult;

                __int64 iValue = p_iValue;
                const int iRadix = p_iRadix;
                int iPrecision = p_iPrecision + 1;
                int iHexAdjust = 'a' - '9' - 1;
                if(p_bUpperCase)
                    iHexAdjust = 'A' - '9' - 1;

                while(--iPrecision > 0 || 0 != iValue)
                {
                    char cNumber = (char)(iValue % iRadix + '0');
                    iValue /= iRadix;
                    if(cNumber > '9')
                        cNumber += iHexAdjust;
                    strResult.append(1, cNumber);
                }

                //16进制和8进制前缀的填补方法不同，16进制放到FormatOutput处理，8进制放到这里处理
                if(8 == iRadix && p_bForcePrefix)
                    if(0 == strResult.size() || '0' != strResult.back())
                        strResult.append(1, '0');

                return std::string(strResult.rbegin(), strResult.rend());
            }
        private:
            EDataType DataType(const char){return dtChar;};
            EDataType DataType(const wchar_t){return dtWideChar;};
            EDataType DataType(const double){return dtDouble;};
            std::string CharToString() const
            {
                return CharToString(m_Value.m_cChar);
            }
            std::string WideCharToString() const
            {
                return WideCharToString(m_Value.m_wcChar);
            }
            std::string StringToString() const
            {
                return StringToString(m_Value.m_sString.m_sValue, m_Value.m_sString.m_iLen);
            }
            std::string WideStringToString() const
            {
                return WideStringToString(m_Value.m_wsString.m_wsValue, m_Value.m_wsString.m_iLen);
            }
            std::string DoubleToString() const throw(...)
            {
                return DoubleToString(m_Value.m_dbDouble, m_iPrecision, m_iFlag & flAlternate);
            }
            std::string Int64ToString() const
            {
                return Int64ToString(m_Value.m_iInt64.m_iValue, m_Value.m_iInt64.m_iRadix,
                    m_iPrecision, m_iFlag & flUpperCase, m_iFlag & flAlternate);
            }
        };

        template<typename CData = CFormatData>
        class CFormatString
        {
        protected:
            enum EState{stNormal, stPercent, stFlag, stWidth, stDot, stPrecision, stSize, stType};
            const char *m_sBegin, *m_sEnd, *m_sCurrent;
            std::list<CData> m_lstData;
            _locale_t m_stLocal;
        protected:
            CFormatString(const char *p_sFormat)
            {
                m_sBegin = p_sFormat;
                m_sEnd = p_sFormat + strlen(p_sFormat);
                m_sCurrent = p_sFormat;

                m_stLocal = _get_current_locale();
            }

            CFormatString(const char *p_sFormat, const _locale_t &p_stLocal)
                : m_stLocal(p_stLocal)
            {
                m_sBegin = p_sFormat;
                m_sEnd = p_sFormat + strlen(p_sFormat);
                m_sCurrent = p_sFormat;
            }

            const char &GetValue() const
            {
                return *m_sCurrent; //允许读取字符串的终止字符'\0'
            }

            void MoveNext(const int p_iStep = 1)
            {
                m_sCurrent += p_iStep;
            }

            const bool IsEof() const
            {
                return m_sCurrent >= m_sEnd;
            }

            virtual EState GetState(const EState p_stLastState)
            {
                const static char cLookupTable[] =
                {
                    /* ' ' */  0x06,
                    /* '!' */  0x00,
                    /* '"' */  0x00,
                    /* '#' */  0x06,
                    /* '$' */  0x00,
                    /* '%' */  0x01,
                    /* '&' */  0x00,
                    /* ''' */  0x00,
                    /* '(' */  0x10,
                    /* ')' */  0x00,
                    /* '*' */  0x03,
                    /* '+' */  0x06,
                    /* ',' */  0x00,
                    /* '-' */  0x06,
                    /* '.' */  0x02,
                    /* '/' */  0x10,
                    /* '0' */  0x04,
                    /* '1' */  0x45,
                    /* '2' */  0x45,
                    /* '3' */  0x45,
                    /* '4' */  0x05,
                    /* '5' */  0x05,
                    /* '6' */  0x05,
                    /* '7' */  0x05,
                    /* '8' */  0x05,
                    /* '9' */  0x35,
                    /* ':' */  0x30,
                    /* ';' */  0x00,
                    /* '<' */  0x50,
                    /* '=' */  0x00,
                    /* '>' */  0x00,
                    /* '?' */  0x00,
                    /* '@' */  0x00,
                    /* 'A' */  0x28,
                    /* 'B' */  0x20,
                    /* 'C' */  0x38,
                    /* 'D' */  0x50,
                    /* 'E' */  0x58,
                    /* 'F' */  0x07,
                    /* 'G' */  0x08,
                    /* 'H' */  0x00,
                    /* 'I' */  0x37,
                    /* 'J' */  0x30,
                    /* 'K' */  0x30,
                    /* 'L' */  0x57,
                    /* 'M' */  0x50,
                    /* 'N' */  0x07,
                    /* 'O' */  0x00,
                    /* 'P' */  0x00,
                    /* 'Q' */  0x20,
                    /* 'R' */  0x20,
                    /* 'S' */  0x08,
                    /* 'T' */  0x00,
                    /* 'U' */  0x00,
                    /* 'V' */  0x00,
                    /* 'W' */  0x00,
                    /* 'X' */  0x08,
                    /* 'Y' */  0x60,
                    /* 'Z' */  0x68,
                    /* '[' */  0x60,
                    /* '\' */  0x60,
                    /* ']' */  0x60,
                    /* '^' */  0x60,
                    /* '_' */  0x00,
                    /* '`' */  0x00,
                    /* 'a' */  0x78,
                    /* 'b' */  0x70,
                    /* 'c' */  0x78,
                    /* 'd' */  0x78,
                    /* 'e' */  0x78,
                    /* 'f' */  0x78,
                    /* 'g' */  0x08,
                    /* 'h' */  0x07,
                    /* 'i' */  0x08,
                    /* 'j' */  0x00,
                    /* 'k' */  0x00,
                    /* 'l' */  0x07,
                    /* 'm' */  0x00,
                    /* 'n' */  0x08,
                    /* 'o' */  0x08,
                    /* 'p' */  0x08,
                    /* 'q' */  0x00,
                    /* 'r' */  0x00,
                    /* 's' */  0x08,
                    /* 't' */  0x00,
                    /* 'u' */  0x08,
                    /* 'v' */  0x00,
                    /* 'w' */  0x07,
                    /* 'x' */  0x08
                };

                int iClass = 0;
                char cValue = GetValue();
                if(' ' <= cValue && 'x' >= cValue)
                    iClass = cLookupTable[cValue - ' '] & 0xF;

                return (EState)(cLookupTable[iClass * (stType + 1) + p_stLastState] >> 4);
            }

            int Format(va_list p_valParams) //这里是值传递，不会改变调用方的值(其他几个函数是引用传递)
            {
                int iRet = 0;

                while(!IsEof())
                {
                    EState stState = GetState(stNormal);
                    if(DoCustomFormat(stState, p_valParams))
                    {
                        continue;
                    }

                    switch(stState)
                    {
                    case stNormal:
                        iRet = DoNormal(p_valParams);
                        break;
                    case stPercent:
                        iRet = DoPercent(p_valParams);
                        break;
                    default:
                        iRet = -1;
                    }

                    if(0 != iRet)
                        return iRet;
                }

                return 0;
            }

            virtual bool DoCustomFormat(const EState p_stState, va_list &p_valParams)
            {
                return false;
            }

            int DoNormal(va_list &p_valParams)
            {
                const char *sBegin = m_sCurrent;
                if(IsEof())
                    return 0;

                do 
                {
                    if(' ' <= *m_sCurrent && '~' >= *m_sCurrent)
                        ++m_sCurrent;
                    else if(0xA == *m_sCurrent || 0xD == *m_sCurrent)
                        ++m_sCurrent;
                    else if(_isleadbyte_l(*m_sCurrent, m_stLocal))
                        m_sCurrent += 2;
                    else
                        ++m_sCurrent;

                    if(stNormal != GetState(stNormal))
                    {
                        break;
                    }
                } while (!IsEof());

                m_lstData.push_back(CData(sBegin, m_sCurrent - sBegin));
                return 0;
            }

            int DoPercent(va_list &p_valParams)
            {
                MoveNext();

                int iFlag = 0, iWidth = 0, iPrecision = -1;
                EState st = stPercent;
                while(!IsEof())
                {
                    int iRet = 0;
                    st = GetState(st);

                    switch(st)
                    {
                    case stFlag:
                        iRet = DoDealFlag(iFlag, p_valParams);
                        break;
                    case stWidth:
                        iRet = DoDealWidth(iWidth, iFlag, p_valParams);
                        break;
                    case stDot:
                        iPrecision = 0;
                        MoveNext();
                        break;
                    case stPrecision:
                        iRet = DoDealPrecision(iPrecision, p_valParams);
                        break;
                    case stSize:
                        {
                            bool bContinue = true;
                            iRet = DoDealSize(bContinue, iFlag, p_valParams);
                            if(!bContinue)
                                return 0;

                            break;
                        }
                    case stType:
                        return DoDealType(iFlag, iWidth, iPrecision, p_valParams);
                    case stNormal:
                        m_lstData.push_back(CData(&GetValue(), 1)); //TODO: 除了“%%”之外，是否还有其他Unicode字符会导致这种情况有待研究
                        MoveNext();
                        return 0;
                    default:
                        return 0; //如果“%”后面的内容不是一个完整的格式定义，则跳过相关处理
                    }

                    if(0 != iRet)
                        return iRet;
                }
                return 0;
            }

            int DoDealFlag(int &p_iFlag, va_list &p_valParams)
            {
                switch (GetValue())
                {
                case '-':
                    p_iFlag |= flLeft;
                    break;
                case '+':
                    p_iFlag |= flSign;
                    break;
                case ' ':
                    p_iFlag |= flSignsp;
                    break;
                case '#':
                    p_iFlag |= flAlternate;
                    break;
                case '0':
                    p_iFlag |= flLeadZero;
                    break;
                }

                MoveNext();
                return 0;
            }

            int DoDealWidth(int &p_iWidth, int &p_iFlag, va_list &p_valParams)
            {
                char cValue = GetValue();
                if('*' == cValue)
                {
                    p_iWidth = va_arg(p_valParams, int);
                    if(0 > p_iWidth)
                    {
                        p_iFlag |= flLeft;
                        p_iWidth = -p_iWidth;
                    }
                }
                else
                {
                    p_iWidth = p_iWidth * 10 + (cValue - '0');
                }

                MoveNext();
                return 0;
            }

            int DoDealPrecision(int &p_iPrecision, va_list &p_valParams)
            {
                char cValue = GetValue();
                if('*' == cValue)
                {
                    p_iPrecision = va_arg(p_valParams, int);
                    if(0 > p_iPrecision)
                    {
                        p_iPrecision = -1;
                    }
                }
                else
                {
                    p_iPrecision = p_iPrecision * 10 + (cValue - '0');
                }

                MoveNext();
                return 0;
            }

            int DoDealSize(bool &p_bContinue, int &p_iFlag, va_list &p_valParams)
            {
                p_bContinue = true;
                switch(GetValue())
                {
                case 'l':
                    if('l' == m_sCurrent[1])
                    {
                        p_iFlag |= flLongLong;
                        MoveNext(2);
                        return 0;
                    }
                    else
                    {
                        p_iFlag |= flLong;
                    }

                    break;
                case 'I':
                    if('6' == m_sCurrent[1] && '4' == m_sCurrent[2])
                    {
                        p_iFlag |= flI64;
                        MoveNext(3);
                        return 0;
                    }
                    else if('3' == m_sCurrent[1] && '2' == m_sCurrent[2])
                    {
                        p_iFlag |= ~flI64;
                        MoveNext(3);
                        return 0;
                    }
                    else
                    {
                        char cValue = m_sCurrent[1];
                        if('d' == cValue || 'i' == cValue || 'o' == cValue
                            || 'u' == cValue || 'x' == cValue || 'X' == cValue)
                        {
                            //什么也不做，按数字类型解析后面的内容
                        }
                        else
                        {
                            p_bContinue = false;
                        }
                    }

                    break;
                case 'h':
                    p_iFlag |= flShort;
                    break;
                case 'w':
                    p_iFlag |= flWideChar;
                    break;
                }

                MoveNext();
                return 0;
            }

            int DoDealType(const int p_iFlag, const int p_iWidth, const int p_iPrecision, va_list &p_valParams)
            {
                int iFlag = p_iFlag;

                switch(GetValue())
                {
                case 'Z':
                case 'n':
                case 'E':
                case 'G':
                case 'A':
                case 'e':
                case 'g':
                case 'a':
                    return -1;
                case 'C':
                    if (!(iFlag & (flShort | flLong | flWideChar)))
                        iFlag |= flWideChar;
                    //继续执行
                case 'c':
                    if(iFlag & (flLong | flWideChar))
                    {
                        wchar_t wcValue = va_arg(p_valParams, wchar_t);
                        m_lstData.push_back(CData(wcValue, iFlag, p_iWidth, p_iPrecision));
                    }
                    else
                    {
                        char cValue = (char)(va_arg(p_valParams, int));
                        m_lstData.push_back(CData(cValue, iFlag, p_iWidth, p_iPrecision));
                    }

                    break;
                case 'S':
                    if (!(iFlag & (flShort | flLong | flWideChar)))
                        iFlag |= flWideChar;
                    //继续执行
                case 's':
                    if(iFlag & (flLong | flWideChar))
                    {
                        const wchar_t *wsValue = va_arg(p_valParams, const wchar_t *);
                        m_lstData.push_back(CData(wsValue, iFlag, p_iWidth, p_iPrecision));
                    }
                    else
                    {
                        const char *sValue = va_arg(p_valParams, const char *);
                        m_lstData.push_back(CData(sValue, iFlag, p_iWidth, p_iPrecision));
                    }

                    break;
                case 'f':
                    {
                        iFlag |= flSigned;
                        double dbValue = va_arg(p_valParams, double);
                        if(0 > dbValue) //原逻辑对负数的处理，是放在将浮点数转换为字符串之后处理的，新的逻辑放在转换之前，保持代码的统一
                                        //从对_fcvt的测试结果看，不管是正数还是负数，四舍五入都是按绝对值较大的方向处理
                        {
                            dbValue = -dbValue;
                            iFlag |= flNegative;
                        }
                        m_lstData.push_back(CData(dbValue, iFlag, p_iWidth, (p_iPrecision < 0 ? 6 : p_iPrecision)));
                    }

                    break;
                case 'd':
                case 'i':
                    iFlag |= flSigned;
                    //继续执行
                case 'u':
                    m_lstData.push_back(CData(p_valParams, 10, iFlag, p_iWidth, p_iPrecision));

                    break;
                case 'p':
                    iFlag |= flLong | flUpperCase;
                    m_lstData.push_back(CData(p_valParams, 16, iFlag, p_iWidth, 2 * sizeof(void *)));

                    break;
                case 'X':
                    iFlag |= flUpperCase;
                    //继续执行
                case 'x':
                    m_lstData.push_back(CData(p_valParams, 16, iFlag, p_iWidth, p_iPrecision));

                    break;
                case 'o':
                    m_lstData.push_back(CData(p_valParams, 8, iFlag, p_iWidth, p_iPrecision));

                    break;
                }

                MoveNext();
                return 0;
            }

            std::string ToString() const
            {
                std::string strResult;
                for(std::list<CData>::const_iterator it = m_lstData.begin(); it != m_lstData.end(); ++it)
                    strResult.append(it->ToString());

                return strResult;
            }
        public:
            static const std::string Format(const char *p_sFormat, ...) throw(...)
            {
                CFormatString fs(p_sFormat);

                va_list valParams = NULL;
                va_start(valParams, p_sFormat);

                if(0 != fs.Format(valParams))
                    throw "格式化失败";

                return fs.ToString();
            }
        };
    }
}