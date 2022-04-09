/*
    Copyright (c) 2021-2031 Steven Shi

    ETS(Extended Tool Set)������չ���߼���

    ����������ǿ�Դ������������������� MIT Э�飬�޸ĺͷ����˳���
    �����˿��Ŀ����ϣ�������ã��������κα�֤��
    ���������������ҵ��Ŀ�����ڱ����е�Bug����������κη��ռ���ʧ�������߲��е��κ����Ρ�

    ��Դ��ַ: https://github.com/ets-ddui/ets
              https://gitee.com/ets-ddui/ets
    ��ԴЭ��: The MIT License (MIT)
    ��������: xinghun87@163.com
    �ٷ����ͣ�https://blog.csdn.net/xinghun61
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
        std::size_t iEnd = m_iEnd.load(boost::memory_order_consume); //���������memory_order_relaxed����Ϊm_iEndֻ����Write�޸�

        std::size_t iSize = p_iLen;

        //ʣ����пռ���p_iLenȡС
        if (c_iSize - GetSize(iBegin, iEnd) < iSize)
        {
            iSize = c_iSize - GetSize(iBegin, iEnd);
        }

        if (0 == iSize)
        {
            return iSize;
        }

        //��������������ƣ���ֻȡ��������ֹ��λ��(���ƵĿռ��´�ѭ����ȡ���Ա�֤ÿ�β����Ķ��������ڴ�)
        if (c_iSize - GetOffset(iEnd) < iSize)
        {
            iSize = c_iSize - GetOffset(iEnd);
        }

        //���ݿ���
        memcpy(m_sBuffer + GetOffset(iEnd), p_sData, iSize);

        //ָ������
        iEnd += iSize;
        if (iEnd >= 2 * c_iSize)
        {
            iEnd -= 2 * c_iSize;
        }
        m_iEnd.store(iEnd, boost::memory_order_release);

        return iSize;
    }

    //��ȡ���ݻ���
    std::vector<boost::asio::const_buffer> GetDate() const
    {
        std::size_t iBegin = m_iBegin.load(boost::memory_order_relaxed);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_consume);
        std::size_t iSize = GetSize(iBegin, iEnd); //��Ч���ݳߴ�
        if (0 == iSize)
        {
            return std::vector<boost::asio::const_buffer>();
        }

        return GetRange<std::vector<boost::asio::const_buffer>>(GetOffset(iBegin), iSize);
    }

    //��ȡ�������򻺴�
    std::vector<boost::asio::mutable_buffer> GetBuffer()
    {
        std::size_t iBegin = m_iBegin.load(boost::memory_order_consume);
        std::size_t iEnd = m_iEnd.load(boost::memory_order_relaxed);
        std::size_t iSize = c_iSize - GetSize(iBegin, iEnd); //��������ߴ�
        if (0 == iSize)
        {
            return std::vector<boost::asio::mutable_buffer>();
        }

        return GetRange<std::vector<boost::asio::mutable_buffer>>(GetOffset(iEnd), iSize);
    }

    //д�����ݺ�����������Ч����
    void Append(const std::size_t p_iSize)
    {
        Increase(m_iEnd, p_iSize);
    }

    //�������ݺ�������Ч���ݷ�Χ
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
    //m_iBegin��m_iEnd��ȡֵ��ΧΪ[0, 2 * p_iSize)��֮����δѡ������[0, p_iSize)��
    //����Ϊ�����޷����֡�����Ϊ�ա��͡�ȫ�����������������������¡�m_iBegin == m_iEnd��������
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