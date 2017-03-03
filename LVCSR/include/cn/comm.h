
#ifndef __CN_COMMON_MODULE__
#define __CN_COMMON_MODULE__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>

#ifndef UNIX
#include <conio.h>
#include <windows.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#include <time.h>
#endif
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <vector>
#ifndef UNIX
#include <hash_map>
#include <hash_set>
#else
#include <ext/hash_map>
#include <ext/hash_set>
using __gnu_cxx::hash_map;
using __gnu_cxx::hash_multimap;
#include <ext/hash_fun.h>
using __gnu_cxx::hash;
#endif
#include <algorithm>

using namespace std;

//////////////////////////////////////////////////////////////////////////
//--- type definition

#ifndef UNIX
typedef __int64	INT64;
#else
typedef long long	INT64;
#endif
typedef unsigned short	ushort;
typedef unsigned int	uint;
typedef unsigned char	uchar;

typedef float	Time;

typedef uint	OffPtr;

typedef ushort	HypoID;
typedef uint	SliceID;
typedef uint	SegID;
typedef uint	DocID;

typedef uint	WordID; 
typedef uint	HitID;

typedef double	LnProb;
//typedef float	LnProb;
typedef string	Word;

#define INVALID_ID    255
const	WordID  INVALID_WORDID = 65535;//1-24-2010, JGao
const	HypoID	INVALID_HYPOID = (HypoID)65356;
const	LnProb	UNDEF_LN	 = -HUGE_VAL;//undefine lnprob
const   int		UNDEF_NUM	 = -9999;	//undefine postive number







typedef vector<Word>	WordVector;
typedef vector<double>            DblVector;//double vector
typedef vector<DblVector>         DblMatrix;//double matrix
typedef vector<int>				  IntVector;

typedef vector<WordID>            WIDVector;//wordID vector
typedef vector<WIDVector>         WIDMatrix;//wordID matrix


//string hashmap
struct equal_str
{
    bool operator()(const string& s1, const string& s2) const
	{
		return s1 == s2;
	}
};

#if defined __GNUC__
struct str_hash
{
	size_t operator()(const string& str) const
	{
		hash<const char*> hchar;
		return hchar(str.c_str());
		/* unsigned long __h = 0;
		for (size_t i = 0 ; i < str.size() ; i ++)
		__h = 5*__h + str[i];
		return size_t(__h);*/
	}
};

/*typedef hash_map<const char*, WordID, str_hash, equal_str> WordIntHMap;
typedef hash_map<const char*, string, str_hash, equal_str> WordWordHMap;*/
typedef hash_map<string, WordID, str_hash, equal_str> WordIntHMap;
typedef hash_map<string, string, str_hash, equal_str> WordWordHMap;
#else
typedef hash_map<string, WordID, hash<string>, equal_str> WordIntHMap;
typedef hash_map<string, string, hash<string>, equal_str> WordWordHMap;
#endif

//////////////////////////////////////////////////////////////////////////
//--- const definition
const Word      START_WD               = "!SENTSTART";
const Word      END_WD                 = "!SENTEND";
const Word      EPS                    = "!NULL";
const Word      START_WD_TIT           = "<enter>";
const Word      END_WD_TIT             = "<exit>";

const WordID    MAX_NO_WORDS           = 512;  // in a sentence
const WordID    MAX_NO_WORDS_LAT       = 5000; // in a lattice

const unsigned  MAX_LINE               = 1000;   // max no of chars on a line
const float     MAX_TIME               = 1000;   // max end time hypothesized for a word (secs)

const uint  MAX_VOC_SIZE           = 200000; // max no of words in the vocabulary
const uint  MAX_NO_FIELDS_NODE     = 6;
const uint  MIN_NO_FIELDS_NODE     = 1; 

const uint	DEFAULT_NO_PRONS			= 2;
const string    BLANK                   = " \t";

const uint SAMPLE_RATE = 8000;
const float FRAME_TIME = 0.010;	//s


//////////////////////////////////////////////////////////////////////////
#define NOTUNE
#define MAX_SUBWOR_NUM 64
#define TIPHONENAMELEN		32 //The max length of the tri-phone name
#define INIT_TRACE_NUM		16 //Backtrace information in the search procedure
//#define MAX_TRIPHONE_NUM	10 //The max tri-phone number in a hypo
//  [2/22/2011 JGao]
#define MAX_TRIPHONE_NUM	64 //The max tri-phone number in a hypo
#define LINEAR_MED_SERACH 1
#define  BACK_TRACE_ALL 1

