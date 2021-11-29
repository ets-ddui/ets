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

#include <string>
#include <map>
#include <vector>
#include "InterfaceImplement.h"
#include "../Debug.h"

namespace vcl4c
{
    namespace itf
    {
        inline HRESULT W2String(std::string &p_sResult, LPCOLESTR p_sValue, const int p_iSize = _ATL_SAFE_ALLOCA_DEF_THRESHOLD)
        {
            if (NULL == p_sValue)
            {
                p_sResult = "";
                return S_OK;
            }

            USES_CONVERSION_EX;
            LPCSTR sResult = W2A_EX(p_sValue, p_iSize);
            if (NULL == sResult)
            {
                return E_OUTOFMEMORY;
            }

            p_sResult = sResult;
            return S_OK;
        }

        inline HRESULT A2WString(std::wstring &p_sResult, LPCSTR p_sValue, const int p_iSize = _ATL_SAFE_ALLOCA_DEF_THRESHOLD)
        {
            if (NULL == p_sValue)
            {
                p_sResult = L"";
                return S_OK;
            }

            USES_CONVERSION_EX;
            LPCOLESTR sResult = A2W_EX(p_sValue, p_iSize);
            if (NULL == sResult)
            {
                return E_OUTOFMEMORY;
            }

            p_sResult = sResult;
            return S_OK;
        }

        inline HRESULT A2BSTR(CComBSTR &p_sResult, LPCSTR p_sValue, const int p_iSize = _ATL_SAFE_ALLOCA_DEF_THRESHOLD)
        {
            if (NULL == p_sValue)
            {
                p_sResult = "";
                return S_OK;
            }

            USES_CONVERSION_EX;
            LPCOLESTR sResult = A2W_EX(p_sValue, p_iSize);
            if (NULL == sResult)
            {
                return E_OUTOFMEMORY;
            }

            p_sResult = sResult;
            return S_OK;
        }

        class CLocalInvoke
        {
        public:
            HRESULT Invoke(void *p_Object, ITypeInfo* p_TypeInfo,
                const DISPID p_DispID, const WORD p_iFlags,
                DISPPARAMS* p_Params,
                VARIANT* p_vResult, EXCEPINFO* p_eiExceptionInfo, UINT* p_iArgumentError)
            {
                HRESULT hRes = S_OK;

                //1.0 获取函数的调用协议、返回类型、入参类型的信息
                if (m_mapParamType.empty())
                {
                    hRes = InitParamType(p_TypeInfo);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }
                }

                iterator it = m_mapParamType.find(CDispID(p_DispID,
                    0 != (p_iFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))));
                if (m_mapParamType.end() == it)
                {
                    return DISP_E_MEMBERNOTFOUND;
                }

                //1.1 基本信息计算
                const CFuncProperty &fp = it->second.first;
                std::vector<VARTYPE> &vecParamType = it->second.second;
                bool bHasOutParam = !vecParamType.empty() && 0 != (*vecParamType.rbegin() & VT_BYREF); //是否有[out, retval]类型的入参

                //在没有传入参数的情况下，Delphi会自动添加一个参数，类型为VT_ERROR，值为DISP_E_PARAMNOTFOUND($80020004)，
                //导致下面在检查参数个数的时候报错，因此，进行修正处理
                if (1 == p_Params->cArgs && VT_ERROR == p_Params->rgvarg[0].vt && DISP_E_PARAMNOTFOUND == p_Params->rgvarg[0].lVal)
                {
                    p_Params->cArgs = 0;
                }

                //如果最后一个参数的类型为[out, retval]，在p_Params->rgvarg中不会包含
                if (bHasOutParam)
                {
                    if (p_Params->cArgs != vecParamType.size() - 1)
                    {
                        return DISP_E_BADPARAMCOUNT;
                    }
                }
                else
                {
                    if (p_Params->cArgs != vecParamType.size())
                    {
                        return DISP_E_BADPARAMCOUNT;
                    }
                }

