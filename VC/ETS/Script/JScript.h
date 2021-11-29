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

#include "resource.h"
#include "IDL/IScript.h"
#include "File.h"

namespace ets
{
    namespace jscript
    {
        #include "../../ThirdParty/msscript/msscript.tlh"

        class CJScript:
            public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
            public IScript
        {
            BEGIN_DEFINE_MAP(CJScript)
                SIMPLE_INTERFACE(IDispatch)
                SIMPLE_INTERFACE(IScript)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

            enum {IDL = IDTL_SCRIPT};

        public:
            CJScript()
                : CDispatch(IDL, &__uuidof(IScript), NULL),
                m_iEventCookie(-1)
            {
            }

            virtual ~CJScript()
            {
                if (m_iEventCookie >= 0)
                {
                    AtlUnadvise(m_scMsscript, __uuidof(DScriptControlSource), m_iEventCookie);
                    m_iEventCookie = -1;
                }

                m_scMsscript->Reset();
            }

            HRESULT Init()
            {
                //1.0 加载msscript控件
                if (nullptr == m_hMsscript)
                {
                    CSingleLock slLock(&m_csLock);
                    if (!slLock.Lock())
                    {
                        return E_FAIL;
                    }

                    if (nullptr != m_hMsscript)
                    {
                        return S_OK;
                    }

                    m_hMsscript = ::LoadLibrary("Lib/JScript/msscript.ocx");
                    if (nullptr == m_hMsscript)
                    {
                        return E_FAIL;
                    }

                    m_funcClassFactory = reinterpret_cast<FClassFactory>(::GetProcAddress(m_hMsscript, "DllGetClassObject"));
                    if (nullptr == m_funcClassFactory)
                    {
                        ::FreeLibrary(m_hMsscript);
                        m_hMsscript = nullptr;
                        return E_FAIL;
                    }
                }

                //2.0 调用类厂接口，创建IScriptControl实例
                CComPtr<IClassFactory> cf;
                HRESULT hRes = m_funcClassFactory(__uuidof(ScriptControl), __uuidof(IClassFactory), &cf);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = cf->CreateInstance(nullptr, __uuidof(IScriptControl), reinterpret_cast<void**>(&m_scMsscript));
                if (FAILED(hRes))
                {
                    return hRes;
                }

                //3.0 IScriptControl初始化
                hRes = m_scMsscript->put_Language(CComBSTR(L"JScript"));
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = m_scMsscript->put_AllowUI(VARIANT_FALSE);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = m_scMsscript->put_Timeout(-1);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                //4.0 注册连接点，响应脚本报错事件
                m_objEvent.AddRef(); //由于m_ErrorSink定义为成员变量，这里加1是为了防止其被意外析构

                hRes = AtlAdvise(m_scMsscript, m_objEvent.GetUnknown(), __uuidof(DScriptControlSource), &m_iEventCookie);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

            HRESULT AddCode(BSTR p_sCode)
            {
                HRESULT hRes = m_scMsscript->AddCode(p_sCode);
                return hRes;
            }

            HRESULT AddModule(BSTR p_sFileName, IDispatch** p_itfResult)
            {
                //1.0 读取文件源码
                CComBSTR bsCode;
                HRESULT hRes = ReadFile(bsCode, p_sFileName);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                //2.0 创建新模块
                CComPtr<IScriptModuleCollection> smc;
                hRes = m_scMsscript->get_Modules(&smc);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                CComPtr<IScriptModule> sm;
                hRes = smc->Add(p_sFileName, &vcl4c::itf::CManager::GetOptional(), &sm);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                //3.0 将源码添加到模块中
                hRes = sm->AddCode(bsCode);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = sm->get_CodeObject(p_itfResult);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

            HRESULT AddObject(BSTR p_sName, IDispatch* p_itfObject, VARIANT_BOOL p_bAddMembers)
            {
                return m_scMsscript->AddObject(p_sName, p_itfObject, p_bAddMembers);
            }

            HRESULT Eval(BSTR p_sCode, BSTR* p_sResult)
            {
                CComVariant vResult;
                HRESULT hRes = m_scMsscript->Eval(p_sCode, &vResult);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = vResult.ChangeType(VT_BSTR);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = vResult.CopyTo(p_sResult);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

        public:
            //IScript实现
            STDMETHOD(RegContainer)(IDispatch* p_itfContainer)
            {
                m_itfContainer = p_itfContainer;

                //核心模块初始化(从Init中移到这里实现，否则，由于m_itfContainer为空，Cache功能无法正常处理)
                m_objEts.AddRef();

                HRESULT hRes = AddObject(CComBSTR(L"Ets"), CComQIPtr<IDispatch>(&m_objEts), VARIANT_FALSE);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                CComBSTR bsEtsCode;
                hRes = ReadFile(bsEtsCode, L"Lib/JScript/Lib/Ets.js");
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = AddCode(bsEtsCode);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

            STDMETHOD(RegFrame)(IDispatch* p_itfFrame)
            {
                m_itfFrame = p_itfFrame;

                return S_OK;
            }

            STDMETHOD(RunModule)(BSTR p_sFileName, BSTR p_sEntryFunction)
            {
                CComBSTR bsCode;
                HRESULT hRes = ReadFile(bsCode, p_sFileName);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                hRes = AddCode(bsCode);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                CComBSTR bsResult;
                hRes = Eval(p_sEntryFunction, &bsResult);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

            STDMETHOD(RunCode)(BSTR p_sCode)
            {
                CComBSTR bsResult;
                HRESULT hRes = Eval(p_sCode, &bsResult);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

        private:
            CComPtr<IScriptControl> m_scMsscript;
            DWORD m_iEventCookie;
            CComPtr<IDispatch> m_itfContainer;
            CComPtr<IDispatch> m_itfFrame;

            class CEvent:
                public vcl4c::itf::CInterfaceBase<ATL::CComMultiThreadModel>,
                public DScriptControlSource
            {
                BEGIN_DEFINE_MAP(CEvent)
                    SIMPLE_INTERFACE(IDispatch)
                    SIMPLE_INTERFACE(DScriptControlSource)
                END_DEFINE_MAP()

                IMPLEMENT_IUNKNOWN()

            public:
                //IDispatch实现
                STDMETHOD(GetTypeInfoCount)(_Out_ UINT* p_iTypeInfoCount)
                {
                    if (NULL == p_iTypeInfoCount)
                    {
                        return E_POINTER;
                    }

                    *p_iTypeInfoCount = 0;

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

                    return E_NOTIMPL;
                }

                STDMETHOD(GetIDsOfNames)(_In_ REFIID /*p_IID*/,
                    _In_count_(p_iNamesCount) _Deref_pre_z_ LPOLESTR* p_sNames, _In_ UINT p_iNamesCount,
                    _In_ LCID p_iLocaleID, _Out_ DISPID* p_DispID)
                {
                    return E_NOTIMPL;
                }

                STDMETHOD(Invoke)(_In_ DISPID p_DispID, _In_ REFIID /*p_IID*/,
                    _In_ LCID p_iLocaleID, _In_ WORD p_iFlags,
                    _In_ DISPPARAMS* p_Params,
                    _Out_opt_ VARIANT* p_vResult, _Out_opt_ EXCEPINFO* p_eiExceptionInfo, _Out_opt_ UINT* p_iArgumentError)
                {
                    switch (p_DispID)
                    {
                    case 3000:
                        reinterpret_cast<CJScript *>(
                            reinterpret_cast<char *>(this) - offsetof(CJScript, m_objEvent))->DoError();
                        return S_OK;
                    case 3001:
                        reinterpret_cast<CJScript *>(
                            reinterpret_cast<char *>(this) - offsetof(CJScript, m_objEvent))->DoTimeout();
                        return S_OK;
                    default:
                        return E_NOTIMPL;
                    }
                }

            } m_objEvent;

            class CEtsForScript:
                public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
                public IEtsForScript
            {
                BEGIN_DEFINE_MAP(CEtsForScript)
                    SIMPLE_INTERFACE(IDispatch)
                    SIMPLE_INTERFACE(IEtsForScript)
                END_DEFINE_MAP()

                IMPLEMENT_IUNKNOWN()

                enum {IDL = IDTL_SCRIPT};

            public:
                CEtsForScript()
                    : CDispatch(IDL, &__uuidof(IEtsForScript), NULL)
                {
                }

                CJScript *GetParent()
                {
                    return reinterpret_cast<CJScript *>(
                        reinterpret_cast<char *>(this) - offsetof(CJScript, m_objEts));
                }

            public:
                //IEtsForScript实现
                STDMETHOD(FileExists)(BSTR p_sFileName, VARIANT_BOOL* p_bResult)
                {
                    ::std::string sFileName;
                    HRESULT hRes = vcl4c::itf::W2String(sFileName, p_sFileName);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    *p_bResult = (vcl4c::file::FileExists(sFileName)) ? VARIANT_TRUE : VARIANT_FALSE;

                    return S_OK;
                }

                STDMETHOD(GetFrame)(BSTR p_sPathName, IDispatch** p_itfResult)
                {
                    CComBSTR str = p_sPathName;
                    if (str == L"")
                    {
                        return GetParent()->m_itfFrame.CopyTo(p_itfResult);
                    }
                    else
                    {
                        CComVariant vResult;
                        return vcl4c::itf::CDispatchHelper::DispatchInvoke(GetParent()->m_itfFrame,
                            "GetFrame", DISPATCH_METHOD, &vResult, "BI*", p_sPathName, p_itfResult);
                    }
                }

                STDMETHOD(GetSetting)(BSTR p_sName, BSTR* p_sResult)
                {
                    CComVariant v;
                    HRESULT hRes = vcl4c::itf::CManager::GetManager()->GetService(CComBSTR(L"Setting"), &v);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    CComVariant vResult;
                    hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(&v, "GetValueByPath", DISPATCH_METHOD, &vResult,
                        "SS", p_sName, L"");
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    if (VT_BSTR != vResult.vt)
                    {
                        hRes = vResult.ChangeType(VT_BSTR);
                        if (FAILED(hRes))
                        {
                            return hRes;
                        }
                    }

                    hRes = vResult.CopyTo(p_sResult);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    return S_OK;
                }

                STDMETHOD(GetStop)(VARIANT_BOOL* p_bResult)
                {
                    if (nullptr == p_bResult)
                    {
                        return E_POINTER;
                    }

                    if (nullptr == GetParent()->m_itfContainer)
                    {
                        *p_bResult = VARIANT_TRUE;
                        return S_OK;
                    }

                    CComVariant vResult;
                    HRESULT hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(GetParent()->m_itfContainer,
                        "Terminated", DISPATCH_PROPERTYGET, &vResult, "");
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    if (VT_BOOL != vResult.vt)
                    {
                        hRes = vResult.ChangeType(VT_BOOL);
                        if (FAILED(hRes))
                        {
                            return hRes;
                        }
                    }

                    *p_bResult = vResult.boolVal;

                    return S_OK;
                }

                STDMETHOD(GetTicketCount)(LONG* p_iResult)
                {
                    *p_iResult = ::GetTickCount();

                    return S_OK;
                }

                STDMETHOD(LoadPlugin)(BSTR p_sFileName, IDispatch** p_itfResult)
                {
                    return vcl4c::itf::CManager::GetManager()->GetPlugins(p_sFileName, p_itfResult);
                }

                STDMETHOD(Log)(BSTR p_sMessage, BSTR p_sFileName)
                {
                    HRESULT hRes = S_OK;

                    ::std::string sFileName;
                    hRes = vcl4c::itf::W2String(sFileName, p_sFileName);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    ::std::string sMessage;
                    hRes = vcl4c::itf::W2String(sMessage, p_sMessage);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    if (!vcl4c::file::Write(sMessage, sFileName))
                    {
                        return E_FAIL;
                    }

                    return S_OK;
                }

                STDMETHOD(Require)(BSTR p_sModuleName, IDispatch** p_itfResult)
                {
                    return GetParent()->AddModule(p_sModuleName, p_itfResult);
                }

                STDMETHOD(ShowLog)(BSTR p_sMessage)
                {
                    vcl4c::itf::CManager::PrintLog(p_sMessage);

                    return S_OK;
                }

                STDMETHOD(Sleep)(LONG p_iMiliSeconds)
                {
                    ::Sleep(p_iMiliSeconds);

                    return S_OK;
                }

            } m_objEts;

            void DoError()
            {
                CComPtr<IScriptError> seError;
                HRESULT hRes = m_scMsscript->get_Error(&seError);
                if (FAILED(hRes))
                {
                    vcl4c::itf::CManager::PrintLog("无法获取错误信息");
                    return;
                }

                long iNumber = 0, iLine = 0, iColumn = 0;
                CComBSTR bsSource, bsDescription;

                seError->get_Number(&iNumber);
                seError->get_Line(&iLine);
                seError->get_Column(&iColumn);
                seError->get_Source(&bsSource);
                seError->get_Description(&bsDescription);

                ::std::string sSource, sDescription;
                vcl4c::itf::W2String(sSource, bsSource);
                vcl4c::itf::W2String(sDescription, bsDescription);

                vcl4c::itf::CManager::PrintLog(vcl4c::string::Format(
                    "JScript执行出错 [%d] (%d行/%d列)：\r\n    %s\r\n    %s",
                    iNumber, iLine, iColumn, sSource.c_str(), sDescription.c_str()));
            }

            void DoTimeout()
            {
                vcl4c::itf::CManager::PrintLog("执行超时");
            }

            HRESULT ReadFile(CComBSTR &p_bsResult, const wchar_t *p_sFileName) const
            {
                HRESULT hRes = S_OK;

                ::std::string sFileName;
                hRes = vcl4c::itf::W2String(sFileName, p_sFileName);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                //从Cache中读取(文件修改后未保存)
                if (nullptr != m_itfContainer)
                {
                    CComVariant vResult;
                    CComPtr<IUnknown> itfCode;
                    VARIANT_BOOL bCached = VARIANT_FALSE;
                    hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(m_itfContainer,
                        "GetCacheFile", DISPATCH_METHOD, &vResult, "Si*b*", p_sFileName, &itfCode, &bCached);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    if (VARIANT_TRUE == bCached)
                    {
                        CComQIPtr<IMemoryBlock> itfMem = itfCode;
                        if (!itfMem)
                        {
                            return E_FAIL;
                        }

                        ULONG iLen = 0;
                        hRes = itfMem->GetSize(&iLen);
                        if (FAILED(hRes))
                        {
                            return hRes;
                        }

                        ::std::string sResult;
                        sResult.resize(iLen);
                        hRes = itfMem->Read(0, reinterpret_cast<BYTE *>(const_cast<char *>(sResult.c_str())), &iLen);
                        if (FAILED(hRes))
                        {
                            return hRes;
                        }

                        /* TODO: 需增加字符集自动检测及转换的代码 */
                        hRes = vcl4c::itf::A2BSTR(p_bsResult, sResult.c_str());
                        if (FAILED(hRes))
                        {
                            return hRes;
                        }

                        return S_OK;
                    }
                }

                //从文件读取
                ::std::string sCode;
                if (!vcl4c::file::Read(sCode, sFileName))
                {
                    return E_ACCESSDENIED;
                }

                hRes = vcl4c::itf::A2BSTR(p_bsResult, sCode.c_str());
                if (FAILED(hRes))
                {
                    return hRes;
                }

                return S_OK;
            }

        private:
            static CCriticalSection m_csLock;
            static HMODULE m_hMsscript;
            typedef HRESULT (__stdcall * FClassFactory)(REFCLSID p_Clsid, REFIID p_IID, IClassFactory** p_Result);
            static FClassFactory m_funcClassFactory;

        public:
            static void UnLoad()
            {
                if (nullptr != m_hMsscript)
                {
                    ::FreeLibrary(m_hMsscript);
                    m_hMsscript = nullptr;
                    m_funcClassFactory = nullptr;
                }
            }

        };

        __declspec(selectany) CCriticalSection CJScript::m_csLock;
        __declspec(selectany) HMODULE CJScript::m_hMsscript = nullptr;
        __declspec(selectany) CJScript::FClassFactory CJScript::m_funcClassFactory = nullptr;

    }
}
