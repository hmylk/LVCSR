
#ifndef __COMMON_MODULE__
#define __COMMON_MODULE__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <assert.h>

// add to get struct WordResult etc. [4/9/2014 Administrator]
#include "../LVCSR/LVCSR_API.h"

#define ViterbiSpeedUp
#define Frame_Shift_Len 10  // ms  per frame shift

#ifndef WIN32
#include <pthread.h>
#include <sys/resource.h>
#define stricmp strcasecmp
#define _timeb timeb
#define _ftime ftime
#define CRITICAL_SECTION pthread_mutex_t
#define EnterCriticalSection pthread_mutex_lock
#define LeaveCriticalSection pthread_mutex_unlock
#define InitializeCriticalSection(x) pthread_mutex_init(x,NULL)
#define DeleteCriticalSection pthread_mutex_destroy
typedef unsigned long DWORD;
#define DeleteFile remove
#define _stat stat
#endif

#include <ipps.h>
#include <ippsr.h>

#ifdef WIN32

#include <windows.h>
#include <process.h>
#include <direct.h>
#include <io.h>

#define getuid()	0
#define gethostname(str,len)  DWORD buffer_name_len=len; \
		GetComputerName(str, &buffer_name_len)
#define sleep(n) Sleep(n)
#define PACK_ATTRIBUTE

#else

#include <unistd.h>
#include <stdio.h>

#define _MAX_FNAME FILENAME_MAX
#define  HANDLE int
#define PACK_ATTRIBUTE  __attribute__ ((packed))
#endif

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef uchar mnphn;
typedef __INT64 phnenc;

#if defined __GNUC__
typedef long long __int64;
#endif


#if defined __GNUC__
typedef long long int phnenc;
#else
typedef __int64 phnenc;
#endif

#ifdef _ISDTLIB
#define ISDTAPI 
#else
#define ISDTAPI __declspec(dllexport)
#endif

class Feature;    
class HMM;        
class MonoPhone;  
class PronLattice;
class WDictName;  
class WListName;  
class PLM;   
class TreeBigram;      
class LogFile;    
class MapFile;    
class MngTrace;   
class LattNet;
class MapFile;
class DcsTree;
class DcsTreeQuestion;
class PhoneEncoding;
class aLattNode;
class Memory;
class EmbedStatis;

#ifdef WIN32
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
/*#ifdef NO_SOUND*/
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
#define ASSERT2(CND,MSG) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ );exit(3);}
#define ASSERT3(CND,MSG,PTM) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM);exit(3);}
#define ASSERT4(CND,MSG,PTM,PTM1) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM,#PTM1);exit(3);}
#define ASSERT5(CND,MSG,PTM,PTM1,PTM2) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM,#PTM1,#PTM2);exit(3);}
#define ASSERT6(CND,MSG,PTM,PTM1,PTM2,PTM3) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM,#PTM1,#PTM2,#PTM3);exit(3);}
#define WARNING1(MSG) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ )
#define WARNING2(MSG,PTM) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,#PTM)
#define WARNING3(MSG,PTM,PTM1) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,#PTM,#PTM1)
#define WARNING4(MSG,PTM,PTM1,PTM2) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,#PTM,#PTM1,#PTM2)
#define WARNING5(MSG,PTM,PTM1,PTM2,PTM3) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,#PTM,#PTM1,#PTM2,#PTM3)
#endif

/*#else
#define WARNING1(MSG) fprintf(stderr, \
   "\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ )
#define WARNING2(MSG,PTM) fprintf(stderr, \
   "\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM)
#define WARNING3(MSG,PTM,PTM1) fprintf(stderr, \
   "\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM,##PTM1)
#define WARNING4(MSG,PTM,PTM1,PTM2) fprintf(stderr, \
   "\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM,##PTM1,##PTM2)
#define WARNING5(MSG,PTM,PTM1,PTM2,PTM3) fprintf(stderr, \
   "\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM,##PTM1,##PTM2,##PTM3)
#endif*/
#ifdef __GNUC__
#define CouldBeReadOpen(X)  access(X,R_OK) >= 0
#else
#define CouldBeReadOpen(X)  _access(X,4) >= 0
#define open   _open
#endif
ISDTAPI void createDirs(char *filename);
#define ReadOpen(X,Y) ASSERT3(X=fopen(Y,"rb"),"Cannot open %s",Y)
#define WriteOpen(X,Y) createDirs(Y); ASSERT3(X=fopen(Y,"wb"),"Cannot open %s",Y)
#define AppendOpen(X,Y) ASSERT3(X=fopen(Y,"ab"),"Cannot open %s",Y)
#define TextReadOpen(X,Y) ASSERT3(X=fopen(Y,"rt"),"Cannot open %s",Y)
#define TextWriteOpen(X,Y) createDirs(Y); ASSERT3(X=fopen(Y,"wt"),"Cannot open %s",Y)
#define TextAppendOpen(X,Y) ASSERT3(X=fopen(Y,"at"),"Cannot open %s",Y)