                //2.0 构造函数入参
                VARTYPE *ppParamType = NULL;
                VARIANTARG **ppParam = NULL;
                std::vector<VARIANTARG *> vecParam; //DispCallFunc的入参prgpvarg是VARIANTARG的指针数组，不能传p_Params->rgvarg的地址
                CComVariant vOutParam; //为[out, retval]类型的参数分配内存
                __int64 pOutParam = 0; //为[out, retval]类型参数指向的地址分配内存(返回结果会写入此地址，结果大小可能小于sizeof(__int64))
                if (!vecParamType.empty())
                {
                    //2.1 p_Params->rgvarg中的参数是按倒序传入，需进行反转
                    for (int i = p_Params->cArgs - 1, iType = 0; i >= 0; --i, ++iType)
                    {
                        if (p_Params->rgvarg[i].vt != vecParamType[iType])
                        {
                            //TODO: 如果p_Params->rgvarg[i]为BSTR类型，vecParamType[iType]也为VT_BSTR，调用VariantChangeType会抛异常，
                            //原因是程序会先拷贝一份字符串，然后释放原字符串，在释放时出现问题，原因不明
                            hRes = ::VariantChangeType(p_Params->rgvarg + i, p_Params->rgvarg + i, 0, vecParamType[iType]);

                            if (FAILED(hRes))
                            {
                                return hRes;
                            }
                        }

                        vecParam.push_back(p_Params->rgvarg + i);
                    }

                    //2.2 p_Params->rgvarg中不包含通过[out, retval]定义的返回参数，这里为其分配内存，供DispCallFunc使用
                    if (bHasOutParam)
                    {
                        //vOutParam.ChangeType(VT_BSTR);
                        //vOutParam.SetByRef((BSTR *)&pOutParam);

                        //由于CComVariant::SetByRef要绑定类型信息，实现很麻烦，因此，将其实现直接拷贝出来，不做类型检查
                        vOutParam.Clear();
                        vOutParam.vt = *vecParamType.rbegin();
                        vOutParam.byref = &pOutParam;

                        vecParam.push_back(&vOutParam);
                    }

                    ppParamType = &vecParamType[0];
                    ppParam = &vecParam[0];
                }

                //2.3 DispCallFunc只允许返回值的类型在范围[VT_EMPTY<0>, VT_UINT<23>]内，因此，将返回值替换为long类型
                VARTYPE vtReturn = fp.m_vtReturn;
                if (VT_HRESULT == vtReturn)
                {
                    vtReturn = VT_I4;
                }
                else if (VT_VOID == vtReturn)
                {
                    vtReturn = VT_EMPTY;
                }

                //3.0 调用接口函数
                //DispCallFunc内部将ppParam中的实际值压入堆栈(未进行任何类型转换)，并调用函数地址，
                //函数返回值(对浮点数、64位数值有特殊处理)直接写入p_vResult，vtReturn也会直接写入p_vResult
                hRes = ::DispCallFunc(p_Object, fp.m_iOVft, fp.m_ccCallConv, vtReturn,
                    vecParamType.size(), ppParamType, ppParam,
                    p_vResult);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                //4.0 处理返回结果
                if (VT_HRESULT == fp.m_vtReturn)
                {
                    hRes = p_vResult->lVal;
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }
                }

                if (bHasOutParam)
                {
                    hRes = ::VariantCopyInd(p_vResult, &vOutParam);

                    //由于vOutParam保存的是引用类型，当期析构时，并不会释放引用的对象，如果不手工清理，会导致内存泄露
                    //实际对象的地址存放在pOutParam中，参考前面2.2节的代码
                    if ((VT_BSTR | VT_BYREF) == vOutParam.vt)
                    {
                        ::SysFreeString(*(BSTR *)vOutParam.byref);
                    }
                    else if ((VT_DISPATCH | VT_BYREF) == vOutParam.vt)
                    {
                        (*(IDispatch **)vOutParam.byref)->Release();
                    }
                    else if ((VT_UNKNOWN | VT_BYREF) == vOutParam.vt)
                    {
                        (*(IUnknown **)vOutParam.byref)->Release();
                    }
                    else
                    {
                        /* TODO: 这里对类型的处理还不完善，有可能导致内存泄露，后续需继续完善 */
                    }
                }
                else if (VT_HRESULT == fp.m_vtReturn)
                {
                    hRes = ::VariantClear(p_vResult);
                }

