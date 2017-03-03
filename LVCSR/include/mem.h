/*****************************************************************************/
/*M*
//                        THINKIT INTERNATIONAL PROPRIETARY INFORMATION
//        This software is supplied under the terms of the license agreement
//		or nondisclosure agreement with Thinkit International and may not be copied
//		or disclosed except in accordance with the terms of that agreement.
//            Copyright (c) 2002 ~ 2008 Thinkit International. All Rights Reserved.
//     VSS:
//     $Workfile:: mem.h                              $
//     $Author:: Savely                               $
//     $Revision:: 4                                  $
//     $Modtime:: 12/28/00 1:49p                      $
//     $NoKeywords: $
//
//
//    Memory Management Head File
//	      
M*/
			  
#ifndef aaaMEMORY_MANAGEMENTaaa
#define aaaMEMORY_MANAGEMENTaaa
#include "isdtexception.h" 
ISDTAPI void *Malloc(int sz,const bool clear=false) throw(BadAlloc);
ISDTAPI void *Malloc(int n1,int itsz,const bool clear=false) throw(BadAlloc);
ISDTAPI void *Malloc32(int sz,const bool clear=false) throw(BadAlloc);
ISDTAPI void *Malloc32(int n1,int itsz,const bool clear=false) throw(BadAlloc);
ISDTAPI void **Malloc(int n1,int n2,int itsz,const bool clear=false) throw(BadAlloc);
ISDTAPI void ***Malloc(int n1,int n2,int n3,int itsz,const bool clear=false) throw(BadAlloc);
ISDTAPI void Free(void *ptr) throw(); 
ISDTAPI void Free32(void *ptr) throw();
ISDTAPI void Free(void **ptr,int n1) throw();
ISDTAPI void Free(void ***ptr,int n1,int n2) throw();

#endif