#define MAX(a,b)  (((a) > (b)) ? (a) : (b))
#define MIN(a,b)  (((a) < (b)) ? (a) : (b))


#define STRINGLENGTH 256
typedef char char256[STRINGLENGTH];

#define PROBTYPE float
#define ACCPROBTYPE float

#define __KWS_PLPP__		1  
#define __KWS_MUL_MODE__	1
#define BACK_TRACE_ALL		1
#define	TRACE_CN_FEAT		1		//trace the feature of the cn files!
#define	TRACE_CN_TRIPHONE	1



#define DELETE_SCORE  -1000000
#define INVALID_TIME  -10
#define INVALID_ID    255





#define Epsilon              1.0e-5
#define Min_Mix_Weight  1.0e-5    //  log(1.0e-5) = MINMIX  in logmath.h


#define PhnNumBits 8
#define PhnNumLmt  (1<<PhnNumBits)
#define MonoMask   PhnNumLmt-1

#define ALIGN_32(l) (((l)+31)&(~31))
#define ALIGN_4F(l) (((l)+3)&(~3))

typedef enum
{
	TIT_SUCCESS=0,
	TIT_ERROR_BUSY=-100,
	TIT_ERROR_INVALID_HANDLE,
	TIT_ERROR_INVALID_DATA,
	TIT_ERROR_INVALID_POINTER,
	TIT_ERROR_INVALID_LANGUAGE,
	TIT_ERROR_INVALID_NBEST_INDEX,
	TIT_ERROR_NO_CONTEXT_DEFINED,
	TIT_ERROR_WRONG_STATE,
	TIT_ERROR_BUFFER_OVERFLOW,
	TIT_ERROR_NODATA,
	TIT_ERROR_SYNTAX,
	TIT_ERROR_NOMEM,
	TIT_ERROR_NOTFOUND,
	TIT_ERROR_OPEN_FILE,
	TIT_ERROR_NOT_SUPPORTED,
	TIT_ERROR_BUF_TOOSMALL,
	TIT_ERROR_OUT_OF_LIMIT,
	TIT_ERROR_SYS_NOT_STARTED,
	TIT_ERROR_NO_LICENSE,
	TIT_ERROR_NOT_INITIALIZED,
	TIT_ERROR_NOT_STARTED,
	TIT_ERROR_TOOMANY_DICT, //there are too many pron dictionaries in one Lexicon class
	TIT_ERROR_TOOMANY_LOW_LEVEL_DICT, //there are too many pron dictionaries whose level is lower than user defined
	TIT_ERROR_NOTFIND_DICT, //not find matched pron dict in lexicon
	TIT_ERROR_MONOPHONE_SET_NOTMATCH, //TEE,SILENCE mono phone index mismatch among all PDBs
	TIT_ERROR_UNKNOWN_MONOPHONE, //unknown mono phone
	TIT_ERROR_PRONS_TOOLONG, //one word's mono number can't exceed MAX_PRONS_PER_WORD!!
	TIT_ERROR_LABLEWORD, //can't lable input word's prons correctly
	TIT_ERROR_NO_LEXICON, //no lexicon in the grammar class
	TIT_ERROR_NO_SYSCONFIGFILE, //system configuration file not found
	TIT_ERROR_LOADDICT_ERROR,//Load dictionary into lexicon error
	TIT_ERROR_LOADLEXICON_ERROR, //load lexicon fail due to not an empty lexion!!
	TIT_ERROR_DECODER_INITIAL,	//initial decoder fail
	TIT_ERROR_INIT_HMM,			//initial HMM model for decoder fail
	TIT_ERROR_INIT_FEATURE,		//initial feat for decoder fail
	TIT_ERROR_REINIT_DECODER,	//reinitial decoder fail
	TIT_ERROR_INVALID_CODINGFORMAT,	// incorrect or unsupported speech coding format 
	TIT_ERROR_INVALID_NETFILE,
	TIT_ERROR_DUPLICATENODE,
	TIT_ERROR_ADDNODE,
	TIT_ERROR_CREATENODE,
	TIT_ERROR_NO_PARSE_RESULT,
	TIT_ERROR_NOACTIVEGRAMMAR,		//not specific active grammar for decoder
	TIT_ERROR_NO_TAG_DEFINED,       // No tag defined
    TIT_ERROR_NO_RESULT,
	TIT_ERROR_DECODER_TERMINATED,
	TIT_ERROR_DECODER_INVALID_PRONS,
	TIT_ERROR_SPEECHTOOLONG,
	TIT_ERROR_STATELATTICENULL,
	TIT_ERROR_POST_MESSAGE,
	TIT_ERROR_DETECT,
	TIT_ERROR_TIMEOUT,
	TIT_ERROR_UNKNOWN,
} TITCODE;

