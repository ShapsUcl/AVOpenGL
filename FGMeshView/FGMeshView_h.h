

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Wed Sep 09 18:18:00 2015
 */
/* Compiler settings for FGMeshView.idl:
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


#ifndef __FGMeshView_h_h__
#define __FGMeshView_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IFGMeshView_FWD_DEFINED__
#define __IFGMeshView_FWD_DEFINED__
typedef interface IFGMeshView IFGMeshView;
#endif 	/* __IFGMeshView_FWD_DEFINED__ */


#ifndef __CFGMeshViewDoc_FWD_DEFINED__
#define __CFGMeshViewDoc_FWD_DEFINED__

#ifdef __cplusplus
typedef class CFGMeshViewDoc CFGMeshViewDoc;
#else
typedef struct CFGMeshViewDoc CFGMeshViewDoc;
#endif /* __cplusplus */

#endif 	/* __CFGMeshViewDoc_FWD_DEFINED__ */


#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __FGMeshView_LIBRARY_DEFINED__
#define __FGMeshView_LIBRARY_DEFINED__

/* library FGMeshView */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_FGMeshView;

#ifndef __IFGMeshView_DISPINTERFACE_DEFINED__
#define __IFGMeshView_DISPINTERFACE_DEFINED__

/* dispinterface IFGMeshView */
/* [uuid] */ 


EXTERN_C const IID DIID_IFGMeshView;

#if defined(__cplusplus) && !defined(CINTERFACE)

    MIDL_INTERFACE("A2C35A88-6AFB-4B85-99A8-2195E79AB8B8")
    IFGMeshView : public IDispatch
    {
    };
    
#else 	/* C style interface */

    typedef struct IFGMeshViewVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IFGMeshView * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IFGMeshView * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IFGMeshView * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IFGMeshView * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IFGMeshView * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IFGMeshView * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [range][in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IFGMeshView * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IFGMeshViewVtbl;

    interface IFGMeshView
    {
        CONST_VTBL struct IFGMeshViewVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IFGMeshView_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IFGMeshView_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IFGMeshView_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IFGMeshView_GetTypeInfoCount(This,pctinfo)	\
    ( (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo) ) 

#define IFGMeshView_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    ( (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo) ) 

#define IFGMeshView_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    ( (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId) ) 

#define IFGMeshView_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    ( (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */


#endif 	/* __IFGMeshView_DISPINTERFACE_DEFINED__ */


EXTERN_C const CLSID CLSID_CFGMeshViewDoc;

#ifdef __cplusplus

class DECLSPEC_UUID("12135306-B61D-4743-85BD-06BDBEC697C5")
CFGMeshViewDoc;
#endif
#endif /* __FGMeshView_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


