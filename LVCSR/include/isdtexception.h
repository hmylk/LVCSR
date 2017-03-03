/*****************************************************************************/
/*M*
//                        THINKIT INTERNATIONAL PROPRIETARY INFORMATION
//        This software is supplied under the terms of the license agreement
//		or nondisclosure agreement with Thinkit International and may not be copied
//		or disclosed except in accordance with the terms of that agreement.
//            Copyright (c) 2002 ~ 2008 Thinkit International. All Rights Reserved.
//     VSS:
//     $Workfile:: isdtexception.h                   $
//     $Author:: Vit                                  $
//     $Revision:: 5                                  $
//     $Modtime:: 11/02/00 10:44a                     $
//     $NoKeywords: $
//
//
//
M*/

#ifndef _ISDT_EXCEPTIONS_
#define _ISDT_EXCEPTIONS_

#include <stdexcept>
#include <string>

#ifdef __GNUC__
// for old C++ compilers now are under unix
#include <strstream>
#else
// C++ Standard
#include <sstream>
#endif

using namespace std;

#define MAX_EMSG_LENGTH 2048 

namespace ISDT
{

template <class T> string& operator<< (string &s, T x)
{ 
  #ifdef __GNUC__
   ostrstream os;    // for g++ compatibility (under unix)
  #else
   ostringstream os; // C++ standard
  #endif
  os << s << x;
  s = os.str();
  return s;
}

class ISDTAPI ISDTException: public std::exception
{
  public:
   ISDTException()                 throw();
   ISDTException(const char *what) throw();
   ISDTException(const string &s)  throw();
   //ISDTException(const ISDTException &eSrc) throw();
   //ISDTException& operator= (const ISDTException&) throw();
   virtual ~ISDTException()        throw();
   virtual const char *what()  const    throw();

  private:
   char *whatIsHappend;
};

class ISDTAPI BadAlloc: public ISDTException
{ public:
   BadAlloc()                   throw();
   BadAlloc(const char *what)   throw();
   BadAlloc(const string &s)    throw();
};

class ISDTAPI BadCreate: public ISDTException
{ public:
   BadCreate()                  throw();
   BadCreate(const char *what)  throw();
   BadCreate(const string &s)   throw();
};

class ISDTAPI RangeError: public ISDTException
{ public:
   RangeError()                 throw();
   RangeError(const char *what) throw();
   RangeError(const string &s)  throw();
};

//-------------------------   F i l e   O p e r a t i o n s  -------------------

class ISDTAPI BadFileOperation: public ISDTException
{ public:
   BadFileOperation()                 throw();
   BadFileOperation(const char *what) throw();
   BadFileOperation(const string &s)  throw();
};

class ISDTAPI BadFileOpen: public BadFileOperation
{ public:
   BadFileOpen()                  throw();
   BadFileOpen(const char *what)  throw();
   BadFileOpen(const string &s)   throw();
};

class ISDTAPI BadFileClose: public BadFileOperation
{ public:
   BadFileClose()                 throw();
   BadFileClose(const char *what) throw();
   BadFileClose(const string &s)  throw();
};

class ISDTAPI BadFileRead: public BadFileOperation
{ public:
   BadFileRead()                  throw();
   BadFileRead(const char *what)  throw();
   BadFileRead(const string &s)   throw();
};

class ISDTAPI BadFileWrite: public BadFileOperation
{ public:
   BadFileWrite()                 throw();
   BadFileWrite(const char *what) throw();
   BadFileWrite(const string &s)  throw();
};

//------------------------- L o g i c  E r r o r s ---------------------------

class ISDTAPI LogicError: public ISDTException
{ public:
   LogicError()                   throw();
   LogicError(const char *what)   throw();
   LogicError(const string &s)    throw();
};


} // namespace ISDT

using namespace ISDT; // for transient period

#endif //_ISDT_EXCEPTIONS_


