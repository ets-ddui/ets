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

#include <windows.h>
#include <atlbase.h>
#include "../Core.h"
#include "../IDL/ETS.h"

namespace vcl4c
{
    namespace itf
    {
        typedef _ATL_INTMAP_ENTRY CMapEntry;

        template <class CThreadModel>
        class CInterfaceBase
            : public NonCopyable
        {
        protected:
            explicit CInterfaceBase(IUnknown *p_pOuterUnknown = NULL)
                : m_bContained(true), m_pOuterUnknown(p_pOuterUnknown) //作为聚合对象，不需增加外部对象p_pOuterUnknown的引用计数，否则，会导致内存无法释放
            {
                if (NULL == m_pOuterUnknown)
                {
                    m_bContained = false;
                    m_iRefCount = 0;
                }
            }

            ULONG InternalAddRef()
            {
                if (m_bContained)
                {
                    return m_pOuterUnknown->AddRef();
                }
                else
                {
                    return CThreadModel::Increment(&m_iRefCount);
                }
            }

            ULONG InternalRelease()
            {
                if (m_bContained)
                {
                    return m_pOuterUnknown->Release();
                }
                else
                {
                    return CThreadModel::Decrement(&m_iRefCount);
                }
            }

            HRESULT InternalQueryInterface(void *p_pObject, const CMapEntry *p_pMapEntry,
                const IID &p_IID, void** p_ppResult)
            {
                if (m_bContained)
                {
                    return m_pOuterUnknown->QueryInterface(p_IID, p_ppResult);
                }
                else
                {
                    return InternalQueryInterfaceForDelegate(p_pObject, p_pMapEntry, p_IID, p_ppResult);
                }
            }

            HRESULT InternalQueryInterfaceForDelegate(void *p_pObject, const CMapEntry *p_pMapEntry,
                const IID &p_IID, void** p_ppResult)
            {
                //以下代码从atlbase.inl中的AtlInternalQueryInterface拷贝而来
                if (NULL == p_pObject || NULL == p_pMapEntry)
                {
                    return E_INVALIDARG;
                }

                if (NULL == p_ppResult)
                {
                    return E_POINTER;
                }

                *p_ppResult = NULL;
                if (InlineIsEqualUnknown(p_IID))
                {
                    IUnknown* pUnk = (IUnknown*)((INT_PTR)p_pObject + p_pMapEntry->dw);
                    pUnk->AddRef();
                    *p_ppResult = pUnk;

                    return S_OK;
                }

                while (NULL != p_pMapEntry->pFunc)
                {
                    BOOL bBlind = (NULL == p_pMapEntry->piid);

                    if (bBlind || InlineIsEqualGUID(*(p_pMapEntry->piid), p_IID))
                    {
                        if (_ATL_SIMPLEMAPENTRY == p_pMapEntry->pFunc)
                        {
                            IUnknown* pUnk = (IUnknown*)((INT_PTR)p_pObject + p_pMapEntry->dw);
                            pUnk->AddRef();
                            *p_ppResult = pUnk;

                            return S_OK;
                        }
                        else
                        {
                            HRESULT hRes = p_pMapEntry->pFunc(p_pObject, p_IID, p_ppResult, p_pMapEntry->dw);

                            if (hRes == S_OK || (!bBlind && FAILED(hRes)))
                            {
                                return hRes;
                            }
                        }
                    }

                    ++p_pMapEntry;
                }

                return E_NOINTERFACE;
            }

            template<class CClass>
            static HRESULT WINAPI Delegate(void* p_pObject, 
                const IID &p_IID, void** p_ppResult, DWORD_PTR p_iOffset)
            {
                CClass* objDelegate = *(CClass**)((DWORD_PTR)p_pObject + p_iOffset);

                if (NULL != objDelegate)
                {
                    return objDelegate->QueryInterfaceForDelegate(p_IID, p_ppResult);
                }
                else
                {
                    return E_NOINTERFACE;
                }
            }

        private:
            bool m_bContained; //表示当前对象是否被聚合，为true则m_pOuterUnknown有意义，否则，m_iRefCount有意义
            union
            {
                long m_iRefCount;
                IUnknown *m_pOuterUnknown;
            };

        };

