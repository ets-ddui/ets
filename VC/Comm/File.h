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
#include <Shlwapi.h>

#if defined(WIN32)
    #include <Windows.h>
#else
    #include <iconv.h>
#endif

namespace vcl4c
{
    namespace file
    {
        inline const bool IsDir(const std::string &p_sPath)
        {
            DWORD iAttr = ::GetFileAttributes(p_sPath.c_str());
            return (iAttr != INVALID_FILE_ATTRIBUTES) && (iAttr & FILE_ATTRIBUTE_DIRECTORY);
        }

        inline void Split(std::string &p_sPath, std::string &p_sName, const std::string &p_sFullName)
        {
            std::string::size_type nPos = p_sFullName.find_last_of("\\/");
            if (nPos == std::string::npos)
            {
                p_sPath.erase();
                p_sName = p_sFullName;
            }
            else
            {
                /* TODO: 针对根目录、网络路径、路径间包含多个分隔符的情况有待完善 */
                p_sPath = p_sFullName.substr(0, nPos);
                p_sName = p_sFullName.substr(nPos + 1);
            }
        }

        inline void CreateFilePath(const char *p_sDirectory)
        {
            if(!::PathFileExistsA(p_sDirectory))
            {
                //创建父目录
                std::string sParentPath, sDirectoryName;
                Split(sParentPath, sDirectoryName, p_sDirectory);
                if (!sParentPath.empty())
                {
                    CreateFilePath(sParentPath.c_str());
                }

                //创建当前目录
                ::CreateDirectoryA(p_sDirectory, NULL);
            }
        }

        inline const bool FileExists(const std::string &p_sFile)
        {
            DWORD iAttr = ::GetFileAttributes(p_sFile.c_str());
            if (iAttr != INVALID_FILE_ATTRIBUTES)
            {
                return 0 == (iAttr & FILE_ATTRIBUTE_DIRECTORY);
            }
            else
            {
                DWORD iLastError = ::GetLastError();

                return (ERROR_FILE_NOT_FOUND != iLastError)
                    && (ERROR_PATH_NOT_FOUND != iLastError)
                    && (ERROR_INVALID_NAME != iLastError)
                    && [] (const std::string &p_sFile) throw() -> bool
                    {
                        WIN32_FIND_DATA fd = {0};
                        HANDLE hFindFirst = ::FindFirstFile(p_sFile.c_str(), &fd);
                        if (INVALID_HANDLE_VALUE != hFindFirst)
                        {
                            ::FindClose(hFindFirst);

                            return 0 == (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
                        }

                        return false;
                    }(p_sFile);
            }
        }

        inline const bool Read(std::string &p_sResult, const std::string &p_sFile)
        {
            FILE *f = ::fopen(p_sFile.c_str(), "rb");
            if (nullptr == f)
            {
                return false;
            }

            ::fseek(f, 0, SEEK_END);
            long iSize = ::ftell(f);
            if (iSize < 0)
            {
                ::fclose(f);
                return false;
            }
            ::fseek(f, 0, SEEK_SET);

            p_sResult.resize(iSize);
            long iRead = ::fread(const_cast<char *>(p_sResult.c_str()), 1, iSize, f);
            ::fclose(f);

            return true;
        }

        inline const bool Write(std::string &p_sValue, const std::string &p_sFile, const bool p_bAppend = true)
        {
            ::std::string sPath, sName;
            Split(sPath, sName, p_sFile);
            if (!sPath.empty() && "." != sPath)
            {
                CreateFilePath(sPath.c_str());
            }

            FILE *pFile = ::fopen(p_sFile.c_str(), p_bAppend ? "a+b" : "wb");
            if (nullptr == pFile)
            {
                return false;
            }

            size_t iSize = fwrite(p_sValue.c_str(), 1, p_sValue.size(), pFile);
            if (p_sValue.size() != iSize)
            {
                fclose(pFile);
                pFile = nullptr;

                return false;
            }

            fclose(pFile);
            pFile = nullptr;

            return true;
        }

    }
}