                return hRes;
            }

        private:
            class CDispID
            {
            public:
                CDispID(DISPID p_iDispID, bool p_bPropertySet)
                    : m_iDispID(p_iDispID), m_bPropertySet(p_bPropertySet)
                {
                }

                bool operator < (const CDispID &p_DispID) const
                {
                    if (m_iDispID < p_DispID.m_iDispID)
                        return true;
                    else if (m_iDispID > p_DispID.m_iDispID)
                        return false;
                    else if (m_bPropertySet < p_DispID.m_bPropertySet)
                        return true;
                    else
                        return false;
                }
            private:
                DISPID m_iDispID;
                bool m_bPropertySet;
            };
            struct CFuncProperty
            {
                SHORT m_iOVft; //偏移地址
                FUNCKIND m_fkFuncKind; //函数类型
                CALLCONV m_ccCallConv; //调用协议
                VARTYPE m_vtReturn; //返回值
            };
            std::map<CDispID, std::pair<CFuncProperty, std::vector<VARTYPE>>> m_mapParamType;
            typedef std::map<CDispID, std::pair<CFuncProperty, std::vector<VARTYPE>>>::iterator iterator;

            HRESULT InitParamType(ITypeInfo* p_TypeInfo)
            {
                TYPEATTR* ta;
                HRESULT hRes = p_TypeInfo->GetTypeAttr(&ta);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                for (int i = 0; i < ta->cFuncs; ++i)
                {
                    std::vector<VARTYPE> vParamType;

                    FUNCDESC* fd;
                    hRes = p_TypeInfo->GetFuncDesc(i, &fd);
                    if (FAILED(hRes))
                    {
                        break;
                    }

                    if (FUNC_PUREVIRTUAL != fd->funckind || CC_STDCALL != fd->callconv)
                    {
                        continue;
                    }

                    //获取函数入参信息(目前未实现对数组类型VT_ARRAY的处理)
                    for (ELEMDESC *ed = fd->lprgelemdescParam, *edEnd = fd->lprgelemdescParam + fd->cParams; ed < edEnd; ++ed)
                    {
                        if (VT_PTR == ed->tdesc.vt)
                        {
                            vParamType.push_back(ed->tdesc.lptdesc->vt | VT_BYREF);
                        }
                        else if (VT_CARRAY == ed->tdesc.vt)
                        {
                            vParamType.push_back(ed->tdesc.lpadesc->tdescElem.vt | VT_ARRAY);
                        }
                        else
                        {
                            vParamType.push_back(ed->tdesc.vt);
                        }
                    }

                    //获取函数基本信息，包括地址偏移量、函数类型(是否是虚函数、静态函数等)、调用协议、返回值类型
                    CFuncProperty fp = {fd->oVft, fd->funckind, fd->callconv, fd->elemdescFunc.tdesc.vt};

                    //对于属性类型，同一个memid既对应Get函数，同时对应Set函数，函数入参都有差异，因此，需要区别对待(仅通过fd->memid不足以区分，需要再增加fd->invkind的判断)
                    m_mapParamType.insert(std::make_pair(CDispID(fd->memid, INVOKE_PROPERTYPUT == fd->invkind || INVOKE_PROPERTYPUTREF == fd->invkind),
                        std::make_pair(fp, vParamType)));

                    p_TypeInfo->ReleaseFuncDesc(fd);
                }

                p_TypeInfo->ReleaseTypeAttr(ta);

                return hRes;
            }

        };

        template <class CThreadModel, class CInvoke = CLocalInvoke>
        class CDispatch:
            public CInterfaceBase<CThreadModel>,
            public IDispatch
        {
            BEGIN_DEFINE_MAP(CDispatch)
                SIMPLE_INTERFACE(IDispatch)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

        public:
            explicit CDispatch(const std::string &p_sTypeLibFile, const GUID *p_GUID, IUnknown *p_Object)
                : CInterfaceBase(NULL),
                m_sTypeLibFile(p_sTypeLibFile), m_GUID(p_GUID), m_Object(p_Object),
                m_pInfo(NULL)
            {
            }

            explicit CDispatch(const long p_iTypeLibIndex, const GUID *p_GUID, IUnknown *p_Object)
                : CInterfaceBase(NULL),
                m_sTypeLibFile(), m_GUID(p_GUID), m_Object(p_Object),
                m_pInfo(NULL)
            {
                m_sTypeLibFile = vcl4c::debugger::GetCurrentModuleFileName();

                char sIndex[32];
                _ltoa(p_iTypeLibIndex, sIndex, 10);

                m_sTypeLibFile.append("\\");
                m_sTypeLibFile.append(sIndex);
            }

        public:
            //IDispatch实现
            STDMETHOD(GetTypeInfoCount)(_Out_ UINT* p_iTypeInfoCount)
            {
                if (NULL == p_iTypeInfoCount)
                {
                    return E_POINTER;
                }

                *p_iTypeInfoCount = 1;

                return S_OK;
            }

            STDMETHOD(GetTypeInfo)(_In_ UINT p_iTypeInfo, _In_ LCID p_iLocaleID, _Deref_out_ ITypeInfo** p_TypeInfo)
            {
                if (0 != p_iTypeInfo)
                {
                    return DISP_E_BADINDEX;
                }

                if (NULL == p_TypeInfo)
                {
                    return E_POINTER;
                }

                *p_TypeInfo = NULL;

                HRESULT hRes = InnerGetTypeInfo(p_iLocaleID);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                *p_TypeInfo = m_pInfo;
                m_pInfo->AddRef();

                return S_OK;
            }

            STDMETHOD(GetIDsOfNames)(_In_ REFIID /*p_IID*/,
                _In_count_(p_iNamesCount) _Deref_pre_z_ LPOLESTR* p_sNames, _In_ UINT p_iNamesCount,
                _In_ LCID p_iLocaleID, _Out_ DISPID* p_DispID)
            {
#ifdef ETSDEBUG
                using namespace vcl4c::logger;
                using namespace vcl4c::string;

                WriteView(Format("GetIDsOfNames(%ls, %d)", p_sNames[0], p_iNamesCount));
#endif
                HRESULT hRes = InnerGetTypeInfo(p_iLocaleID);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                std::wstring sName;
                if (1 == p_iNamesCount)
                {
                    sName = p_sNames[0];
                    std::map<std::wstring, DISPID>::const_iterator it = m_mapDispIDs.find(sName);
                    if (m_mapDispIDs.end() != it)
                    {
                        *p_DispID = it->second;
#ifdef ETSDEBUG
                        WriteView(Format("GetIDsOfNames_Return(%ls, %d)", p_sNames[0], *p_DispID));
#endif
                        return S_OK;
                    }
                }

                hRes = m_pInfo->GetIDsOfNames(p_sNames, p_iNamesCount, p_DispID);
#ifdef ETSDEBUG
                WriteView(Format("GetIDsOfNames_Return(%ls, %d)", p_sNames[0], *p_DispID));
#endif
                if (SUCCEEDED(hRes) && 1 == p_iNamesCount)
                {
                    sName = p_sNames[0];
                    m_mapDispIDs.insert(std::make_pair(sName, *p_DispID));
                }

                return hRes;
            }

            STDMETHOD(Invoke)(_In_ DISPID p_DispID, _In_ REFIID /*p_IID*/,
                _In_ LCID p_iLocaleID, _In_ WORD p_iFlags,
                _In_ DISPPARAMS* p_Params,
                _Out_opt_ VARIANT* p_vResult, _Out_opt_ EXCEPINFO* p_eiExceptionInfo, _Out_opt_ UINT* p_iArgumentError)
            {
#ifdef ETSDEBUG
                using namespace vcl4c::logger;
                using namespace vcl4c::string;

                WriteView(Format("Invoke(%d, 0x%hx, %d, %d)", p_DispID, p_iFlags, p_Params->cArgs, p_Params->cNamedArgs));
#endif
                //经测试，p_iFlags应该是个集合类型，例如，读取属性值时，p_iFlags等于3(DISPATCH_METHOD、DISPATCH_PROPERTYGET的组合)
                //javascript样例如下：
                //var s = quickfix.Value('11'); //var的声明是必须的，否则，会被当成函数调用来处理
                if (p_iFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF))
                {
                    if (1 != p_Params->cNamedArgs || DISPID_PROPERTYPUT != p_Params->rgdispidNamedArgs[0])
                    {
                        return DISP_E_NONAMEDARGS;
                    }
                }
                else
                {
                    if (0 != p_Params->cNamedArgs)
                    {
                        return DISP_E_NONAMEDARGS;
                    }
                }

                HRESULT hRes = InnerGetTypeInfo(p_iLocaleID);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                VARIANT vResult;
                if (NULL == p_vResult)
                {
                    p_vResult = &vResult;
                }

                EXCEPINFO eiExceptionInfo;
                if (NULL == p_eiExceptionInfo)
                {
                    p_eiExceptionInfo = &eiExceptionInfo;
                }

                UINT iArgumentError = 0;
                if (NULL == p_iArgumentError)
                {
                    p_iArgumentError = &iArgumentError;
                }

                if (!m_Object)
                {
                    IUnknown *itf = NULL;
                    hRes = QueryInterface(*m_GUID, (void **)&itf);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    hRes = m_Invoke.Invoke(itf, m_pInfo,
                        p_DispID, p_iFlags,
                        p_Params,
                        p_vResult, p_eiExceptionInfo, p_iArgumentError);

                    itf->Release();
                }
                else
                {
                    hRes = m_Invoke.Invoke((IUnknown *)m_Object, m_pInfo,
                        p_DispID, p_iFlags,
                        p_Params,
                        p_vResult, p_eiExceptionInfo, p_iArgumentError);
                }

                return hRes;
            }

        private:
            std::string m_sTypeLibFile;
            const GUID *m_GUID;
            CComPtr<IUnknown> m_Object;
            ITypeInfo* m_pInfo;
            typename CThreadModel::AutoCriticalSection m_csLock;
            std::map<std::wstring, DISPID> m_mapDispIDs;
            CInvoke m_Invoke;

            HRESULT InnerGetTypeInfo(const LCID p_iLocaleID)
            {
                if (NULL != m_pInfo)
                {
                    return S_OK;
                }

                HRESULT hRes = E_FAIL;

                CComCritSecLock<CThreadModel::AutoCriticalSection> csLock(m_csLock, false);
                hRes = csLock.Lock();
                if (FAILED(hRes))
                {
                    return hRes;
                }

                if (NULL == m_pInfo)
                {
                    USES_CONVERSION_EX;
                    LPOLESTR sTypeLibFile = A2W_EX(m_sTypeLibFile.c_str(), _ATL_SAFE_ALLOCA_DEF_THRESHOLD);
                    if (NULL == sTypeLibFile)
                    {
                        return E_OUTOFMEMORY;
                    }

                    ITypeLib* itfTypeLib = NULL;
                    hRes = ::LoadTypeLibEx(sTypeLibFile, REGKIND_NONE, &itfTypeLib);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    CComPtr<ITypeInfo> itfTypeInfo;
                    hRes = itfTypeLib->GetTypeInfoOfGuid(*m_GUID, &itfTypeInfo);
                    if (SUCCEEDED(hRes))
                    {
                        m_pInfo = itfTypeInfo.Detach();
#ifdef ETSDEBUG
                        ListMember();
#endif
                    }

                    itfTypeLib->Release();

                    return hRes;
                }

                return S_OK;
            }

