

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Tue Jun 16 10:51:06 2020
 */
/* Compiler settings for E:\MyWork\src\VC\Tools\IDL\ETS.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __ETS_h__
#define __ETS_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IMemoryBlock_FWD_DEFINED__
#define __IMemoryBlock_FWD_DEFINED__
typedef interface IMemoryBlock IMemoryBlock;
#endif 	/* __IMemoryBlock_FWD_DEFINED__ */


#ifndef __IManager_FWD_DEFINED__
#define __IManager_FWD_DEFINED__
typedef interface IManager IManager;
#endif 	/* __IManager_FWD_DEFINED__ */


#ifndef __IMemoryBlock_FWD_DEFINED__
#define __IMemoryBlock_FWD_DEFINED__
typedef interface IMemoryBlock IMemoryBlock;
#endif 	/* __IMemoryBlock_FWD_DEFINED__ */


#ifndef __IManager_FWD_DEFINED__
#define __IManager_FWD_DEFINED__
typedef interface IManager IManager;
#endif 	/* __IManager_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_ETS_0000_0000 */
/* [local] */ 


enum EEncodingType
    {	etBinary	= 0,
	etGbk	= ( etBinary + 1 ) ,
	etUtf8	= ( etGbk + 1 ) ,
	etUnicode	= ( etUtf8 + 1 ) 
    } ;


extern RPC_IF_HANDLE __MIDL_itf_ETS_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_ETS_0000_0000_v0_0_s_ifspec;

#ifndef __IMemoryBlock_INTERFACE_DEFINED__
#define __IMemoryBlock_INTERFACE_DEFINED__

/* interface IMemoryBlock */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IMemoryBlock;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A186C64B-DA6F-438E-A4B8-A485F7C7573C")
    IMemoryBlock : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetEncoding( 
            /* [retval][out] */ BYTE *p_iResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetEncoding( 
            /* [in] */ BYTE p_iValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetSize( 
            /* [retval][out] */ ULONG *p_iResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetSize( 
            /* [in] */ ULONG p_iValue) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Read( 
            /* [in] */ ULONG p_iPosition,
            /* [in] */ BYTE *p_pValue,
            /* [out][in] */ ULONG *p_piLength) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Write( 
            /* [in] */ ULONG p_iPosition,
            /* [in] */ BYTE *p_pValue,
            /* [in] */ ULONG p_iLength) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IMemoryBlockVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMemoryBlock * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMemoryBlock * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMemoryBlock * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetEncoding )( 
            IMemoryBlock * This,
            /* [retval][out] */ BYTE *p_iResult);
        
        HRESULT ( STDMETHODCALLTYPE *SetEncoding )( 
            IMemoryBlock * This,
            /* [in] */ BYTE p_iValue);
        
        HRESULT ( STDMETHODCALLTYPE *GetSize )( 
            IMemoryBlock * This,
            /* [retval][out] */ ULONG *p_iResult);
        
        HRESULT ( STDMETHODCALLTYPE *SetSize )( 
            IMemoryBlock * This,
            /* [in] */ ULONG p_iValue);
        
        HRESULT ( STDMETHODCALLTYPE *Read )( 
            IMemoryBlock * This,
            /* [in] */ ULONG p_iPosition,
            /* [in] */ BYTE *p_pValue,
            /* [out][in] */ ULONG *p_piLength);
        
        HRESULT ( STDMETHODCALLTYPE *Write )( 
            IMemoryBlock * This,
            /* [in] */ ULONG p_iPosition,
            /* [in] */ BYTE *p_pValue,
            /* [in] */ ULONG p_iLength);
        
        END_INTERFACE
    } IMemoryBlockVtbl;

    interface IMemoryBlock
    {
        CONST_VTBL struct IMemoryBlockVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMemoryBlock_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IMemoryBlock_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IMemoryBlock_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IMemoryBlock_GetEncoding(This,p_iResult)	\
    ( (This)->lpVtbl -> GetEncoding(This,p_iResult) ) 

#define IMemoryBlock_SetEncoding(This,p_iValue)	\
    ( (This)->lpVtbl -> SetEncoding(This,p_iValue) ) 

#define IMemoryBlock_GetSize(This,p_iResult)	\
    ( (This)->lpVtbl -> GetSize(This,p_iResult) ) 

#define IMemoryBlock_SetSize(This,p_iValue)	\
    ( (This)->lpVtbl -> SetSize(This,p_iValue) ) 

#define IMemoryBlock_Read(This,p_iPosition,p_pValue,p_piLength)	\
    ( (This)->lpVtbl -> Read(This,p_iPosition,p_pValue,p_piLength) ) 

#define IMemoryBlock_Write(This,p_iPosition,p_pValue,p_iLength)	\
    ( (This)->lpVtbl -> Write(This,p_iPosition,p_pValue,p_iLength) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMemoryBlock_INTERFACE_DEFINED__ */


#ifndef __IManager_INTERFACE_DEFINED__
#define __IManager_INTERFACE_DEFINED__

/* interface IManager */
/* [unique][helpstring][uuid][object][local] */ 


EXTERN_C const IID IID_IManager;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("E43AC553-3020-4A0A-91D4-7CA936DE27E4")
    IManager : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetPlugins( 
            /* [in] */ BSTR p_sFileName,
            /* [retval][out] */ IDispatch **p_itfResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetService( 
            /* [in] */ BSTR p_sServiceName,
            /* [retval][out] */ VARIANT *p_vResult) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE Lock( void) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE UnLock( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IManagerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IManager * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IManager * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetPlugins )( 
            IManager * This,
            /* [in] */ BSTR p_sFileName,
            /* [retval][out] */ IDispatch **p_itfResult);
        
        HRESULT ( STDMETHODCALLTYPE *GetService )( 
            IManager * This,
            /* [in] */ BSTR p_sServiceName,
            /* [retval][out] */ VARIANT *p_vResult);
        
        HRESULT ( STDMETHODCALLTYPE *Lock )( 
            IManager * This);
        
        HRESULT ( STDMETHODCALLTYPE *UnLock )( 
            IManager * This);
        
        END_INTERFACE
    } IManagerVtbl;

    interface IManager
    {
        CONST_VTBL struct IManagerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IManager_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IManager_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IManager_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IManager_GetPlugins(This,p_sFileName,p_itfResult)	\
    ( (This)->lpVtbl -> GetPlugins(This,p_sFileName,p_itfResult) ) 

#define IManager_GetService(This,p_sServiceName,p_vResult)	\
    ( (This)->lpVtbl -> GetService(This,p_sServiceName,p_vResult) ) 

#define IManager_Lock(This)	\
    ( (This)->lpVtbl -> Lock(This) ) 

#define IManager_UnLock(This)	\
    ( (This)->lpVtbl -> UnLock(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IManager_INTERFACE_DEFINED__ */



#ifndef __ETSLib_LIBRARY_DEFINED__
#define __ETSLib_LIBRARY_DEFINED__

/* library ETSLib */
/* [helpstring][version][uuid] */ 




EXTERN_C const IID LIBID_ETSLib;
#endif /* __ETSLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


