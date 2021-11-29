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

#if defined(WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include <odbcss.h>
#include <vector>
#include <list>
#include <stdexcept>
#include <string>
#include "../Core.h"
#include "../StringHelper.h"
#include "DataSet.h"

#pragma comment(lib, "sqlncli11")

namespace vcl4c
{
    namespace db
    {
        class CODBC
            : public NonCopyable
        {
        private:
            class COdbcDataSet
                : public CDataSet
            {
            public:
                COdbcDataSet(CODBC * p_Parent)
                    : m_Parent(p_Parent)
                {
                }

                SQLRETURN BindStmt(const SQLHSTMT p_hStmt, const SQLSMALLINT p_iColumnCount)
                {
                    SQLRETURN iRetcode = 0;
                    ClearField();

                    iRetcode = RetrieveFieldInfo(p_hStmt, p_iColumnCount);
                    if (0 > iRetcode)
                    {
                        return iRetcode;
                    }

                    iRetcode = BindField(p_hStmt);
                    if (0 > iRetcode)
                    {
                        return iRetcode;
                    }

                    iRetcode = RetrieveData(p_hStmt);
                    if (0 > iRetcode)
                    {
                        return iRetcode;
                    }

                    return SQL_SUCCESS;
                }

            private:
                SQLRETURN RetrieveFieldInfo(const SQLHSTMT p_hStmt, const SQLSMALLINT p_iColumnCount)
                {
                    SQLRETURN iRetcode = 0;

                    for (SQLSMALLINT iCol = 1; iCol <= p_iColumnCount; ++iCol)
                    {
                        // 1.0 获取字段类型(字符串左对齐显示，数字右对齐显示)
                        SQLLEN iType = 0;
                        iRetcode = SQLColAttribute(p_hStmt, iCol, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &iType);
                        GetDiagnosticMessage(p_hStmt, SQL_HANDLE_STMT, iRetcode);
                        if (0 > iRetcode)
                        {
                            return iRetcode;
                        }

                        // 2.0 获取数据长度
                        SQLLEN iSize = 0;
                        iRetcode = SQLColAttribute(p_hStmt, iCol, SQL_DESC_DISPLAY_SIZE, NULL, 0, NULL, &iSize);
                        GetDiagnosticMessage(p_hStmt, SQL_HANDLE_STMT, iRetcode);
                        if (0 > iRetcode)
                        {
                            return iRetcode;
                        }
                        ++iSize; //增加字符串结尾符

                        // 3.0 获取列名
                        SQLSMALLINT iFieldNameLen = 0;
                        iRetcode = SQLColAttribute(p_hStmt, iCol, SQL_DESC_NAME, NULL, 0, &iFieldNameLen, NULL);
                        //在Linux中，会提示"Data truncated"，推测和传入NULL地址有关，屏蔽这类信息
                        if (SQL_SUCCESS_WITH_INFO != iRetcode && SQL_SUCCESS != iRetcode)
                        {
                            GetDiagnosticMessage(p_hStmt, SQL_HANDLE_STMT, iRetcode);
                        }

                        if (0 > iRetcode)
                        {
                            return iRetcode;
                        }

                        ++iFieldNameLen;
                        std::string sName;
                        sName.resize(iFieldNameLen);

                        iRetcode = SQLColAttribute(p_hStmt, iCol, SQL_DESC_NAME,
                            (SQLPOINTER)sName.c_str(), (SQLSMALLINT)sName.size(), NULL, NULL);
                        GetDiagnosticMessage(p_hStmt, SQL_HANDLE_STMT, iRetcode);
                        if (0 > iRetcode)
                        {
                            return iRetcode;
                        }

                        AddField(sName.c_str(), iSize); //sName后面可能会有NULL字符，用sName.c_str()重新构造std::string，可去除
                    }

                    return SQL_SUCCESS;
                }

                SQLRETURN BindField(const HSTMT p_hStmt)
                {
                    SQLRETURN iRetcode = 0;

                    m_sBuffer.resize(GetRecordSize());
                    m_vIndicator.resize(GetFieldCount());

                    SQLSMALLINT iCol = 1;
                    for (unsigned int i = 0; i < GetFieldCount(); ++i)
                    {
                        CFieldHelper fh = GetField(i);

                        iRetcode = SQLBindCol(p_hStmt, iCol++, SQL_C_TCHAR,
                            (SQLPOINTER)&m_sBuffer[fh.GetOffset()], fh.GetSize(), &m_vIndicator[i]);
                        GetDiagnosticMessage(p_hStmt, SQL_HANDLE_STMT, iRetcode);
                        if (0 > iRetcode)
                        {
                            return iRetcode;
                        }
                    }

                    return SQL_SUCCESS;
                }

                SQLRETURN RetrieveData(const SQLHSTMT p_hStmt)
                {
                    SQLRETURN iRetcode = 0;

                    while (true)
                    {
                        iRetcode = SQLFetch(p_hStmt);
                        if (SQL_NO_DATA == iRetcode)
                        {
                            First();
                            return 0;
                        }

                        GetDiagnosticMessage(p_hStmt, SQL_HANDLE_STMT, iRetcode);
                        if (0 > iRetcode)
                        {
                            First();
                            return iRetcode;
                        }

                        AppendRawData(m_sBuffer);
                        for (unsigned int i = 0; i < m_vIndicator.size(); ++i)
                        {
                            if (SQL_NULL_DATA == m_vIndicator[i])
                            {
                                SetNull(i);
                            }
                            else
                            {
                                EraseNull(i);
                            }
                        }
                    }

                    return 0;
                }

                void GetDiagnosticMessage(const SQLHANDLE p_hHandle, const SQLSMALLINT p_hType, const RETCODE p_iRetcode)
                {
                    m_Parent->GetDiagnosticMessage(p_hHandle, p_hType, p_iRetcode);
                }

            private:
                CODBC *m_Parent;
                std::string m_sBuffer;
                std::vector<SQLLEN> m_vIndicator;
            };

        public:
            CODBC()
                : m_hEnv(NULL), m_hDbc(NULL)
            {

            }

            virtual ~CODBC()
            {
                DisConnect();

                if (m_hEnv)
                {
                    SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
                    m_hEnv = NULL;
                }
            }

            SQLRETURN Connect(const std::string &p_sDsn, const std::string &p_sUser, const std::string &p_sPassword)
            {
                SQLRETURN iRetcode = SQL_SUCCESS;
                m_sMessage.clear();

                //1.0 分配环境句柄
                if (NULL == m_hEnv)
                {
                    iRetcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
                    if (0 > iRetcode)
                    {
                        return iRetcode;
                    }

                    iRetcode = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
                    GetDiagnosticMessage(m_hEnv, SQL_HANDLE_ENV, iRetcode);
                    if (0 > iRetcode)
                    {
                        SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
                        m_hEnv = NULL;

                        return iRetcode;
                    }
                }

                //2.0 分配连接句柄
                if (NULL == m_hDbc)
                {
                    iRetcode = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
                    GetDiagnosticMessage(m_hEnv, SQL_HANDLE_ENV, iRetcode);
                    if (0 > iRetcode)
                    {
                        return iRetcode;
                    }

                    //开启BCP的开关
                    iRetcode = SQLSetConnectAttr(m_hDbc, SQL_COPT_SS_BCP, (SQLPOINTER)SQL_BCP_ON, SQL_IS_INTEGER);
                    GetDiagnosticMessage(m_hDbc, SQL_HANDLE_DBC, iRetcode);
                    if (0 > iRetcode)
                    {
                        SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
                        m_hDbc = NULL;

                        return iRetcode;
                    }

                    iRetcode = SQLConnect(m_hDbc,
                        (SQLCHAR *)p_sDsn.c_str(), (SQLSMALLINT)p_sDsn.size(),
                        (SQLCHAR *)p_sUser.c_str(), (SQLSMALLINT)p_sUser.size(),
                        (SQLCHAR *)p_sPassword.c_str(), (SQLSMALLINT)p_sPassword.size());
                    GetDiagnosticMessage(m_hDbc, SQL_HANDLE_DBC, iRetcode);
                    if (0 > iRetcode)
                    {
                        SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
                        m_hDbc = NULL;

                        return iRetcode;
                    }
                }

                return SQL_SUCCESS;
            }

            void DisConnect()
            {
                m_sMessage.clear();

                for (std::list<SQLHSTMT>::const_iterator it = m_lStmt.begin(); it != m_lStmt.end(); ++it)
                {
                    SQLFreeHandle(SQL_HANDLE_STMT, *it);
                }
                m_lStmt.clear();

                if (m_hDbc)
                {
                    SQLDisconnect(m_hDbc);
                    SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
                    m_hDbc = NULL;
                }
            }

            //Execute以阻塞方式执行SQL语句，并获取返回结果
            //返回值含义：
            //    < 0：执行出错
            SQLRETURN Execute(const std::string &p_sSQL, const bool p_bDiscardResult = false)
            {
                SQLRETURN iRetcode = SQL_SUCCESS;
                m_sMessage.clear();
                m_lDataSet.clear();

                //1.0 分配语句执行句柄
                SQLHSTMT hStmt = NULL;
                iRetcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &hStmt);
                GetDiagnosticMessage(m_hDbc, SQL_HANDLE_DBC, iRetcode);
                if (0 > iRetcode)
                {
                    return iRetcode;
                }
                m_lStmt.push_back(hStmt);

                //2.0 执行SQL语句
                iRetcode = SQLExecDirect(hStmt, (SQLCHAR *)p_sSQL.c_str(), SQL_NTS);
                GetDiagnosticMessage(hStmt, SQL_HANDLE_STMT, iRetcode);
                if (0 > iRetcode)
                {
                    return iRetcode;
                }

                //3.0 解析结果
                if (!p_bDiscardResult)
                {
                    do
                    {
                        SQLSMALLINT iColumnCount = 0;
                        iRetcode = SQLNumResultCols(hStmt, &iColumnCount);
                        GetDiagnosticMessage(hStmt, SQL_HANDLE_STMT, iRetcode);
                        if (0 > iRetcode)
                        {
                            return iRetcode;
                        }

                        if (0 >= iColumnCount)
                        {
                            //3.1 没有结果集，获取影响行数
                            SQLLEN iAffectCount = 0;
                            iRetcode = SQLRowCount(hStmt, &iAffectCount);
                            GetDiagnosticMessage(hStmt, SQL_HANDLE_STMT, iRetcode);
                            if (0 > iRetcode)
                            {
                                return iRetcode;
                            }

                            if (0 <= iAffectCount)
                            {
                                m_sMessage.push_back(vcl4c::string::Format("(%ld) row(s) affected", iAffectCount));
                            }
                        }
                        else
                        {
                            //3.2 获取结果集数据
                            m_lDataSet.push_back(COdbcDataSet(this));
                            iRetcode = m_lDataSet.back().BindStmt(hStmt, iColumnCount);
                            if (0 > iRetcode)
                            {
                                return iRetcode;
                            }
                        }
                    } while (SQL_SUCCESS == SQLMoreResults(hStmt));
                }

                //4.0 释放资源
                m_lStmt.pop_back();
                iRetcode = SQLFreeStmt(hStmt, SQL_CLOSE);
                GetDiagnosticMessage(hStmt, SQL_HANDLE_STMT, iRetcode);
                if (0 > iRetcode)
                {
                    return iRetcode;
                }

                return SQL_SUCCESS;
            }