#ifdef ETSDEBUG
            void ListMember()
            {
                using namespace vcl4c::logger;
                using namespace vcl4c::string;

                WriteView(Format("类型库路径：%s", m_sTypeLibFile.c_str()));

                TYPEATTR* ta;
                HRESULT hRes = m_pInfo->GetTypeAttr(&ta);
                if (FAILED(hRes))
                {
                    return;
                }

                WriteView(Format("符号个数：%d", ta->cFuncs));

                for (int i = 0; i < ta->cFuncs; ++i)
                {
                    FUNCDESC* fd;
                    if (SUCCEEDED(m_pInfo->GetFuncDesc(i, &fd)))
                    {
                        CComBSTR sName;

                        if (SUCCEEDED(m_pInfo->GetDocumentation(fd->memid, &sName, NULL, NULL, NULL)))
                        {
                            USES_CONVERSION_EX;
                            char *s = W2A_EX(sName, sName.Length());

                            WriteView(Format("成员：%d %s", fd->memid, s));
                        }

                        for (int iInner = 0; iInner < fd->cParams; ++iInner)
                        {
                            ELEMDESC *ed = fd->lprgelemdescParam + iInner;

                            WriteView(Format("入参[%d]：%d(%s)", iInner, ed->tdesc.vt, GetVarTypeName(ed->tdesc.vt).c_str()));
                        }

                        WriteView(Format("返回值：%d(%s)", fd->elemdescFunc.tdesc.vt, GetVarTypeName(fd->elemdescFunc.tdesc.vt).c_str()));

                        m_pInfo->ReleaseFuncDesc(fd);
                    }
                }

                m_pInfo->ReleaseTypeAttr(ta);
            }

            const std::string GetVarTypeName(const VARTYPE p_VarType)
            {
                static std::map<VARTYPE, std::string> g_VarTypeName;

                if (g_VarTypeName.empty())
                {
                    g_VarTypeName.insert(std::make_pair(VT_EMPTY, "VT_EMPTY"));
                    g_VarTypeName.insert(std::make_pair(VT_NULL, "VT_NULL"));
                    g_VarTypeName.insert(std::make_pair(VT_I2, "VT_I2"));
                    g_VarTypeName.insert(std::make_pair(VT_I4, "VT_I4"));
                    g_VarTypeName.insert(std::make_pair(VT_R4, "VT_R4"));
                    g_VarTypeName.insert(std::make_pair(VT_R8, "VT_R8"));
                    g_VarTypeName.insert(std::make_pair(VT_CY, "VT_CY"));
                    g_VarTypeName.insert(std::make_pair(VT_DATE, "VT_DATE"));
                    g_VarTypeName.insert(std::make_pair(VT_BSTR, "VT_BSTR"));
                    g_VarTypeName.insert(std::make_pair(VT_DISPATCH, "VT_DISPATCH"));
                    g_VarTypeName.insert(std::make_pair(VT_ERROR, "VT_ERROR"));
                    g_VarTypeName.insert(std::make_pair(VT_BOOL, "VT_BOOL"));
                    g_VarTypeName.insert(std::make_pair(VT_VARIANT, "VT_VARIANT"));
                    g_VarTypeName.insert(std::make_pair(VT_UNKNOWN, "VT_UNKNOWN"));
                    g_VarTypeName.insert(std::make_pair(VT_DECIMAL, "VT_DECIMAL"));
                    g_VarTypeName.insert(std::make_pair(VT_I1, "VT_I1"));
                    g_VarTypeName.insert(std::make_pair(VT_UI1, "VT_UI1"));
                    g_VarTypeName.insert(std::make_pair(VT_UI2, "VT_UI2"));
                    g_VarTypeName.insert(std::make_pair(VT_UI4, "VT_UI4"));
                    g_VarTypeName.insert(std::make_pair(VT_I8, "VT_I8"));
                    g_VarTypeName.insert(std::make_pair(VT_UI8, "VT_UI8"));
                    g_VarTypeName.insert(std::make_pair(VT_INT, "VT_INT"));
                    g_VarTypeName.insert(std::make_pair(VT_UINT, "VT_UINT"));
                    g_VarTypeName.insert(std::make_pair(VT_VOID, "VT_VOID"));
                    g_VarTypeName.insert(std::make_pair(VT_HRESULT, "VT_HRESULT"));
                    g_VarTypeName.insert(std::make_pair(VT_PTR, "VT_PTR"));
                    g_VarTypeName.insert(std::make_pair(VT_SAFEARRAY, "VT_SAFEARRAY"));
                    g_VarTypeName.insert(std::make_pair(VT_CARRAY, "VT_CARRAY"));
                    g_VarTypeName.insert(std::make_pair(VT_USERDEFINED, "VT_USERDEFINED"));
                    g_VarTypeName.insert(std::make_pair(VT_LPSTR, "VT_LPSTR"));
                    g_VarTypeName.insert(std::make_pair(VT_LPWSTR, "VT_LPWSTR"));
                    g_VarTypeName.insert(std::make_pair(VT_RECORD, "VT_RECORD"));
                    g_VarTypeName.insert(std::make_pair(VT_INT_PTR, "VT_INT_PTR"));
                    g_VarTypeName.insert(std::make_pair(VT_UINT_PTR, "VT_UINT_PTR"));
                    g_VarTypeName.insert(std::make_pair(VT_FILETIME, "VT_FILETIME"));
                    g_VarTypeName.insert(std::make_pair(VT_BLOB, "VT_BLOB"));
                    g_VarTypeName.insert(std::make_pair(VT_STREAM, "VT_STREAM"));
                    g_VarTypeName.insert(std::make_pair(VT_STORAGE, "VT_STORAGE"));
                    g_VarTypeName.insert(std::make_pair(VT_STREAMED_OBJECT, "VT_STREAMED_OBJECT"));
                    g_VarTypeName.insert(std::make_pair(VT_STORED_OBJECT, "VT_STORED_OBJECT"));
                    g_VarTypeName.insert(std::make_pair(VT_BLOB_OBJECT, "VT_BLOB_OBJECT"));
                    g_VarTypeName.insert(std::make_pair(VT_CF, "VT_CF"));
                    g_VarTypeName.insert(std::make_pair(VT_CLSID, "VT_CLSID"));
                    g_VarTypeName.insert(std::make_pair(VT_VERSIONED_STREAM, "VT_VERSIONED_STREAM"));
                    g_VarTypeName.insert(std::make_pair(VT_BSTR_BLOB, "VT_BSTR_BLOB"));
                    g_VarTypeName.insert(std::make_pair(VT_VECTOR, "VT_VECTOR"));
                    g_VarTypeName.insert(std::make_pair(VT_ARRAY, "VT_ARRAY"));
                    g_VarTypeName.insert(std::make_pair(VT_BYREF, "VT_BYREF"));
                    g_VarTypeName.insert(std::make_pair(VT_RESERVED, "VT_RESERVED"));
                    g_VarTypeName.insert(std::make_pair(VT_ILLEGAL, "VT_ILLEGAL"));
                    g_VarTypeName.insert(std::make_pair(VT_ILLEGALMASKED, "VT_ILLEGALMASKED"));
                    g_VarTypeName.insert(std::make_pair(VT_TYPEMASK, "VT_TYPEMASK"));
                }

                std::map<VARTYPE, std::string>::const_iterator it = g_VarTypeName.find(p_VarType);
                if (g_VarTypeName.end() != it)
                {
                    return it->second;
                }
                else
                {
                    return "";
                }
            }
