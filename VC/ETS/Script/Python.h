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
            CComPtr<IDispatch> m_itfFrame;

            /** Python��չģ��ETS��ʵ����
            */
            class CEtsForPython
            {
            public:
                static char m_sModuleName[];

                static void Entry(void)
                {
                    PyObject *objModule = Py_InitModule3(m_sModuleName, m_mdEntry, "ETS�ӿ�ģ�飬������Python�з���ETS�Ĺ���");
                    if (nullptr == objModule)
                    {
                        return;
                    }
                }

            private:
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
            {"Show",   CPython::CEtsForPython::Show,  METH_VARARGS, "Show(str)\n�����Ϣ����־����"},
            {"Stop",   CPython::CEtsForPython::Stop,  METH_VARARGS, "Stop()\n����û��Ƿ���ֹ��ǰ����"},
            {nullptr, nullptr}
        };

        __declspec(selectany) _inittab CPython::m_itExtendModule[] = {
            {CPython::CEtsForPython::m_sModuleName, CPython::CEtsForPython::Entry},
            {nullptr, nullptr}
        };

        __declspec(selectany) long CPython::m_iInstanceCount = 0;
        __declspec(selectany) CComPtr<IDispatch> CPython::m_itfContainer;

    }
}
