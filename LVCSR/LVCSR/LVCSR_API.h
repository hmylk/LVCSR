
#ifndef __LVCSR_API_H_
#define __LVCSR_API_H_


#ifndef LVCSR_API_API
#ifdef WIN32
#define LVCSR_API_API  __declspec(dllexport)
#else
#define LVCSR_API_API
typedef long long int __int64;
#endif
#endif

/************************************************************************/
/* Data struct  definition                                              */
/************************************************************************/
#include "commStuct.h"

#define ENHANCED_RESULT

enum SOUND_TYPE
{	
	MALE=0, 
	FEMALE, 
	MUSIC, 
	NOISE, 
	UNK,
};

// Candidate rec result struct
struct ACand
{
	char *text;         // Candidate rec text result 
	float score;        // Candidate score，0.00~100.00，candNum个
	float startTime;    // the start time，Unit for seconds
	float endTime;      // the end time，Unit for seconds
	ACand *next;        // next Candidate rec struct

#ifdef ENHANCED_RESULT
	char * phone;       //phoneme string
	char * segTime;     //segTime string
#endif
	ACand ()
	{
		text = 0;
		next = 0;
		score = startTime = endTime = 0;
		phone = 0;
		segTime = 0;
	};
};

//lvcsr result struct
struct WordResult   
{
	int   candNum;     // Candidate rec result number ,at least one , the first ACand is 1-best
	ACand *cands;      // Candidate result link
	WordResult *next;  //the next sentence result
	WordResult()
	{
		candNum = 0;
		cands = 0;
		next = 0;
	}
};

// rec result/decode task definition
struct Task	{                       
	void         *newSet;           // could also be pResultArray, pWORDRESULT
	void         *latSet;	        //lattice set
	int           frameNum;         // could also be numberOfTasks, wordNumber
	long long         sessionId;
	unsigned long saveFileIdx;      // real serialNum, from 0 , 类内唯一
	unsigned long dataTimestamp;    // sample of start
	long          decoderId;        // could also be hHandle
	unsigned long dataTimestampEnd; // sample of end
	SOUND_TYPE    soundType;
};


enum SPEECHTYPE
{
	TSR_ALAW_PCM         = 0,       //8k 8bit a-law
	TSR_ULAW_PCM         = 1,       //8k 8bit u-law
	TSR_LINEAR_PCM       = 2,       //8k 8bit linear pcm, centered at value 128
	TSR_ADPCM            = 3,       //ADPCM
	TSR_RAW_16           = 4 | TSR_LINEAR_PCM,   //8k 16 bit raw data
	TSR_16K_16           = 8 | TSR_RAW_16,       //16k 16 bit raw data
};


typedef struct _TriPhoneLnk {
	int dur;
	int start;
	char phone[32];
	char lphone[32];
	char rphone[32];
	unsigned short phoneIdx;		// corresponding to state index
	float PLPP;						// PLPP confidence
	_TriPhoneLnk()
	{
		dur			= 0;
		start		= 0;
		phone[0]	= '\0';
		lphone[0]	= '\0';
		rphone[0]	= '\0';
		phoneIdx = 0;
		PLPP = -100;
	}
}aTriPhoneLnk;


//for a keyword 
typedef struct _aWORDLINK
{
	char	word[256];
	int		starttime;			//start time in frame numbers
	int		endtime;			//end time in frame numbers
	float	cn_confidence;		//confidence from confusion network
	float	plpp_confidence;	//confidence from the plpp
	float	mix_confidence;		//confidence from fusions
	int		nPhone;
	aTriPhoneLnk*triphone;		//tri-phone information
	_aWORDLINK()
	{
		word[0]			='\0';
		starttime		=0;
		endtime			=0;
		cn_confidence	=0;
		plpp_confidence	=0;
		mix_confidence	=0;
		triphone		=0;
		nPhone			=0;
	}
}aWORDLINK;

enum LVCSR_ERROR
{
	LVCSR_SUCCESS = 0,
	LVCSR_ERROR_COMMON = -1,
	LVCSR_ERROR_LICENSE_INVALID = -2
};

/************************************************************************/
/* Interface function  definition                                       */
/************************************************************************/
LVCSR_API_API int LVCSR_Init(const char* sysDir,const char*  cfgFile,int lineNum);

LVCSR_API_API int LVCSR_Exit();

LVCSR_API_API int LVCSR_Start(int sessionId=0);

LVCSR_API_API int LVCSR_StopRecording(int sessionId = 0);

LVCSR_API_API int LVCSR_Stop(int sessionId);

LVCSR_API_API int LVCSR_SendData(FeSegInfo* segInfo,FeSpeechAttri* dataInfo,int sessionId);

typedef void (*LVCSR_ResultCallback) (FeSegInfo * pResultArray ,bool bIsOver,int sessionId);   
LVCSR_API_API int LVCSR_SetResultCallbackFunc(LVCSR_ResultCallback pFunc, int sessionId = 0);

LVCSR_API_API int LVCSR_SampleRate();

LVCSR_API_API int LVCSR_SetParam(char *paramname, char *paramvalue, int sessionId);
#endif