            SQLRETURN Bcp(CDataSetBase * p_dsData, const std::string & p_sDB, const std::string & p_sTable,
                const std::string & p_sRelation = "", const bool p_bDropData = true)
            {
                SQLRETURN iRetCode = SQL_SUCCESS;
                m_sMessage.clear();

                //1.0 清空存量数据
                if (p_bDropData)
                {
                    iRetCode = Execute(vcl4c::string::Format("truncate table %s.dbo.%s", p_sDB.c_str(), p_sTable.c_str()), true);
                    if (0 > iRetCode)
                    {
                        return iRetCode;
                    }
                }

                //2.0 bcp初始化
                //2.1 获取列信息
                iRetCode = Execute(vcl4c::string::Format("select b.name, b.xtype, b.prec, b.colid "
                    "    from %s.dbo.sysobjects a inner join %s.dbo.syscolumns b on a.id = b.id "
                    "    where a.id = object_id('%s.dbo.%s')",
                    p_sDB.c_str(), p_sDB.c_str(),
                    p_sDB.c_str(), p_sTable.c_str()));
                if (0 > iRetCode)
                {
                    return iRetCode;
                }

                if (1 != GetDataSetCount())
                {
                    m_sMessage.push_back("表字段信息获取失败");
                    return SQL_ERROR;
                }

                //2.2 声明存储字段信息的结构变量
                class CBcpStruct
                    : public CStruct
                {
                public:
                    virtual char * GetAddress(const int p_iFieldIndex) const
                    {
                        return __super::GetAddress(p_iFieldIndex);
                    }
                } bcpRow;

                std::vector<int> vType;
                std::vector<int> vColid;
                CDataSetBase * ds = GetDataSet(0);
                ds->First();
                while (!ds->IsEof())
                {
                    bcpRow.AddField(ds->GetField("name").Get<std::string>(), ds->GetField("prec").Get<int>());
                    vType.push_back(ds->GetField("xtype").Get<int>());
                    vColid.push_back(ds->GetField("colid").Get<int>());

                    ds->Next();
                }

                //2.3 绑定bcp表名
                std::string sTable = vcl4c::string::Format("%s.dbo.%s", p_sDB.c_str(), p_sTable.c_str());
                if (SUCCEED != bcp_init(m_hDbc, sTable.c_str(), NULL, NULL, DB_IN))
                {
                    m_sMessage.push_back(vcl4c::string::Format("bcp_init(%s)执行失败", sTable.c_str()));
                    return SQL_ERROR;
                }

                //2.4 bcp参数初始化
                //只要有一个错误，就触发bcp失败(默认在出现10个错误时，才会导致bcp失败)
                if (SUCCEED != bcp_control(m_hDbc, BCPMAXERRS, (void *)1))
                {
                    m_sMessage.push_back("bcp_control(BCPMAXERRS)执行失败");
                    return SQL_ERROR;
                }

                //bcp过程中，使用表锁，提升性能(默认用的是行锁)
                SQLTCHAR sTableLock[] = "TABLOCK";
                if (SUCCEED != bcp_control(m_hDbc, BCPHINTS, (void *)sTableLock))
                {
                    m_sMessage.push_back("bcp_control(BCPHINTS)执行失败");
                    return SQL_ERROR;
                }

                //2.5 绑定每个字段的内存地址
                char * sTerminator = "";
                for (unsigned int i = 0; i < bcpRow.GetFieldCount(); ++i)
                {
                    if (SUCCEED != bcp_bind(m_hDbc,
                        reinterpret_cast<BYTE *>(bcpRow.GetAddress(i)), 0, bcpRow.GetField(i).GetSize(),
                        reinterpret_cast<BYTE *>(sTerminator), 1, SQLCHARACTER, vColid[i]))
                    {
                        m_sMessage.push_back(vcl4c::string::Format("bcp_bind(%s, %s)执行失败",
                            sTable.c_str(), bcpRow.GetField(i).GetName().c_str()));
                        return SQL_ERROR;
                    }
                }

                //2.6 绑定每个字段与入参数据集字段的对应关系
                std::vector<int> vRelation;
                for (unsigned int i = 0; i < bcpRow.GetFieldCount(); ++i)
                {
                    vRelation.push_back(p_dsData->IndexOfField(bcpRow.GetField(i).GetName()));
                }

                std::map<std::string, std::string> mapRelation;
                vcl4c::string::SplitString(mapRelation, p_sRelation, '=', '&');
                for (std::map<std::string, std::string>::const_iterator it = mapRelation.begin();
                    it != mapRelation.end(); ++it)
                {
                    int iIndex = bcpRow.IndexOfField(it->second);
                    if (iIndex < 0)
                    {
                        continue;
                    }

                    vRelation[iIndex] = p_dsData->IndexOfField(it->first);
                }

                //3.0 转入数据
                int iRecordRec = 0;
                p_dsData->First();
                while (!p_dsData->IsEof())
                {
                    ++iRecordRec;
                    for (unsigned int i = 0; i < bcpRow.GetFieldCount(); ++i)
                    {
                        if (0 <= vRelation[i])
                        {
                            std::string strValue = p_dsData->GetField(vRelation[i]).Get<std::string>();
                            if (!strValue.empty())
                            {
                                if (bcpRow.GetField(i).GetSize() < strValue.size())
                                {
                                    m_sMessage.push_back(vcl4c::string::Format("bcp_sendrow(%s)执行失败，数据内容过长，行列号：%d %d(从1开始计数)",
                                        sTable.c_str(), iRecordRec, i + 1));
                                    return SQL_ERROR;
                                }

                                bcpRow.GetField(i).Set<std::string>(strValue);
                                continue;
                            }
                        }

                        switch(vType[i])
                        {
                        case SQLINTN:
                        case SQLINT1:
                        case SQLBIT:
                        case SQLINT2:
                        case SQLINT4:
                        case SQLMONEY:
                        case SQLDATETIME:
                        case SQLFLT8:
                        case SQLFLTN:
                        case SQLMONEYN:
                        case SQLDATETIMN:
                        case SQLFLT4:
                        case SQLMONEY4:
                        case SQLDATETIM4:
                        case SQLDECIMAL:
                        case SQLNUMERIC:
                        case SQLBITN:
                        case SQLINT8:
                            bcpRow.GetField(i).Set<std::string>("0");
                            break;
                        default:
                            bcpRow.GetField(i).Set<std::string>(" ");
                        }
                    }

                    if (SUCCEED != bcp_sendrow(m_hDbc))
                    {
                        m_sMessage.push_back(vcl4c::string::Format("bcp_sendrow(%s)执行失败，行号：%d(从1开始计数)",
                            sTable.c_str(), iRecordRec));
                        return SQL_ERROR;
                    }

                    p_dsData->Next();
                }

                //4.0 事务提交
                if (-1 == bcp_done(m_hDbc))
                {
                    m_sMessage.push_back(vcl4c::string::Format("bcp_done(%s)执行失败", sTable.c_str()));
                    return SQL_ERROR;
                }

                return SQL_SUCCESS;
            }