        template<class CClass>
        inline HRESULT ObjectToDispatch(IDispatch ** p_itfResult, CClass * p_objClass)
        {
            if (nullptr == p_objClass)
            {
                return E_OUTOFMEMORY;
            }

            HRESULT hRes = p_objClass->Init();
            if (FAILED(hRes))
            {
                delete p_objClass;
                return hRes;
            }

            CComPtr<IUnknown> itfClass = p_objClass->GetUnknown(); //这里引用计数加1，防止后面QueryInterface释放对象的可能
            return itfClass->QueryInterface(p_itfResult);
        }

        #define BEGIN_DEFINE_MAP(CClass) \
            public: \
                const static char *GetClassName() throw() \
                { \
                    static char sClassName[] = #CClass; \
                    return sClassName; \
                } \
            private: \
                typedef CClass CRootClass; \
                const static vcl4c::itf::CMapEntry *GetMapEntry() throw() \
                { \
                    static const vcl4c::itf::CMapEntry c_pMapEntry[] = \
                    {

        #define SIMPLE_INTERFACE(IInterface) \
                        {&__uuidof(IInterface), offsetofclass(IInterface, CRootClass), _ATL_SIMPLEMAPENTRY},

        #define AGGREGATE_INTERFACE(IInterface, CClass, m_pClass) \
                        {&__uuidof(IInterface), (DWORD_PTR)offsetof(CRootClass, m_pClass), Delegate<CClass>},

        #define END_DEFINE_MAP \
                        {NULL, 0, 0} \
                    }; \
                    return c_pMapEntry; \
                } \
            public: \
                IUnknown *GetUnknown() \
                { \
                    return (IUnknown *)((INT_PTR)this + GetMapEntry()->dw); \
                }

#ifdef ETSDEBUG
        #define IMPLEMENT_IUNKNOWN \
            public: \
                STDMETHOD_(ULONG, AddRef)() \
                { \
                    ULONG l = InternalAddRef(); \
                    vcl4c::logger::WriteView(vcl4c::string::Format("%d = %s.AddRef()", l, GetClassName())); \
                    return l; \
                } \
                STDMETHOD_(ULONG, Release)() \
                { \
                    ULONG l = InternalRelease(); \
                    vcl4c::logger::WriteView(vcl4c::string::Format("%d = %s.Release()", l, GetClassName())); \
                    if (l == 0) \
                    delete this; \
                    return l; \
                } \
                STDMETHOD(QueryInterface)(const IID &p_IID, void** p_ppResult) throw() \
                { \
                    return InternalQueryInterface(this, GetMapEntry(), p_IID, p_ppResult); \
                } \
                STDMETHOD(QueryInterfaceForDelegate)(const IID &p_IID, void** p_ppResult) throw() \
                { \
                    return InternalQueryInterfaceForDelegate(this, GetMapEntry(), p_IID, p_ppResult); \
                } \
                template <class COtherClass> \
                HRESULT STDMETHODCALLTYPE QueryInterface(COtherClass** p_ppResult) throw() \
                { \
                    return QueryInterface(__uuidof(COtherClass), (void**)p_ppResult); \
                }
#else
        #define IMPLEMENT_IUNKNOWN \
            public: \
                STDMETHOD_(ULONG, AddRef)() \
                { \
                    return InternalAddRef(); \
                } \
                STDMETHOD_(ULONG, Release)() \
                { \
                    ULONG l = InternalRelease(); \
                    if (l == 0) \
                        delete this; \
                    return l; \
                } \
                STDMETHOD(QueryInterface)(const IID &p_IID, void** p_ppResult) throw() \
                { \
                    return InternalQueryInterface(this, GetMapEntry(), p_IID, p_ppResult); \
                } \
                STDMETHOD(QueryInterfaceForDelegate)(const IID &p_IID, void** p_ppResult) throw() \
                { \
                    return InternalQueryInterfaceForDelegate(this, GetMapEntry(), p_IID, p_ppResult); \
                } \
                template <class COtherClass> \
                HRESULT STDMETHODCALLTYPE QueryInterface(COtherClass** p_ppResult) throw() \
                { \
                    return QueryInterface(__uuidof(COtherClass), (void**)p_ppResult); \
                }
#endif

    }
}