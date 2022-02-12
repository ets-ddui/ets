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

#include "resource.h"
#include "IDL/IScript.h"
#include "Debug.h"
#include "File.h"

namespace ets
{
    namespace python
    {
        #include "../../ThirdParty/python/include/Python.h"
        #include "../../ThirdParty/python/include/frameobject.h"

        /** PyObjectָ�������
            @brief �Զ����PyObject�ϵ����ü�������
        */
        class CAuto
        {
        public:
            explicit CAuto(PyObject *p_Object)
                : m_Object(p_Object)
            {
            }

            ~CAuto()
            {
                Py_XDECREF(m_Object);
                m_Object = nullptr;
            }

            PyObject & operator *()
            {
                return *m_Object;
            }

            operator PyObject *() const
            {
                return m_Object;
            }

            bool operator !() const
            {
                return nullptr == m_Object;
            }

        private:
            PyObject *m_Object;

        };

        class CPython:
            public vcl4c::itf::CDispatch<ATL::CComMultiThreadModel>,
            public IScript
        {
            BEGIN_DEFINE_MAP(CPython)
                SIMPLE_INTERFACE(IDispatch)
                SIMPLE_INTERFACE(IScript)
            END_DEFINE_MAP()

            IMPLEMENT_IUNKNOWN()

            enum {IDL = IDTL_SCRIPT};

        public:
            CPython()
                : CDispatch(IDL, &__uuidof(IScript), NULL)
            {
            }

            virtual ~CPython()
            {
                long iCount = InterlockedDecrement(&m_iInstanceCount);
                if (0 != iCount)
                {
                    return;
                }

                Py_Finalize();
            }

            HRESULT Init()
            {
                long iCount = InterlockedIncrement(&m_iInstanceCount);
                if (1 != iCount)
                {
                    //��ֹ���߳�����(Python��ȫ���������߳�û����)
                    return E_ACCESSDENIED;
                }

                //����ģ��Ĳ���·��
                _putenv("PYTHONPATH=./Lib/Python/DLLs;./Lib/Python/Lib");

                Py_Initialize();
                PyImport_ExtendInittab(m_itExtendModule);

                return S_OK;
            }

        public:
            //IScriptʵ��
            STDMETHOD(RegContainer)(IDispatch* p_itfContainer)
            {
                m_itfContainer = p_itfContainer;

                return S_OK;
            }

            STDMETHOD(RegFrame)(IDispatch* p_itfFrame)
            {
                m_itfFrame = p_itfFrame;

                return S_OK;
            }

            STDMETHOD(RunModule)(BSTR p_sFileName, BSTR p_sEntryFunction)
            {
                HRESULT hRes = S_OK;

                ::std::string sCode;
                if (!ReadFile(sCode, p_sFileName))
                {
                    return E_FAIL;
                }

                PyObject *objSysPath = nullptr;
                objSysPath = PySys_GetObject("path");
                if (nullptr != objSysPath)
                {
                    ::std::string sFileName;
                    hRes = vcl4c::itf::W2String(sFileName, p_sFileName);
                    if (FAILED(hRes))
                    {
                        return hRes;
                    }

                    ::std::string sPath, sName;
                    vcl4c::file::Split(sPath, sName, sFileName);

                    if (!sPath.empty())
                    {
                        CAuto objPath(PyString_FromString(sPath.c_str()));
                        PyList_Insert(objSysPath, 0, objPath);
                    }
                }

                int iReturn = PyRun_SimpleString(sCode.c_str());
                if (0 > iReturn)
                {
                    //PyRun_SimpleStringִ��ʧ�ܺ󣬻����PyErr_Print��������Ϣ�����stderr�У�
                    //ͬʱ��������Ϣ��¼��last_type��last_value��last_traceback�����У�
                    //���ڴ��ڳ����޷����stderr�����ݣ���ˣ���дPyErr_Print���߼�����last_XXX�������������־����
                    PrintLastError();
                    return E_FAIL;
                }

                return S_OK;
            }

            STDMETHOD(RunCode)(BSTR p_sCode)
            {
                ::std::string sCode;
                HRESULT hRes = vcl4c::itf::W2String(sCode, p_sCode);
                if (FAILED(hRes))
                {
                    return hRes;
                }

                int iReturn = PyRun_SimpleString(sCode.c_str());
                if (0 > iReturn)
                {
                    PrintLastError();
                    return E_FAIL;
                }

                return S_OK;
            }

        private:
            static _inittab m_itExtendModule[];
            static long m_iInstanceCount;
            static CComPtr<IDispatch> m_itfContainer;
            static CComPtr<IDispatch> m_itfFrame;

            /** Python��չģ��ETS��ʵ����
            */
            class CEtsForPython
            {
            public:
                static char m_sModuleName[];

                static void Entry(void)
                {
                    if (PyType_Ready(&m_toDispatch) < 0)
                    {
                        return;
                    }

                    m_toFunction.tp_name = "ets.Dispatch.Function";
                    m_toFunction.tp_basicsize = sizeof(CFunctionForPython);
                    m_toFunction.tp_getattr = nullptr;
                    m_toFunction.tp_setattr = nullptr;
                    m_toFunction.tp_call = CFunctionForPython::Call;
                    m_toFunction.tp_doc = "��Dispatch��Ա�����İ�װ";
                    if (PyType_Ready(&m_toFunction) < 0)
                    {
                        return;
                    }

                    PyObject *objModule = Py_InitModule3(m_sModuleName, m_mdEntry, "ETS�ӿ�ģ�飬������Python�з���ETS�Ĺ���");
                    if (nullptr == objModule)
                    {
                        return;
                    }

                    Py_INCREF(&m_toDispatch);
                    PyModule_AddObject(objModule, "Dispatch", (PyObject *)&m_toDispatch);
                }

            private:
                //etsģ���еķ���ʵ��
                static PyMethodDef m_mdEntry[];

                static bool ObjectToString(std::string &p_sResult, PyObject *p_objValue)
                {
                    if (!p_sResult.empty())
                    {
                        p_sResult.push_back(',');
                    }

                    if (PyString_Check(p_objValue))
                    {
                        p_sResult.append(PyString_AsString(p_objValue));
                    }
                    else if (PyUnicode_Check(p_objValue))
                    {
                        std::string str;
                        if (FAILED(vcl4c::itf::W2String(str, PyUnicode_AsUnicode(p_objValue))))
                        {
                            return false;
                        }

                        p_sResult.append(str);
                    }
                    else
                    {
                        CAuto strItem(PyObject_Str(p_objValue));
                        if (nullptr == strItem)
                        {
                            return false;
                        }

                        return ObjectToString(p_sResult, strItem);
                    }

                    return true;
                }

                static PyObject *GetFrame(PyObject *p_Self, PyObject *p_Args)
                {
                    if (!CPython::m_itfFrame)
                    {
                        PyErr_SetString(PyExc_TypeError, "�����ڿؼ�������");
                        return nullptr;
                    }

                    int iLen = PyTuple_Size(p_Args);
                    if (0 == iLen)
                    {
                        return PyDispatch_FromDispatch(CPython::m_itfFrame);
                    }
                    else if (1 == iLen)
                    {
                        PyObject *objItem = PyTuple_GetItem(p_Args, 0); //���ص�ָ���ǽ�������
                        if (nullptr == objItem)
                        {
                            return nullptr;
                        }

                        std::string str;
                        if (!ObjectToString(str, objItem))
                        {
                            PyErr_SetString(PyExc_TypeError, "�������Ͳ��Ϸ�");
                            return nullptr;
                        }

                        CComVariant vResult;
                        IDispatch *itfResult = nullptr;
                        HRESULT hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(CPython::m_itfFrame,
                            "GetFrame", DISPATCH_METHOD, &vResult, "sI*", str.c_str(), &itfResult);
                        if (FAILED(hRes))
                        {
                            PyErr_SetString(PyExc_TypeError, vcl4c::string::Format("�ؼ�[%s]��ȡʧ��", str.c_str()).c_str());
                            return nullptr;
                        }

                        return PyDispatch_FromDispatch(itfResult);
                    }
                    else
                    {
                        PyErr_SetString(PyExc_TypeError, vcl4c::string::Format("��������[%d]���Ϸ�", iLen).c_str());
                        return nullptr;
                    }
                }

                static PyObject *Show(PyObject *p_Self, PyObject *p_Args)
                {
                    int iLen = PyTuple_Size(p_Args);
                    for (int i = 0; i < iLen; ++i)
                    {
                        PyObject *objItem = PyTuple_GetItem(p_Args, i); //���ص�ָ���ǽ�������
                        if (nullptr == objItem)
                        {
                            return nullptr;
                        }

                        std::string str;
                        if (!ObjectToString(str, objItem))
                        {
                            PyErr_SetString(PyExc_TypeError, vcl4c::string::Format("�޷�ʶ�����[%d]����������", i).c_str());
                            return nullptr;
                        }

                        vcl4c::itf::CManager::PrintLog(str);
                    }

                    Py_RETURN_NONE;
                }

                static PyObject *Stop(PyObject *p_Self, PyObject *p_Args)
                {
                    CComVariant vResult;
                    HRESULT hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(CPython::m_itfContainer,
                        "Terminated", DISPATCH_PROPERTYGET, &vResult, "");
                    if (FAILED(hRes))
                    {
                        PyErr_SetString(PyExc_RuntimeError, "Terminated����ʧ��");
                        return nullptr;
                    }

                    if (VT_BOOL != vResult.vt)
                    {
                        hRes = vResult.ChangeType(VT_BOOL);
                        if (FAILED(hRes))
                        {
                            return nullptr;
                        }
                    }

                    if (VARIANT_TRUE == vResult.boolVal)
                    {
                        return PyBool_FromLong(TRUE);
                    }
                    else
                    {
                        return PyBool_FromLong(FALSE);
                    }
                }

            private:
                //etsģ���е�Dispatch���͵�ʵ��
                static PyTypeObject m_toDispatch;
                static PyTypeObject m_toFunction;

                struct CDispatchForPython
                {
                    PyObject_HEAD
                    IDispatch *m_itfObject;
                    std::map<std::string, INVOKEKIND> *m_mapNames;

                    static void DeAlloc(PyObject *p_Self)
                    {
                        CDispatchForPython *obj = reinterpret_cast<CDispatchForPython *>(p_Self);

                        if (nullptr != obj->m_itfObject)
                        {
                            obj->m_itfObject->Release();
                            obj->m_itfObject = nullptr;
                        }

                        if (nullptr != obj->m_mapNames)
                        {
                            delete obj->m_mapNames;
                            obj->m_mapNames = nullptr;
                        }

                        obj->ob_type->tp_free(obj);
                    }

                    static int Init(PyObject *p_Self, PyObject *p_Args, PyObject *p_Kwds)
                    {
                        CDispatchForPython *obj = reinterpret_cast<CDispatchForPython *>(p_Self);

                        if (nullptr != obj->m_itfObject)
                        {
                            obj->m_itfObject->Release();
                            obj->m_itfObject = nullptr;
                        }

                        if (0 == _PyArg_NoKeywords("ets.Dispatch()", p_Kwds))
                        {
                            return -1;
                        }

                        char *sFileName = nullptr;
                        if (0 == PyArg_ParseTuple(p_Args, "s", &sFileName))
                        {
                            PyErr_SetString(PyExc_RuntimeError, "������������ȷ");
                            return -1;
                        }

                        CComBSTR bsFileName;
                        HRESULT hRes = vcl4c::itf::A2BSTR(bsFileName, sFileName);
                        if (FAILED(hRes))
                        {
                            PyErr_SetString(PyExc_RuntimeError, "�������Ͳ��Ϸ�");
                            return -1;
                        }

                        hRes = vcl4c::itf::CManager::GetManager()->GetPlugins(bsFileName, &obj->m_itfObject);
                        if (FAILED(hRes))
                        {
                            PyErr_SetString(PyExc_RuntimeError,
                                vcl4c::string::Format("ģ��(%s)����ʧ��", sFileName).c_str());
                            return -1;
                        }

                        return 0;
                    }

                    static PyObject *GetAttr(PyObject *p_Self, char *p_sName)
                    {
                        CDispatchForPython *obj = reinterpret_cast<CDispatchForPython *>(p_Self);

                        if (nullptr == obj->m_mapNames)
                        {
                            obj->m_mapNames = new std::map<std::string, INVOKEKIND>();
                            InitNames(*obj->m_mapNames, obj->m_itfObject);
                        }

                        const auto it = obj->m_mapNames->find(p_sName);
                        if (it != obj->m_mapNames->end())
                        {
                            if (INVOKE_PROPERTYGET == it->second)
                            {
                                CComVariant v;
                                HRESULT hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                    obj->m_itfObject, p_sName, DISPATCH_PROPERTYGET, &v, "");
                                if (FAILED(hRes))
                                {
                                    PyErr_SetString(PyExc_RuntimeError, vcl4c::string::Format("��ȡ����[%s]ʧ��", p_sName).c_str());
                                    return nullptr;
                                }

                                return VariantToPython(v);
                            }
                        }

                        return PyFunction_FromDispatch(obj->m_itfObject, p_sName);
                    }

                    static int SetAttr(PyObject *p_Self, char *p_sName, PyObject *p_Value)
                    {
                        CDispatchForPython *obj = reinterpret_cast<CDispatchForPython *>(p_Self);

                        HRESULT hRes = S_OK;
                        CComVariant v;
                        if (PyDispatch_Check(p_Value))
                        {
                            CDispatchForPython *objValue = reinterpret_cast<CDispatchForPython *>(p_Value);
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "I", objValue->m_itfObject);
                        }
                        else if (PyInt_Check(p_Value))
                        {
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "d", int(PyInt_AS_LONG(p_Value)));
                        }
                        else if (PyLong_Check(p_Value))
                        {
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "D", PyLong_AsUnsignedLongLongMask(p_Value));
                        }
                        else if (PyFloat_Check(p_Value))
                        {
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "f", PyFloat_AS_DOUBLE(p_Value));
                        }
                        else if (PyByteArray_Check(p_Value))
                        {
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "s", PyByteArray_AsString(p_Value));
                        }
                        else if (PyString_Check(p_Value))
                        {
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "s", PyString_AS_STRING(p_Value));
                        }
                        else if (PyUnicode_Check(p_Value))
                        {
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "S", PyUnicode_AS_UNICODE(p_Value));
                        }
                        else if (PyCallable_Check(p_Value))
                        {
                            CEvent *e = new(std::nothrow) CEvent(p_Value);
                            if (nullptr == e)
                            {
                                PyErr_SetString(PyExc_MemoryError, "CEvent����ʧ��");
                                return -1;
                            }

                            IDispatch *itf = static_cast<IDispatch *>(e);
                            itf->AddRef();
                            hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(
                                obj->m_itfObject, p_sName, DISPATCH_PROPERTYPUT, &v, "I", itf);
                            itf->Release();
                        }
                        else
                        {
                            PyErr_SetString(PyExc_TypeError, "�޷�ʶ����������");
                            return -1;
                        }

                        if (FAILED(hRes))
                        {
                            PyErr_SetString(PyExc_RuntimeError, vcl4c::string::Format("��������[%s]ʧ��", p_sName).c_str());
                            return -1;
                        }

                        return 0;
                    }

                private:
                    static void InitNames(std::map<std::string, INVOKEKIND> &p_mapResult, IDispatch *p_itfObject)
                    {
                        p_mapResult.clear();

                        UINT iTypeInfoCount = 0;
                        HRESULT hRes = p_itfObject->GetTypeInfoCount(&iTypeInfoCount);
                        if (FAILED(hRes) || 0 == iTypeInfoCount)
                        {
                            return;
                        }

                        ITypeInfo *ti = nullptr;
                        hRes = p_itfObject->GetTypeInfo(0, 0, &ti);
                        if (FAILED(hRes))
                        {
                            return;
                        }
                        CComPtr<ITypeInfo> cpTypeInfo; //����ʱ�ͷ����ü���
                        cpTypeInfo.Attach(ti);

                        TYPEATTR* ta;
                        hRes = cpTypeInfo->GetTypeAttr(&ta);
                        if (FAILED(hRes))
                        {
                            return;
                        }
                        std::unique_ptr<TYPEATTR, CTypeAttrDeleter> upTypeAttr(ta, CTypeAttrDeleter(ti));

                        for (int i = 0; i < upTypeAttr->cFuncs; ++i)
                        {
                            FUNCDESC* fd = nullptr;
                            hRes = cpTypeInfo->GetFuncDesc(i, &fd);
                            if (FAILED(hRes))
                            {
                                continue;
                            }
                            std::unique_ptr<FUNCDESC, CFuncDescDeleter> upFuncDesc(fd, CFuncDescDeleter(ti));

                            //��ʱֻ֧�ֲ������Ӳ������������ͣ���ˣ�SetAttr��ֱ�ӵ���IDispatch.Invoke���ɣ������ߵ�������
                            //�ܴ����˺���ִ�е�ֻ����GetAttr�����ԣ������������õĳ���������ֱ������
                            //��Ҫע�⣬��/д������tlb�л���������¼�����ﲻ���˵Ļ���mapû����
                            if (INVOKE_PROPERTYPUT == upFuncDesc->invkind || INVOKE_PROPERTYPUTREF == upFuncDesc->invkind)
                            {
                                continue;
                            }

                            CComBSTR bsName;
                            if (SUCCEEDED(cpTypeInfo->GetDocumentation(upFuncDesc->memid, &bsName, NULL, NULL, NULL)))
                            {
                                USES_CONVERSION_EX;
                                char *s = W2A_EX(bsName, bsName.Length());

                                p_mapResult.insert(std::make_pair(s, upFuncDesc->invkind));
                            }
                        }
                    }

                    struct CEvent
                        : public CInterfaceBase<ATL::CComMultiThreadModel>,
                        public IDispatch
                    {
                        BEGIN_DEFINE_MAP(CEvent)
                            SIMPLE_INTERFACE(IDispatch)
                        END_DEFINE_MAP()

                        IMPLEMENT_IUNKNOWN()

                    public:
                        explicit CEvent(PyObject *p_Function)
                            : CInterfaceBase(NULL), m_Function(p_Function)
                        {
                        }

                    public:
                        //IDispatchʵ��
                        STDMETHOD(GetTypeInfoCount)(_Out_ UINT* p_iTypeInfoCount)
                        {
                            return E_NOTIMPL;
                        }

                        STDMETHOD(GetTypeInfo)(_In_ UINT p_iTypeInfo, _In_ LCID p_iLocaleID, _Deref_out_ ITypeInfo** p_TypeInfo)
                        {
                            return E_NOTIMPL;
                        }

                        STDMETHOD(GetIDsOfNames)(_In_ REFIID /*p_IID*/,
                            _In_count_(p_iNamesCount) _Deref_pre_z_ LPOLESTR* p_sNames, _In_ UINT p_iNamesCount,
                            _In_ LCID p_iLocaleID, _Out_ DISPID* p_DispID)
                        {
                            if (1 == p_iNamesCount)
                            {
                                *p_DispID = 0;
                                return S_OK;
                            }
                            else
                            {
                                return E_INVALIDARG;
                            }
                        }

                        STDMETHOD(Invoke)(_In_ DISPID p_DispID, _In_ REFIID /*p_IID*/,
                            _In_ LCID p_iLocaleID, _In_ WORD p_iFlags,
                            _In_ DISPPARAMS* p_Params,
                            _Out_opt_ VARIANT* p_vResult, _Out_opt_ EXCEPINFO* p_eiExceptionInfo, _Out_opt_ UINT* p_iArgumentError)
                        {
                            if (DISPATCH_METHOD != p_iFlags)
                            {
                                return E_INVALIDARG;
                            }

                            //δ����ʱ��Delphi���Զ����һ��VT_ERROR���͵Ĳ�����ֵΪDISP_E_PARAMNOTFOUND������������
                            if (1 == p_Params->cArgs && VT_ERROR == p_Params->rgvarg[0].vt && DISP_E_PARAMNOTFOUND == p_Params->rgvarg[0].lVal)
                            {
                                p_Params->cArgs = 0;
                            }

                            PyObject *tup = PyTuple_New(p_Params->cArgs);
                            if (nullptr == tup)
                            {
                                return E_OUTOFMEMORY;
                            }

                            for (int iParam = p_Params->cArgs - 1, iTuple = 0; iParam >= 0; --iParam, ++iTuple)
                            {
                                PyTuple_SetItem(tup, iTuple, VariantToPython(p_Params->rgvarg[iParam]));
                            }

                            PyObject *objResult = PyObject_Call(m_Function, tup, nullptr);
                            if (nullptr == objResult)
                            {
                                return E_FAIL;
                            }
                            Py_DECREF(tup);
                            Py_DECREF(objResult);

                            return PyErr_Occurred() ? E_UNEXPECTED : S_OK;
                        }

                    private:
                        PyObject *m_Function;

                    };

                };

                struct CFunctionForPython
                {
                    PyObject_HEAD
                    IDispatch *m_itfObject;
                    DISPID m_didDispid;

                    static PyObject *Call(PyObject *p_Self, PyObject *p_Args, PyObject *p_Kwds)
                    {
                        if (0 == _PyArg_NoKeywords("Dispatch function call", p_Kwds))
                        {
                            return nullptr;
                        }

                        //1.0 ��Python��Σ�ת��ΪVariant����
                        std::vector<CComVariant> vParams;
                        int iLen = PyTuple_Size(p_Args);
                        for (int i = 0; i < iLen; ++i)
                        {
                            PyObject *objItem = PyTuple_GetItem(p_Args, i); //���ص�ָ���ǽ�������
                            if (nullptr == objItem)
                            {
                                return nullptr;
                            }

                            std::string str;
                            if (!ObjectToString(str, objItem))
                            {
                                PyErr_SetString(PyExc_TypeError, vcl4c::string::Format("�޷�ʶ�����[%d]����������", i).c_str());
                                return nullptr;
                            }

                            vParams.push_back(CComVariant(str.c_str()));
                        }

                        //2.0 ����IDispatch����
                        HRESULT hRes = S_OK;
                        CFunctionForPython *obj = reinterpret_cast<CFunctionForPython *>(p_Self);
                        CComVariant vResult;
                        if (vParams.empty())
                        {
                            DISPPARAMS dpParams = {nullptr, nullptr, 0, 0};
                            hRes = obj->m_itfObject->Invoke(obj->m_didDispid,
                                IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                                &dpParams, &vResult, nullptr, nullptr);
                        }
                        else
                        {
                            std::vector<CComVariant> vRevertParams(vParams.rbegin(), vParams.rend());

                            DISPPARAMS dpParams = {&vRevertParams[0], nullptr, vRevertParams.size(), 0};
                            hRes = obj->m_itfObject->Invoke(obj->m_didDispid,
                                IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD,
                                &dpParams, &vResult, nullptr, nullptr);
                        }

                        if (FAILED(hRes))
                        {
                            PyErr_SetString(PyExc_RuntimeError, "��������ʧ��");
                            return nullptr;
                        }

                        return VariantToPython(vResult);
                    }

                };

            private:
                //���ߺ�������
                static PyObject *PyDispatch_FromDispatch(IDispatch *p_itfValue)
                {
                    CDispatchForPython *dfpResult = reinterpret_cast<CDispatchForPython *>(
                        m_toDispatch.tp_new(&m_toDispatch, nullptr, nullptr));
                    if (nullptr == dfpResult)
                    {
                        return nullptr;
                    }

                    dfpResult->m_itfObject = p_itfValue;
                    dfpResult->m_itfObject->AddRef();

                    return reinterpret_cast<PyObject *>(dfpResult);
                }

                static PyObject *PyFunction_FromDispatch(IDispatch *p_itfValue, const char *p_sFunctionName)
                {
                    DISPID did = 0;
                    HRESULT hRes = vcl4c::itf::CDispatchHelper::GetIDsOfNames(did, p_itfValue, p_sFunctionName);
                    if (FAILED(hRes))
                    {
                        return nullptr;
                    }

                    CFunctionForPython *ffpResult = reinterpret_cast<CFunctionForPython *>(
                        m_toFunction.tp_new(&m_toFunction, nullptr, nullptr));
                    if (nullptr == ffpResult)
                    {
                        return nullptr;
                    }

                    ffpResult->m_itfObject = p_itfValue;
                    ffpResult->m_itfObject->AddRef();
                    ffpResult->m_didDispid = did;

                    return reinterpret_cast<PyObject *>(ffpResult);
                }

                static PyObject *VariantToPython(VARIANT &p_vValue)
                {
                    switch (p_vValue.vt)
                    {
                    case VT_BOOL:
                        return PyBool_FromLong(ATL_VARIANT_TRUE == p_vValue.boolVal);
                    case VT_I1:
                        return PyInt_FromLong(p_vValue.cVal);
                    case VT_UI1:
                        return PyInt_FromLong(p_vValue.bVal);
                    case VT_I2:
                        return PyInt_FromLong(p_vValue.iVal);
                    case VT_UI2:
                        return PyInt_FromLong(p_vValue.uiVal);
                    case VT_I4:
                    case VT_INT:
                        return PyInt_FromLong(p_vValue.intVal);
                    case VT_UI4:
                    case VT_UINT:
                        return PyInt_FromLong(p_vValue.uintVal);
                    case VT_I8:
                        return PyLong_FromLongLong(p_vValue.llVal);
                    case VT_UI8:
                        return PyLong_FromUnsignedLongLong(p_vValue.ullVal);
                    case VT_R4:
                        return PyFloat_FromDouble(p_vValue.fltVal);
                    case VT_R8:
                        return PyFloat_FromDouble(p_vValue.dblVal);
                    case VT_BSTR:
                        return PyUnicode_FromWideChar(p_vValue.bstrVal, ::SysStringByteLen(p_vValue.bstrVal));
                    case VT_UNKNOWN:
                        //��IUnknownת��ΪIDispatch
                        if (FAILED(::VariantChangeType(&p_vValue, nullptr, 0, VT_DISPATCH)))
                        {
                            return nullptr;
                        }

                        //����ִ��VT_DISPATCH���߼�
                    case VT_DISPATCH:
                        return PyDispatch_FromDispatch(p_vValue.pdispVal);
                    default:
                        return nullptr;
                    }
                }

                static bool PyDispatch_Check(const PyObject *p_Value)
                {
                    //Check�ຯ��ͨ���Ժ�ķ�ʽʵ��
                    return PyObject_TypeCheck(p_Value, &m_toDispatch);
                }

                struct CTypeAttrDeleter
                {
                    CTypeAttrDeleter(ITypeInfo *p_TypeInfo)
                        : m_TypeInfo(p_TypeInfo)
                    {
                    }

                    void operator()(TYPEATTR *p_TypeAttr)
                    {
                        m_TypeInfo->ReleaseTypeAttr(p_TypeAttr);
                    }

                private:
                    ITypeInfo *m_TypeInfo;

                };

                struct CFuncDescDeleter
                {
                    CFuncDescDeleter(ITypeInfo *p_TypeInfo)
                        : m_TypeInfo(p_TypeInfo)
                    {
                    }

                    void operator()(FUNCDESC *p_FuncDesc)
                    {
                        m_TypeInfo->ReleaseFuncDesc(p_FuncDesc);
                    }

                private:
                    ITypeInfo *m_TypeInfo;

                };

            };

            static void PrintLastError(void)
            {
                //PySys_GetObject���صĶ����ǡ�Borrowed reference��
                PyObject *objType = PySys_GetObject("last_type"),
                    *objValue = PySys_GetObject("last_value"),
                    *objTraceback = PySys_GetObject("last_traceback");

                std::vector<std::string> vLog;

                //����쳣����Ϣ
                vLog.push_back("����ִ�г����쳣��");
                char *strModuleName = nullptr, *strClassName = nullptr, *strValue = nullptr;
                if (PyExceptionClass_Check(objType))
                {
                    CAuto objModule(PyObject_GetAttrString(objType, "__module__"));
                    if (nullptr != objModule)
                    {
                        strModuleName = PyString_AsString(objModule);
                    }

                    strClassName = PyExceptionClass_Name(objType);
                }

                if (Py_None != objValue)
                {
                    CAuto strValueString(PyObject_Str(objValue));
                    strValue = PyString_AsString(strValueString);
                }

                vLog.push_back(vcl4c::string::Format("    %s|%s|%s", strModuleName, strClassName, strValue));

                //�������ջ����Ϣ(�ο�PyTraceBack_Print��ʵ��)
                if (nullptr != objTraceback && PyTraceBack_Check(objTraceback))
                {
                    PyTracebackObject *toBegin = reinterpret_cast<PyTracebackObject *>(objTraceback);

                    //PyTracebackObject�Ǹ�����ṹ���䰴�Ӹߵ��͵�˳���¼��Ϊ��ֹ����ѭ�����µ���ջ����������ӡ100����������
                    vLog.push_back("    ����ջ��Ϣ��");
                    for (int iCount = 0; iCount < 100 && nullptr != toBegin; ++iCount)
                    {
                        vLog.push_back(vcl4c::string::Format("    %s|%d|%s",
                            PyString_AsString(toBegin->tb_frame->f_code->co_filename), //Դ���ļ���
                            toBegin->tb_lineno, //�к�
                            PyString_AsString(toBegin->tb_frame->f_code->co_name))); //������
                        toBegin = toBegin->tb_next;
                    }
                }

                vcl4c::itf::CManager::PrintLog(vLog);
            }

            bool ReadFile(::std::string &p_sResult, const wchar_t *p_sFileName) const
            {
                p_sResult.clear();

                //��Cache�ж�ȡ(�ļ��޸ĺ�δ����)
                CComVariant vResult;
                CComPtr<IUnknown> itfCode;
                VARIANT_BOOL bCached = VARIANT_FALSE;
                HRESULT hRes = vcl4c::itf::CDispatchHelper::DispatchInvoke(m_itfContainer,
                    "GetCacheFile", DISPATCH_METHOD, &vResult, "Si*b*", p_sFileName, &itfCode, &bCached);
                if (FAILED(hRes))
                {
                    return false;
                }

                if (VARIANT_TRUE == bCached)
                {
                    CComQIPtr<IMemoryBlock> itfMem = itfCode;
                    if (!itfMem)
                    {
                        return false;
                    }

                    ULONG iLen = 0;
                    hRes = itfMem->GetSize(&iLen);
                    if (FAILED(hRes))
                    {
                        return false;
                    }

                    p_sResult.resize(iLen);
                    hRes = itfMem->Read(0, reinterpret_cast<BYTE *>(const_cast<char *>(p_sResult.c_str())), &iLen);
                    if (FAILED(hRes))
                    {
                        return false;
                    }

                    return true;
                }

                //���ļ���ȡ
                ::std::string sFileName;
                hRes = vcl4c::itf::W2String(sFileName, p_sFileName);
                if (FAILED(hRes))
                {
                    return false;
                }

                if (!vcl4c::file::Read(p_sResult, sFileName))
                {
                    return false;
                }

                return true;
            }
            
        };

        __declspec(selectany) char CPython::CEtsForPython::m_sModuleName[] = "ets";

        __declspec(selectany) PyMethodDef CPython::CEtsForPython::m_mdEntry[] = {
            {"GetFrame",CPython::CEtsForPython::GetFrame,   METH_VARARGS, "GetFrame(str)\n��ȡ���ڿؼ�"},
            {"Show",    CPython::CEtsForPython::Show,       METH_VARARGS, "Show(str)\n�����Ϣ����־����"},
            {"Stop",    CPython::CEtsForPython::Stop,       METH_VARARGS, "Stop()\n����û��Ƿ���ֹ��ǰ����"},
            {nullptr, nullptr}
        };

        __declspec(selectany) _inittab CPython::m_itExtendModule[] = {
            {CPython::CEtsForPython::m_sModuleName, CPython::CEtsForPython::Entry},
            {nullptr, nullptr}
        };

        __declspec(selectany) PyTypeObject CPython::CEtsForPython::m_toDispatch = {
            PyObject_HEAD_INIT(NULL)
            0,                          /* ob_size */
            "ets.Dispatch",             /* tp_name */
            sizeof(CDispatchForPython), /* tp_basicsize */
            0,                          /* tp_itemsize */
            CDispatchForPython::DeAlloc,/* tp_dealloc */
            0,                          /* tp_print */
            CDispatchForPython::GetAttr,/* tp_getattr */
            CDispatchForPython::SetAttr,/* tp_setattr */
            0,                          /* tp_compare */
            0,                          /* tp_repr */
            0,                          /* tp_as_number */
            0,                          /* tp_as_sequence */
            0,                          /* tp_as_mapping */
            0,                          /* tp_hash */
            0,                          /* tp_call */
            0,                          /* tp_str */
            0,                          /* tp_getattro */
            0,                          /* tp_setattro */
            0,                          /* tp_as_buffer */
            Py_TPFLAGS_DEFAULT,         /* tp_flags */
            "Dispatch��װ��\n"          /* tp_doc */
            "\n"
            "��IDispatch�ӿڵĶ��ΰ�װ�����ڽ�ETS�еĹ��ܵ��뵽Python��ʹ��\n"
            "ets.Dispatch(p_sFileName)\n"
            "p_sFileName - ģ�������ײ�ͨ������'vcl4c::itf::CManager::GetManager()->GetPlugins'������Ӧ��ģ��",
            0,                          /* tp_traverse */
            0,                          /* tp_clear */
            0,                          /* tp_richcompare */
            0,                          /* tp_weaklistoffset */
            0,                          /* tp_iter */
            0,                          /* tp_iternext */
            0,                          /* tp_methods */
            0,                          /* tp_members */
            0,                          /* tp_getset */
            0,                          /* tp_base */
            0,                          /* tp_dict */
            0,                          /* tp_descr_get */
            0,                          /* tp_descr_set */
            0,                          /* tp_dictoffset */
            CDispatchForPython::Init,   /* tp_init */
            0,                          /* tp_alloc */
            PyType_GenericNew,          /* tp_new */
            0,                          /* tp_free */
        };
        __declspec(selectany) PyTypeObject CPython::CEtsForPython::m_toFunction = CPython::CEtsForPython::m_toDispatch;

        __declspec(selectany) long CPython::m_iInstanceCount = 0;
        __declspec(selectany) CComPtr<IDispatch> CPython::m_itfContainer;
        __declspec(selectany) CComPtr<IDispatch> CPython::m_itfFrame;

    }
}