            int GetMessageCount() const
            {
                return m_sMessage.size();
            }

            std::string GetMessage(int p_iIndex) const
            {
                if (p_iIndex < 0 || (unsigned int)p_iIndex >= m_sMessage.size())
                {
                    return "";
                }

                return m_sMessage[p_iIndex];
            }

            int GetDataSetCount() const
            {
                return m_lDataSet.size();
            }

            CDataSetBase * GetDataSet(int p_iIndex)
            {
                if (p_iIndex < 0 || (unsigned int)p_iIndex >= m_lDataSet.size())
                {
                    return NULL;
                }

                std::list<COdbcDataSet>::iterator it = m_lDataSet.begin();
                for (int i = 0; i < p_iIndex; ++i)
                {
                    ++it;
                }

                return &(*it);
            }

        protected:
            void GetDiagnosticMessage(const SQLHANDLE p_hHandle, const SQLSMALLINT p_hType, const RETCODE p_iRetcode)
            {
                if (SQL_SUCCESS == p_iRetcode)
                {
                    return;
                }

                if (SQL_INVALID_HANDLE == p_iRetcode)
                {
                    m_sMessage.push_back("Invalid handle");
                    return;
                }

                SQLINTEGER iError;
                char szMessage[1000];
                char szState[SQL_SQLSTATE_SIZE+1];

                SQLSMALLINT iRec = 0;
                while (SQL_SUCCESS == SQLGetDiagRec(p_hType, p_hHandle, ++iRec, (SQLCHAR *)szState, &iError,
                    (SQLCHAR *)szMessage, (SQLSMALLINT)(sizeof(szMessage) / sizeof(char)), (SQLSMALLINT *)NULL))
                {
                    // Hide data truncated..
                    //if (strncmp(szState, "01004", 5))
                    m_sMessage.push_back(vcl4c::string::Format("[%5.5s](%d) %s", szState, iError, szMessage));
                }
            }

        private:
            SQLHENV m_hEnv;
            SQLHDBC m_hDbc;
            std::vector<std::string> m_sMessage;
            std::list<SQLHSTMT> m_lStmt;
            std::list<COdbcDataSet> m_lDataSet;
        };
    }
}