//#define USE_ENG 1// 共3处，第2处


//////////////////////////////////////////////////////////////////////////
//--- assert definition 
#ifndef UNIX
#define ASSERT2(CND,MSG) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ );exit(3);}
#define ASSERT3(CND,MSG,PTM) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM);exit(3);}
#define ASSERT4(CND,MSG,PTM,PTM1) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM,##PTM1);exit(3);}
#define ASSERT5(CND,MSG,PTM,PTM1,PTM2) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM,##PTM1,##PTM2);exit(3);}
#define ASSERT6(CND,MSG,PTM,PTM1,PTM2,PTM3) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM,##PTM1,##PTM2,##PTM3);exit(3);}
//--- warning defintion (no sound)
#define WARNING1(MSG) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ )
#define WARNING2(MSG,PTM) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM)
#define WARNING3(MSG,PTM,PTM1) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM,##PTM1)
#define WARNING4(MSG,PTM,PTM1,PTM2) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM,##PTM1,##PTM2)
#define WARNING5(MSG,PTM,PTM1,PTM2,PTM3) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM,##PTM1,##PTM2,##PTM3)
#else
#define ASSERT2(CND,MSG) assert(CND);
#define ASSERT3(CND,MSG,PTM) assert(CND);
#define ASSERT4(CND,MSG,PTM,PTM1) assert(CND);
#define ASSERT5(CND,MSG,PTM,PTM1,PTM2) assert(CND);
#define ASSERT6(CND,MSG,PTM,PTM1,PTM2,PTM3) assert(CND);
/*#ifdef NO_SOUND*/
#define WARNING1(MSG) 
#define WARNING2(MSG,PTM)
#define WARNING3(MSG,PTM,PTM1) 
#define WARNING4(MSG,PTM,PTM1,PTM2)
#define WARNING5(MSG,PTM,PTM1,PTM2,PTM3) 
#endif



//////////////////////////////////////////////////////////////////////////
//---definition
// lib or dll selection 
#ifndef _ISDTLIB
#define _ISDTLIB
#ifdef _ISDTLIB
#define ISDTAPI 
#else
#define ISDTAPI __declspec(dllexport)
#endif
#endif

//////////////////////////////////////////////////////////////////////////
//--- IO operation
#define ASSERT(CND,MSG,PTM) if (!(CND)) {fprintf(stderr, \
        "(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ );exit(3);}

#define ReadOpen(X,Y) ASSERT3(X=fopen(Y,"rb"),"Cannot open %s",Y)
//#define WriteOpen(X,Y) ASSERT3(X=fopen(Y,"wb"),"Cannot open %s",Y)
#define AppendOpen(X,Y) ASSERT3(X=fopen(Y,"ab"),"Cannot open %s",Y)
#define TextReadOpen(X,Y) ASSERT3(X=fopen(Y,"rt"),"Cannot open %s",Y)
//#define TextWriteOpen(X,Y) ASSERT3(X=fopen(Y,"wt"),"Cannot open %s",Y)
#define TextAppendOpen(X,Y) ASSERT3(X=fopen(Y,"at"),"Cannot open %s",Y)

#if !defined (INVALID_FILE_ATTRIBUTES) 
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1) 
#endif




#define STRINGLENGTH 256
typedef char AString[STRINGLENGTH];			//array string


//--- function
#define MAX_STRLEN	2048
#define INIT_NDOCSET	100
#define	INIT_NDOC	1
#define INIT_NSEG	(INIT_NDOC*256)
#define INIT_NSLICE	(INIT_NSEG * 16)
#define INIT_NHYPO (INIT_NSLICE *8)	


//------------------Function-------------------
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#define min(a,b)  (((a) < (b)) ? (a) : (b))

#define ALIGN_32(l) (((l)+31)&(~31))
#define ALIGN_4F(l) (((l)+3)&(~3))

int getlineH(istream& istr, string& line);
int myfgets( FILE *fp, int count, char *line);
//string SkipTune(string strTune);
string SkipTune(string strTune,char &tonename);//  [3/8/2011 zhzhang]
LnProb Ln(double prob);


inline LnProb AddLnP(LnProb x, LnProb y)
{
    if (x < y) 
	{
		LnProb temp = x; x = y; y = temp;
    }
	
    if (y == -HUGE_VAL) 
	{
		return x;
    } 
	else 
	{
		LnProb diff = y - x;
		return x + log(1.0 + exp(diff));
    }
}


#endif