#endif

        };

        class CDispatchHelper
        {
        public:
            template<class CClass, class IClassInterface>
            static HRESULT CreateDispatch(IDispatch **p_itfResult)
            {
                CComPtr<IDispatch> module;
                module = new CDispatch<ATL::CComMultiThreadModel>(CClass::IDL,
                    &__uuidof(IClassInterface),
                    static_cast<IClassInterface *>(new CClass()));

                *p_itfResult = module.Detach();

                return S_OK;
            }

            template<class CClass, class IClassInterface>
            static HRESULT CreateDispatch(IDispatch **p_itfResult, CClass *p_pObject)
            {
                CComPtr<IDispatch> module;
                module = new CDispatch<ATL::CComMultiThreadModel>(CClass::IDL,
                    &__uuidof(IClassInterface),
                    static_cast<IClassInterface *>(p_pObject));

                *p_itfResult = module.Detach();

                return S_OK;
            }

            static HRESULT GetIDsOfNames(DISPID &p_didResult, IDispatch *p_itfObject, const char *p_sFunctionName)
            {
                std::wstring strName;
                HRESULT hRes = vcl4c::itf::A2WString(strName, p_sFunctionName);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                LPOLESTR pstrName = const_cast<LPOLESTR>(strName.c_str());
                return p_itfObject->GetIDsOfNames(IID_NULL, &pstrName, 1, LOCALE_USER_DEFAULT, &p_didResult);
            }

            static HRESULT DispatchInvoke(IDispatch *p_itfObject, const char *p_sFunctionName, WORD p_iFlags,
                VARIANT* p_vResult, const char *p_sFormat, ...)
            {
                DISPID didDispid = 0;
                HRESULT hRes = GetIDsOfNames(didDispid, p_itfObject, p_sFunctionName);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                va_list valParams = NULL;
                va_start(valParams, p_sFormat);

                return DispatchInvoke(p_itfObject, didDispid, p_iFlags, p_vResult, p_sFormat, valParams);
            }

            static HRESULT DispatchInvoke(IDispatch *p_itfObject, const DISPID p_didDispid, WORD p_iFlags,
                VARIANT* p_vResult, const char *p_sFormat, ...)
            {
                va_list valParams = NULL;
                va_start(valParams, p_sFormat);

                return DispatchInvoke(p_itfObject, p_didDispid, p_iFlags, p_vResult, p_sFormat, valParams);
            }

            static HRESULT DispatchInvoke(VARIANT *p_vObject, const char *p_sFunctionName, WORD p_iFlags,
                VARIANT* p_vResult, const char *p_sFormat, ...)
            {
                if (VT_DISPATCH != p_vObject->vt)
                {
                    return E_INVALIDARG;
                }
                IDispatch *itfObject = p_vObject->pdispVal;

                DISPID didDispid = 0;
                HRESULT hRes = GetIDsOfNames(didDispid, itfObject, p_sFunctionName);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                va_list valParams = NULL;
                va_start(valParams, p_sFormat);

                return DispatchInvoke(itfObject, didDispid, p_iFlags, p_vResult, p_sFormat, valParams);
            }

            static HRESULT DispatchInvoke(VARIANT *p_vObject, const DISPID p_didDispid, WORD p_iFlags,
                VARIANT* p_vResult, const char *p_sFormat, ...)
            {
                if (VT_DISPATCH != p_vObject->vt)
                {
                    return E_INVALIDARG;
                }
                IDispatch *itfObject = p_vObject->pdispVal;

                va_list valParams = NULL;
                va_start(valParams, p_sFormat);

                return DispatchInvoke(itfObject, p_didDispid, p_iFlags, p_vResult, p_sFormat, valParams);
            }

        private:
            static HRESULT DispatchInvoke(IDispatch *p_itfObject, const DISPID p_didDispid, WORD p_iFlags,
                VARIANT* p_vResult, const char *p_sFormat, va_list &p_valParams)
            {
                HRESULT hRes = S_OK;

                DISPPARAMS dpParams = {nullptr, nullptr, 0, 0};
                if (nullptr == p_sFormat || '\0' == p_sFormat[0])
                {
                    hRes = p_itfObject->Invoke(p_didDispid, IID_NULL, LOCALE_USER_DEFAULT, p_iFlags,
                        &dpParams, p_vResult, nullptr, nullptr);
                }
                else
                {
                    std::vector<CComVariant> vParams;

                    for (const char *cpType = p_sFormat; '\0' != *cpType; ++cpType)
                    {
                        CComVariant vParam;

                        switch (*cpType)
                        {
                        case 'c':
                            vParams.push_back(CComVariant(char(va_arg(p_valParams, int))));
                            break;
                        case 'C':
                            vParams.push_back(CComVariant(va_arg(p_valParams, wchar_t)));
                            break;
                        case 's':
                            vParams.push_back(CComVariant(va_arg(p_valParams, const char *)));
                            break;
                        case 'S':
                            vParams.push_back(CComVariant(va_arg(p_valParams, const wchar_t *)));
                            break;
                        case 'd':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, int *));
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, int)));
                            }

                            break;
                        case 'D':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, __int64 *));
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, __int64)));
                            }

                            break;
                        case 'f':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, double *));
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, double)));
                            }

                            break;
                        case 'b':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, VARIANT_BOOL *));
                                vParam.vt = VT_BOOL | VT_BYREF; //atlcomcli.h中未定义CVarTypeInfo< VARIANT_BOOL* >类型，导致变量被误识别为CVarTypeInfo< short* >
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, VARIANT_BOOL)));
                            }

                            break;
                        case 'B':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, BSTR *));
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, BSTR)));
                            }

                            break;
                        case 'i':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, IUnknown **));
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, IUnknown *)));
                            }

                            break;
                        case 'I':
                            if ('*' == *(cpType + 1))
                            {
                                ++cpType;
                                vParam.SetByRef(va_arg(p_valParams, IDispatch **));
                                vParams.push_back(vParam);
                            }
                            else
                            {
                                vParams.push_back(CComVariant(va_arg(p_valParams, IDispatch *)));
                            }

                            break;
                        default:
                            return E_INVALIDARG;
                        }
                    }

                    //参数要按从后向前的顺序传入
                    std::vector<CComVariant> vRevertParams(vParams.rbegin(), vParams.rend());

                    dpParams.rgvarg = &vRevertParams[0];
                    dpParams.cArgs = vRevertParams.size();
                    hRes = p_itfObject->Invoke(p_didDispid, IID_NULL, LOCALE_USER_DEFAULT, p_iFlags,
                        &dpParams, p_vResult, nullptr, nullptr);
                }

                return hRes;
            }

        };

        class CManager
            : public NonCopyable
        {
        public:
            static IManager *GetManager()
            {
                return m_itfManager; //C++内部使用不加引用计数，由外部调用者在赋值时自行决定
            }

            static void InitManager(IManager *p_itfManager)
            {
                if (m_itfManager == p_itfManager)
                {
                    return;
                }

                if (nullptr != m_itfManager)
                {
                    m_itfManager->Release();
                    m_itfManager = nullptr;
                }

                m_itfManager = p_itfManager;
                if (nullptr != m_itfManager)
                {
                    m_itfManager->AddRef();
                }
            }

            static void PrintLog(const std::vector<std::string> &p_vMessage)
            {
                CComVariant v;
                HRESULT hRes = vcl4c::itf::CManager::GetManager()->GetService(CComBSTR(L"Log"), &v);
                if (FAILED(hRes))
                {
                    return;
                }

                for (auto it = p_vMessage.begin(); it != p_vMessage.end(); ++it)
                {
                    vcl4c::itf::CDispatchHelper::DispatchInvoke(&v, "AddLog", DISPATCH_METHOD, nullptr, "s", it->c_str());
                }
            }

            static void PrintLog(const std::string &p_sMessage)
            {
                CComVariant v;
                HRESULT hRes = vcl4c::itf::CManager::GetManager()->GetService(CComBSTR(L"Log"), &v);
                if (FAILED(hRes))
                {
                    return;
                }

                vcl4c::itf::CDispatchHelper::DispatchInvoke(&v, "AddLog", DISPATCH_METHOD, nullptr, "s", p_sMessage.c_str());
            }

            static void PrintLog(const BSTR &p_sMessage)
            {
                CComVariant v;
                HRESULT hRes = vcl4c::itf::CManager::GetManager()->GetService(CComBSTR(L"Log"), &v);
                if (FAILED(hRes))
                {
                    return;
                }

                vcl4c::itf::CDispatchHelper::DispatchInvoke(&v, "AddLog", DISPATCH_METHOD, nullptr, "S", p_sMessage);
            }

            static VARIANT &GetOptional()
            {
                return m_vOptional;
            }

        private:
            static IManager * m_itfManager;
            static VARIANT m_vOptional;

        };

        template <class CThreadModel, class CInvoke = CLocalInvoke>
        class CMemoryBlock:
            public CInterfaceBase<CThreadModel>,
            public IMemoryBlock
        {
            BEGIN_DEFINE_MAP(CMemoryBlock)
                SIMPLE_INTERFACE(IMemoryBlock)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

        public:
            explicit CMemoryBlock()
            {
            }

        public:
            //IMemoryBlock实现
            STDMETHOD(GetEncoding)(BYTE *p_iResult)
            {
                if (nullptr == p_iResult)
                {
                    return E_POINTER;
                }

                *p_iResult = m_etEncoding;

                return S_OK;
            }

            STDMETHOD(SetEncoding)(BYTE p_iValue)
            {
                /* TODO: 添加编码集转换的代码 */
                m_etEncoding = p_iValue;

                return S_OK;
            }

            STDMETHOD(GetSize)(ULONG *p_iResult)
            {
                if (nullptr == p_iResult)
                {
                    return E_POINTER;
                }

                *p_iResult = m_sBuffer.size();

                return S_OK;
            }

            STDMETHOD(SetSize)(ULONG p_iValue)
            {
                m_sBuffer.resize(p_iValue);

                return S_OK;
            }

            STDMETHOD(Read)(ULONG p_iPosition, BYTE *p_pValue, ULONG *p_piLength)
            {
                if (nullptr == p_pValue || nullptr == p_piLength)
                {
                    return E_POINTER;
                }

                if (p_iPosition >= m_sBuffer.size())
                {
                    return E_INVALIDARG;
                }

                if (p_iPosition + *p_piLength > m_sBuffer.size())
                {
                    *p_piLength = m_sBuffer.size() - p_iPosition;
                }

                memcpy(p_pValue, m_sBuffer.c_str() + p_iPosition, *p_piLength);

                return S_OK;
            }

            STDMETHOD(Write)(ULONG p_iPosition, BYTE *p_pValue, ULONG p_iLength)
            {
                if (nullptr == p_pValue)
                {
                    return E_POINTER;
                }

                if (p_iPosition + p_iLength >= m_sBuffer.size())
                {
                    return E_INVALIDARG;
                }

                memcpy(const_cast<char *>(m_sBuffer.c_str() + p_iPosition), p_pValue, p_iLength);

                return S_OK;
            }

        private:
            std::string m_sBuffer;
            EEncodingType m_etEncoding;

        };

        __declspec(selectany) IManager *CManager::m_itfManager = nullptr;
        __declspec(selectany) VARIANT CManager::m_vOptional = {VT_ERROR, 0, 0, 0, (ULONG)DISP_E_PARAMNOTFOUND};

    }
}
