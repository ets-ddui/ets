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

#include <boost/atomic.hpp>

template<std::size_t c_iSize>
class CRingBuffer
{
    CRingBuffer(const CRingBuffer &p_rbBuffer); //= delete;
    CRingBuffer &operator =(const CRingBuffer &p_rbBuffer); //= delete;

public:
    CRingBuffer()
        : m_iBegin(0), m_iEnd(0)
    {

    }

    std::size_t Write(const unsigned char *p_sData, std::size_t p_iLen)
    {
        if (0 == p_iLen)
        {
            return 0;
        }

        std::size_t iBegin = m_iBegin.load(boost::memory_order_consume);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_consume); //这里可以用memory_order_relaxed，因为m_iEnd只会由Write修改

        std::size_t iSize = p_iLen;

        //剩余空闲空间与p_iLen取小
        if (c_iSize - GetSize(iBegin, iEnd) < iSize)
        {
            iSize = c_iSize - GetSize(iBegin, iEnd);
        }

        if (0 == iSize)
        {
            return iSize;
        }

        //如果结束索引回绕，则只取到缓存终止的位置(回绕的空间下次循环再取，以保证每次操作的都是连续内存)
        if (c_iSize - GetOffset(iEnd) < iSize)
        {
            iSize = c_iSize - GetOffset(iEnd);
        }

        //数据拷贝
        memcpy(m_sBuffer + GetOffset(iEnd), p_sData, iSize);

        //指针修正
        iEnd += iSize;
        if (iEnd >= 2 * c_iSize)
        {
            iEnd -= 2 * c_iSize;
        }
        m_iEnd.store(iEnd, boost::memory_order_release);

        return iSize;
    }

    //读取数据缓存
    std::vector<boost::asio::const_buffer> GetDate() const
    {
        std::size_t iBegin = m_iBegin.load(boost::memory_order_relaxed);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_consume);
        std::size_t iSize = GetSize(iBegin, iEnd); //有效数据尺寸
        if (0 == iSize)
        {
            return std::vector<boost::asio::const_buffer>();
        }

        return GetRange<std::vector<boost::asio::const_buffer>>(GetOffset(iBegin), iSize);
    }

    //读取空闲区域缓存
    std::vector<boost::asio::mutable_buffer> GetBuffer()
    {
        std::size_t iBegin = m_iBegin.load(boost::memory_order_consume);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_relaxed);
        std::size_t iSize = c_iSize - GetSize(iBegin, iEnd); //空闲区域尺寸
        if (0 == iSize)
        {
            return std::vector<boost::asio::mutable_buffer>();
        }

        return GetRange<std::vector<boost::asio::mutable_buffer>>(GetOffset(iEnd), iSize);
    }

    //写入数据后，增加数据有效长度
    void Append(const std::size_t p_iSize)
    {
        Increase(m_iEnd, p_iSize);
    }

    //读出数据后，缩减有效数据范围
    void Erase(const std::size_t p_iSize)
    {
        Increase(m_iBegin, p_iSize);
    }

    void Clear()
    {
        m_iBegin.store(0, boost::memory_order_relaxed);
        m_iEnd.store(0, boost::memory_order_relaxed);
    }

    bool IsEmpty() const
    {
        std::size_t iBegin = m_iBegin.load(boost::memory_order_relaxed);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_relaxed);
        return 0 == GetSize(iBegin, iEnd);
    }

    bool IsFull() const
    {
        std::size_t iBegin = m_iBegin.load(boost::memory_order_relaxed);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_relaxed);
        return c_iSize == GetSize(iBegin, iEnd);
    }

private:
    unsigned char m_sBuffer[c_iSize];
    //m_iBegin、m_iEnd的取值范围为[0, 2 * p_iSize)，之所以未选择区间[0, p_iSize)，
    //是因为后者无法区分“缓存为空”和“全部填满”的情况，这种情况下“m_iBegin == m_iEnd”均成立
    boost::atomic<std::size_t> m_iBegin, m_iEnd;

    std::size_t GetSize(std::size_t p_iBegin, std::size_t p_iEnd) const
    {
        return (p_iBegin <= p_iEnd) ? (p_iEnd - p_iBegin) : (2 * c_iSize + p_iEnd - p_iBegin);
    }

    std::size_t GetOffset(int p_iPos) const
    {
        return (p_iPos >= c_iSize) ? (p_iPos - c_iSize) : p_iPos;
    }

    template<typename CBuffer>
    CBuffer GetRange(const std::size_t p_iIndex, const std::size_t p_iSize) const
    {
        unsigned char *sBuffer = const_cast<unsigned char *>(m_sBuffer);
        CBuffer vData;
        if (c_iSize - p_iIndex < p_iSize)
        {
            vData.push_back(boost::asio::buffer(sBuffer + p_iIndex, c_iSize - p_iIndex));
            vData.push_back(boost::asio::buffer(sBuffer, p_iSize - (c_iSize - p_iIndex)));
        }
        else
        {
            vData.push_back(boost::asio::buffer(sBuffer + p_iIndex, p_iSize));
        }

        return vData;
    }

    void Increase(boost::atomic<std::size_t> &p_iIndex, const std::size_t p_iSize)
    {
        std::size_t iIndex = p_iIndex.load(boost::memory_order_relaxed);
        iIndex += p_iSize;
        if (iIndex >= 2 * c_iSize)
        {
            iIndex -= 2 * c_iSize;
        }

        p_iIndex.store(iIndex, boost::memory_order_consume);
    }

};