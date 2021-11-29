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

#include "Core.h"
#include <list>
#include <stdexcept>
#include "array_auto_ptr.h"

class CMemQueue
    : public NonCopyable
{
private:
    typedef long CPU_TYPE;
    enum {NODE_COUNT = 256, NODE_SIZE = 4096};

    class CNode
        : public NonCopyable
    {
    private:
        enum {ALIGN_SIZE = sizeof(CPU_TYPE)};

    public:
        explicit CNode()
            : m_sBuffer(NULL), m_iPosRead(0), m_iPosWrite(0), m_iCapacity(0), m_ndNext(NULL)
        {

        }

        ~CNode()
        {
            if (NULL != m_sBuffer)
            {
                delete [] m_sBuffer;
                m_sBuffer = NULL;
            }
        }

        bool Init(const unsigned int p_iCapacity)
        {
            if (NULL != m_sBuffer || 0 == p_iCapacity)
            {
                return false;
            }

            m_iCapacity = p_iCapacity;
            m_sBuffer = new(std::nothrow) unsigned char[m_iCapacity];

            return NULL != m_sBuffer;
        }

        void Reset()
        {
            m_iPosRead = m_iPosWrite = 0;
        }

        CNode * GetNext() const
        {
            return m_ndNext;
        }

        unsigned long GetCapacity() const
        {
            return m_iCapacity;
        }

        void Append(CNode * p_ndNode)
        {
            p_ndNode->m_ndNext = m_ndNext;
            m_ndNext = p_ndNode;
        }

        //ͨ��GetMemory��ȡ�ڴ�ʱ���ڴ濪ʼ��4�ֽڱ�ʾ�ڴ泤�ȣ���ʼ��Ϊ��������ʾ�������ڴ棬��û����
        //����Write������д�����ʱ���Ὣ�ڴ泤�ȵ�ֵ��Ϊ����
        //�����ڶ�ȡʱ��������ڴ泤�ȵ����������߳�ͬ��
        HANDLE GetMemory(const unsigned long p_iSize, CNode * &p_ndNext)
        {
            unsigned char * sPosCurr = &m_sBuffer[m_iPosWrite];
            unsigned long iSize = 0;
            if (NODE_SIZE >= p_iSize)
            {
                iSize = sizeof(CPU_TYPE) + (p_iSize + ALIGN_SIZE - 1) / ALIGN_SIZE * ALIGN_SIZE; //������Ҫ�����ڴ���(������+ʵ������)
            }
            else
            {
                iSize = sizeof(CPU_TYPE) + sizeof(CPU_TYPE); //������ݳ��ȳ���һ���ڵ�������������ָ��洢(������+ָ����)
            }

            //1.0 ��ǰ�ڵ�ʣ���ڴ��㹻���
            unsigned long iLeft = m_iCapacity - m_iPosWrite;
            if (iSize <= iLeft)
            {
                m_iPosWrite += iSize;

                p_ndNext = m_iPosWrite < m_iCapacity ? this : m_ndNext;
                *reinterpret_cast<CPU_TYPE *>(sPosCurr) = -1 * p_iSize;
                return sPosCurr;
            }

            //2.0 ��ǰ�ڵ�ʣ���ڴ治���ţ���ֶ���ڵ�
            iSize -= iLeft;
            CNode * nd = m_ndNext;
            while (NULL != nd)
            {
                if (iSize <= nd->m_iCapacity)
                {
                    m_iPosWrite = m_iCapacity;
                    nd->m_iPosWrite = iSize;

                    p_ndNext = nd->m_iPosWrite < nd->m_iCapacity ? nd : nd->m_ndNext;
                    *reinterpret_cast<CPU_TYPE *>(sPosCurr) = -1 * p_iSize;
                    return sPosCurr;
                }

                iSize -= nd->m_iCapacity;
                //nd->m_iPosWrite = nd->m_iCapacity; //�Ƶ�Write������
                nd = nd->m_ndNext;
            }

            return NULL;
        }

        void Write(HANDLE p_iHandle, const unsigned char * p_sData, const unsigned long p_iSize)
        {
            //1.0 ������Ч�Լ��
            unsigned char * sBuffer = reinterpret_cast<unsigned char *>(p_iHandle);
            if (sBuffer < m_sBuffer || (sBuffer + sizeof(CPU_TYPE)) > &m_sBuffer[m_iCapacity])
            {
                throw std::out_of_range("�����Ч");
            }

            CPU_TYPE * piSize = reinterpret_cast<CPU_TYPE *>(sBuffer);
            if (p_iSize != -1 * (*piSize))
            {
                throw std::invalid_argument("p_iSize��β��Ϸ�");
            }

            if (0 == p_iSize)
            {
                return;
            }

            //2.0 ��������
            sBuffer += sizeof(CPU_TYPE);
            unsigned long iLeft = &m_sBuffer[m_iCapacity] - sBuffer;

            if (NODE_SIZE >= p_iSize)
            {
                //2.1 ��ǰ�ڵ�ʣ���ڴ��㹻��ţ�ֱ�ӿ������ݲ�����
                if (p_iSize <= iLeft)
                {
                    memcpy(sBuffer, p_sData, p_iSize);
                    *piSize = p_iSize;
                    return;
                }

                //2.2 ��ǰ�ڵ�ʣ���ڴ治�����������ݣ������ݷֶδ�ŵ�����ڵ���
                memcpy(sBuffer, p_sData, iLeft);
                const unsigned char * psData = &p_sData[iLeft];
                unsigned long iSize = p_iSize - iLeft;
                CNode * nd = m_ndNext;
                while (0 < iSize)
                {
                    if (iSize <= nd->m_iCapacity)
                    {
                        memcpy(nd->m_sBuffer, psData, iSize);
                        *piSize = p_iSize;
                        return;
                    }

                    //GetMemory�ڴ治��ᵼ��ִ��ʧ�ܣ�����������Nodeʱ��δ��ȷ����m_iPosWrite������ʼ���߼��Ƶ�����ִ��
                    nd->m_iPosWrite = nd->m_iCapacity;

                    memcpy(nd->m_sBuffer, psData, nd->m_iCapacity);
                    psData += nd->m_iCapacity;
                    iSize -= nd->m_iCapacity;
                    nd = nd->m_ndNext;
                }
            }
            else
            {
                //2.1 ������ݳ��ȳ���һ���ڵ�������������ָ��洢
                if (sizeof(CPU_TYPE) <= iLeft)
                {
                    *reinterpret_cast<CPU_TYPE *>(sBuffer) = reinterpret_cast<CPU_TYPE>(p_sData); //����洢����ָ��
                    *piSize = p_iSize;
                }
                else
                {
                    *reinterpret_cast<CPU_TYPE *>(m_ndNext->m_sBuffer) = reinterpret_cast<CPU_TYPE>(p_sData); //����洢����ָ��
                    *piSize = p_iSize;
                }
            }
        }

        HANDLE BeginRead(CNode * &p_ndNext, long & p_iSize)
        {
            if (m_iPosRead >= m_iPosWrite)
            {
                return NULL;
            }

            p_iSize = *reinterpret_cast<CPU_TYPE *>(&m_sBuffer[m_iPosRead]);
            if (0 > p_iSize)
            {
                return NULL;
            }

            //1.0 ����û�в�ֽڵ㣬һ���Զ�ȡ
            unsigned char * sPosCurr = &m_sBuffer[m_iPosRead];
            unsigned long iSize = 0;
            if (0 == p_iSize)
            {
                m_iPosRead += sizeof(CPU_TYPE);

                p_ndNext = m_iPosRead < m_iCapacity ? this : m_ndNext;
                return sPosCurr;
            }
            else if (NODE_SIZE >= p_iSize)
            {
                iSize = sizeof(CPU_TYPE) + (p_iSize + ALIGN_SIZE - 1) / ALIGN_SIZE * ALIGN_SIZE;
            }
            else
            {
                iSize = sizeof(CPU_TYPE) + sizeof(CPU_TYPE);
            }
            unsigned long iLeft = m_iCapacity - m_iPosRead;
            if (iSize <= iLeft)
            {
                ::InterlockedIncrement(&m_iReadCount);
                m_iPosRead += iSize;

                p_ndNext = m_iPosRead < m_iCapacity ? this : m_ndNext;
                return sPosCurr;
            }

            //2.0 ���ݲ��Ϊ����ڵ㣬���ڵ��ִ���
            ::InterlockedIncrement(&m_iReadCount);
            m_iPosRead = m_iCapacity;
            iSize -= iLeft;
            CNode * nd = m_ndNext;
            while (0 < iSize)
            {
                if (iSize <= nd->m_iCapacity)
                {
                    ::InterlockedIncrement(&nd->m_iReadCount);
                    nd->m_iPosRead = iSize;

                    p_ndNext = nd->m_iPosRead < nd->m_iCapacity ? nd : nd->m_ndNext;
                    return sPosCurr;
                }

                ::InterlockedIncrement(&nd->m_iReadCount);
                nd->m_iPosRead = nd->m_iCapacity;

                iSize -= nd->m_iCapacity;
                nd = nd->m_ndNext;
            }

            return NULL;
        }

        void Read(HANDLE p_iHandle, unsigned char * p_sData, const unsigned long p_iSize)
        {
            //1.0 ������Ч�Լ��
            unsigned char * sBuffer = reinterpret_cast<unsigned char *>(p_iHandle);
            if (sBuffer < m_sBuffer || (sBuffer + sizeof(CPU_TYPE)) > &m_sBuffer[m_iCapacity])
            {
                throw std::out_of_range("�����Ч");
            }

            unsigned long iSize = *reinterpret_cast<CPU_TYPE *>(sBuffer);
            if (static_cast<long>(iSize) < 0)
            {
                throw std::invalid_argument("����δд��");
            }

            if (NODE_SIZE >= iSize)
            {
                if (iSize > p_iSize)
                {
                    throw std::invalid_argument("����ڴ治��");
                }
            }
            else
            {
                if (sizeof(CPU_TYPE) > p_iSize)
                {
                    throw std::invalid_argument("����ڴ治��");
                }
            }

            if (0 == iSize)
            {
                return;
            }

            //2.0 ��ȡ����
            sBuffer += sizeof(CPU_TYPE);
            unsigned long iLeft = &m_sBuffer[m_iCapacity] - sBuffer;

            if (NODE_SIZE >= iSize)
            {
                //2.1 ����δ��ֶ�ڵ㣬ֱ�Ӷ�ȡ
                if (iSize <= iLeft)
                {
                    memcpy(p_sData, sBuffer, iSize);
                    ::InterlockedDecrement(&m_iReadCount);
                    return;
                }

                //2.2 ���ݲ���˶���ڵ㣬�ֶζ�ȡ
                memcpy(p_sData, sBuffer, iLeft);
                p_sData += iLeft;
                iSize -= iLeft;
                CNode * nd = m_ndNext;
                ::InterlockedDecrement(&m_iReadCount);
                while (iSize > 0)
                {
                    if (iSize <= nd->m_iCapacity)
                    {
                        memcpy(p_sData, nd->m_sBuffer, iSize);
                        ::InterlockedDecrement(&nd->m_iReadCount);
                        return;
                    }

                    memcpy(p_sData, nd->m_sBuffer, nd->m_iCapacity);
                    p_sData += nd->m_iCapacity;
                    iSize -= nd->m_iCapacity;
                    nd = nd->m_ndNext;
                    ::InterlockedDecrement(&nd->m_iReadCount);
                }
            }
            else
            {
                CPU_TYPE * pData = reinterpret_cast<CPU_TYPE *>(p_sData);

                if (sizeof(CPU_TYPE) <= iLeft)
                {
                    *pData = *reinterpret_cast<CPU_TYPE *>(sBuffer);
                    ::InterlockedDecrement(&m_iReadCount);
                }
                else
                {
                    *pData = *reinterpret_cast<CPU_TYPE *>(m_ndNext->m_sBuffer);
                    ::InterlockedDecrement(&m_ndNext->m_iReadCount);
                    ::InterlockedDecrement(&m_iReadCount);
                }
            }
        }

        bool CanRelease() const
        {
            return (m_iPosRead >= m_iCapacity) && (m_iPosWrite >= m_iCapacity) && (0 == m_iReadCount);
        }

    private:
        unsigned char * m_sBuffer;
        unsigned long m_iPosRead, m_iPosWrite, m_iCapacity;
        CNode * m_ndNext;
        //���ݶ�ȡ��Ϻ󣬻��Զ������ڴ棬��׷�ӵ��������λ��
        //�������ݶ�ȡ���������첽��ʽ�����(�������������Ӷ�ȡƫ�Ƶ�ַ��Ȼ���ͷ������ٿ�������)
        //����BeginReadִ������޷����̻����ڴ棬��ˣ���m_iReadCount��ʶ�ж����߳�����(��Ҫ)ִ��Read����
        long volatile m_iReadCount;

    };

private:
    CNode * m_ndHead, * m_ndTail, *m_ndRead, * m_ndWrite;
    unsigned long m_iNodeSize; //�ڴ��С(����NODE_SIZE��С���ڴ棬�������ⲿ���ڴ�)
    unsigned long m_iAdditionalSize; //�����ڴ��С(����NODE_SIZE��С���ڴ棬��ռ���ⲿ���ڴ棬���׵�ַָ����¼��m_iNodeSize��)
    unsigned long m_iCapacity; //�ڴ��С���ޣ�����Ϊc_iNodeSize��������
    CSpinLock m_csLock;

    bool AddNode()
    {
        CSingleLock slLock(&m_csLock, TRUE);

        if (m_iNodeSize + NODE_SIZE > m_iCapacity)
        {
            return false;
        }

        CNode * ndNew = new(std::nothrow) CNode();
        if (NULL == ndNew)
        {
            return false;
        }
        if (!ndNew->Init(NODE_SIZE))
        {
            delete ndNew;
            return false;
        }

        m_ndTail->Append(ndNew);
        m_iNodeSize += NODE_SIZE;
        m_ndTail = ndNew;

        if (NULL == m_ndWrite)
        {
            m_ndWrite = ndNew;
        }

        if (NULL == m_ndRead)
        {
            m_ndRead = ndNew;
        }

        return true;
    }

    void ReleaseNode()
    {
        CSingleLock slLock(&m_csLock, TRUE);

        while (m_ndHead->CanRelease())
        {
            if (m_iNodeSize > m_iCapacity)
            {
                m_iNodeSize -= m_ndHead->GetCapacity();

                CNode * ndDelete = m_ndHead;
                m_ndHead = m_ndHead->GetNext();
                delete ndDelete;
            }
            else
            {
                CNode * ndNextHead = m_ndHead->GetNext();

                m_ndTail->Append(m_ndHead);
                m_ndHead->Reset();

                m_ndTail = m_ndHead;
                m_ndHead = ndNextHead;
            }
        }
    }

public:
    CMemQueue(const unsigned long p_iNodeMaxCount = NODE_COUNT) throw(...)
        : m_ndHead(NULL), m_ndTail(NULL), m_ndRead(NULL), m_ndWrite(NULL),
        m_iNodeSize(NODE_SIZE), m_iAdditionalSize(0), m_iCapacity(p_iNodeMaxCount * NODE_SIZE)
    {
        //m_iCapacity����Ҫ��2���ڵ�Ĵ洢�ռ�
        if (NODE_SIZE >= m_iCapacity)
        {
            m_iCapacity = 2 * NODE_SIZE;
        }

        m_ndHead = new(std::nothrow) CNode();
        m_ndTail = m_ndRead = m_ndWrite = m_ndHead;
        if (NULL == m_ndHead || !m_ndHead->Init(NODE_SIZE))
        {
            throw std::runtime_error("�ڴ治��");
        }
    }

    virtual ~CMemQueue()
    {
        std::string sData;
        while (0 < m_iAdditionalSize)
        {
            Read(sData);
        }

        while (NULL != m_ndHead)
        {
            CNode * ndNext = m_ndHead->GetNext();
            delete m_ndHead;
            m_ndHead = ndNext;
        }
    }

    void Resize(const unsigned long p_iNodeMaxCount)
    {
        if (2 > p_iNodeMaxCount)
        {
            m_iCapacity = 2 * NODE_SIZE;
        }
        else
        {
            m_iCapacity = p_iNodeMaxCount * NODE_SIZE;
        }
    }

    bool Write(const unsigned char * p_sData, const unsigned int p_iSize)
    {
        if (p_iSize > m_iCapacity)
        {
            return false;
        }

        vcl4c::other::array_auto_ptr<unsigned char> aapData;
        const unsigned char * sData = NULL;
        if (p_iSize <= NODE_SIZE)
        {
            sData = p_sData;
        }
        else
        {
            aapData.reset(new(std::nothrow) unsigned char[p_iSize]);
            if (!aapData)
            {
                return false;
            }
            memcpy(aapData.get(), p_sData, p_iSize);

            sData = aapData.get();
        }

        CSingleLock slLock(&m_csLock);
        CNode * nd = NULL;
        HANDLE hNode = NULL;

        while (TRUE)
        {
            slLock.Lock();

            if (NULL == m_ndWrite)
            {
                if (!AddNode())
                {
                    return false;
                }
            }

            nd = m_ndWrite;
            hNode = m_ndWrite->GetMemory(p_iSize, m_ndWrite);
            if (NULL != hNode)
            {
                slLock.Unlock();
                break;
            }

            if (!AddNode())
            {
                return false;
            }

            slLock.Unlock();
        }

        aapData.release();
        nd->Write(hNode, sData, p_iSize);
        if (sData != p_sData)
        {
            ::InterlockedExchangeAdd (&m_iAdditionalSize, p_iSize);
        }

        return true;
    }

    int Read(std::string & p_sData)
    {
        CSingleLock slLock(&m_csLock);
        CNode * nd = NULL;
        HANDLE hNode = NULL;

        slLock.Lock();

        if (NULL == m_ndRead)
        {
            return false;
        }

        nd = m_ndRead;
        long iSize = 0;
        hNode = m_ndRead->BeginRead(m_ndRead, iSize);
        slLock.Unlock();
        if (NULL == hNode)
        {
            return false;
        }

        if (0 == iSize)
        {
            p_sData.clear();
        }
        else if (NODE_SIZE >= iSize)
        {
            p_sData.resize(iSize);
            nd->Read(hNode, (unsigned char *)(p_sData.c_str()), iSize);
        }
        else
        {
            CPU_TYPE pData = 0;
            nd->Read(hNode, (unsigned char *)&pData, sizeof(CPU_TYPE));
            unsigned char * sData = reinterpret_cast<unsigned char *>(pData);

            p_sData.resize(iSize);
            memcpy(const_cast<char *>(p_sData.c_str()), sData, iSize);

            delete [] sData;
            ::InterlockedExchangeAdd (&m_iAdditionalSize, -1 * iSize);
        }

        ReleaseNode();

        return true;
    }

};