typedef struct _aLexTreeHeader {
      // pronunciation lattice information
	  ushort bGramRestrict;  
      ushort silStartStateIdx; 
      ushort teeStateIdx; 
      int     wordSetNum; 
      int     wordNum; 
      int	stateIdxNum; 
      int 	nMaxWordMonoNum; 
      int	  nodeNum;
	  int	  edgeNum;
      int     stateNodeNum; 
      int     stateEdgeNum; 
      int 	spNodeNum; 
      int	spEdgeNum; 
      int 	weNodeNum; 
      int 	weEdgeNum; 
	  int 	maxHomoWordNum; 
      int  weStateNum; 
      int  weStateFanOuSum; 
	  int     maxchildren; 
	  int layer1stNodeNum;

} aLexTreeHeader;

typedef enum _TSR_STREAM_FLAG
{
    TSR_STREAM_START=0,
    TSR_STREAM_CONT,
    TSR_STREAM_END,
    TSR_STREAM_ALL
}TSR_STREAM_FLAG;



/*
enum SOUND_TYPE
{	// 语音参考分类
    MALE=0, //男
    FEMALE, //女
    MUSIC, //音乐
    NOISE, //噪声
	UNK
};*/

enum EngineType {
    ENGINE_ONEPASS = 0,
    ENGINE_WFST,
    ENGINE_UNKNOW
};


//--------------------lattice structure interface---------------------
#define ENHANCED_RESULT		1
struct NodeCand
{
    short nodeId; 
    float time;    
    short pronId;
    char text[32];   
    NodeCand (){
        nodeId = 0;
        pronId=0;
        time = 0;
        text[0] = '\0';
    };
};

struct LinkCand
{
    short linkId;
    int sNode;
    int eNode;
    float amProb;
    float lmProb;
    char text[32];
    LinkCand (){
        linkId = 0;
        sNode=eNode=0;
        amProb = lmProb=0;
        text[0] = '\0';
    };
};

struct LatticeResult
{
    float lmscale;
    float wdpenalty;
    float acscale;
    float base;
    int nodeNum; 
    int linkNum;
    float version;
    NodeCand *nCands;
    LinkCand *lCands;
    LatticeResult(){
        lmscale = wdpenalty=acscale=base=0;
        nodeNum = linkNum=0;
        version = 1.0;
        nCands = NULL;
        lCands = NULL;
    }
    ~LatticeResult(){
        if (nCands) delete []nCands;
        if (lCands) delete []lCands;
    }
};


/*
struct ACand
{
    char *text;   // 候选文本结果
    float score;   // 候选得分，0.00~100.00，candNum个
    float startTime;   // 开始时刻，单位为秒s
    float endTime;   // 结束时刻，单位为秒s
    ACand *next;   // 到下一个的指针
#ifdef ENHANCED_RESULT
    char * phone;//phoneme string
    char * segTime;//segTime string
#endif
    ACand (){
        text = 0;
        next = 0;
        score = startTime = endTime = 0;
#ifdef ENHANCED_RESULT
        phone = 0;
        segTime = 0;
#endif
    };
};

struct WordResult
{
    int   candNum; 
    ACand *cands;
    WordResult *next;
    WordResult(){
        candNum = 0;
        cands = 0;
        next = 0;
    }
};
*/



//#define _MMIE_DECODING_
//#define _1stPASS_BIGRAM_
#include "options.h"
#include "imem.h"
#include "logmath.h"
#include "logfile.h"
#define G_E 2.71828182845904523
extern class LogFile *flog;
#endif






