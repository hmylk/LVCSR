/* 
   This file defines common extention to C++ language
*/

#ifndef __COMMON_MODULE__
#define __COMMON_MODULE__
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
//#include <unistd.h>
#include "logmath.h"

/* no redundant output */
#define SILENCE_MODE
/* no redundant output */

/* multi thread */
#define MULTI_THREAD
/* multi thread */

/* not write feature */
//#define NOT_WRITE_FEATURE
/* not write feature */

/* consider riff */
#define CONSIDER_RIFF
/* consider riff */

//LJ--
//#include "..\\Tprintf.h"
//#define printf	tprintf

/* below from feat_src */
//#define ushort	unsigned short
#define STRINGLENGTH 256
typedef char stringbuf[STRINGLENGTH];

#define ASSERT1(CND,MSG)	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__);exit(3);}
#define ASSERT2(CND,MSG,PTM) if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM);exit(3);}
#define ASSERT3(CND,MSG,PTM1,PTM2) 	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM1,##PTM2);exit(3);}
#define ASSERT4(CND,MSG,PTM1,PTM2,PTM3)  if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM1,##PTM2,##PTM3);exit(3);}
#define ASSERT5(CND,MSG,PTM1,PTM2,PTM3,PTM4) 	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM1,##PTM2,##PTM3,##PTM4);exit(3);}
#define WARNING1(MSG) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__)
#define WARNING2(MSG,PTM) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,##PTM)
#define WARNING3(MSG,PTM1,PTM2) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,##PTM1,##PTM2)
#define WARNING4(MSG,PTM1,PTM2,PTM3) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,##PTM1,##PTM2,##PTM3)
/* above from feat_src */

#ifdef __GNUC__

#include <unistd.h>

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(CND,MSG,PTM...) if (!(CND)) {fprintf(stderr, \
   "\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM);exit(3);}
#ifdef NO_SOUND
#define WARNING(MSG,PTM...) fprintf(stderr, \
   "WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM)
#else
#define WARNING(MSG,PTM...) fprintf(stderr, \
   "\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ ,##PTM)
#endif

#else

typedef unsigned short ushort;

#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(CND,MSG,PTM) if (!(CND)) {fprintf(stderr, \
        "(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ );exit(3);}

#ifdef NO_SOUND
#define WARNING(MSG,PTM) fprintf(stderr, \
	"WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ )
#else
#define WARNING(MSG,PTM) fprintf(stderr, \
	"WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__ )
#endif
#endif


#define ReadOpen(X,Y) ASSERT(X=fopen(Y,"rb"),"Cannot open %s",Y)
#define WriteOpen(X,Y) ASSERT(X=fopen(Y,"wb"),"Cannot open %s",Y)
#define AppendOpen(X,Y) ASSERT(X=fopen(Y,"ab"),"Cannot open %s",Y)
#define TextReadOpen(X,Y) ASSERT(X=fopen(Y,"rt"),"Cannot open %s",Y)
#define TextWriteOpen(X,Y) ASSERT(X=fopen(Y,"wt"),"Cannot open %s",Y)
#define TextAppendOpen(X,Y) ASSERT(X=fopen(Y,"at"),"Cannot open %s",Y)

#ifdef __GNUC__
#ifndef USE_STL
#define min(X,Y)  ((X) <? (Y))
#define max(X,Y)  ((X) >? (Y))
#endif
#else
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/* below from feat_src */
#define Max_Mono_Num		2048			// 2003.11.25  : add
#define Max_Phone_Num       10000

#define Max_Vec_Size		256

#define Min_Mix_Weight      1.0e-5

#define MINVAR				1E-30
#define MAXVAR				1E+30

#define MAXgarbageModels	16

#define MAXwordNameLen		32
#define MAXphnNameLen		32
#define MAXvecSize			52
#define MAXmacroNameLen		32
#define MAXhmmMixtureNum	15
#define MAXspellNum			32
#define MAXmonoPhnNameLen	32
#define MAXnBest			20
#define MAXappendLine		16
#define MAXnGram			4
#define MAXwordLen			10000
#define MAXalignLen			50000


// 2004.06.30 plu : ADD
#define FYT_MFCC     6
#define FYT_MFCCPLP  9
#define FYT_RASTAPLP 12
#define FYT_MFCCPLP_NCC 15
#define KINDMASK    (0x3f)

#define DYN_RANGE   50

// 2007.01.18 plu : add
#define HASENERGY  0100       /* _E log energy included */
#define HASDELTA   0400       /* _D delta coef appended */
#define HASACCS   01000       /* _A acceleration coefs appended */
//#define HASZEROM  04000       /* _Z zero meaned */
#define HASTHIRD 0100000       /* _T has Delta-Delta-Delta index attached */
#define HASZEROC 020000       /* _0 0'th Cepstra included */

#define BASEMASK  077         /* Mask to remove qualifiers */


// 2008.5.26 llu : ADD
#define FYT_ALAW     1
#define FYT_PCM16    0


struct FEATURE_BASEINFO
{
	char  targetKind[20];

	int   smpPeriod;				// 采样周期，帧长
	int   framePeriod;

	// MfccBased
	bool  zeroGlobalMean;
	int   chlNum;					// 滤波器数目（通道数），倒谱数
	int	  cepNum;

	int	  cepLifter;				// 倒谱提升，窗长
	int	  winSize;

	float lowPass;					// 低/高截至频率
	float highPass;

	float WarpAlpha;				// 频谱卷曲参数
	float WarpLCutoff;				// WARP 低端截止频率 
	float WarpUCutoff;				// WARP 高端截止频率

	// LpcBased
	int   lpcOrder;						// LPC阶数
	float compressFact;				// Compression Factor fo PLP 

	// EnergyBased
	bool  normEnergy;
	float energyScale;
	float silFloor;

	// RastaBased
	bool bDoRasta;
	bool bDoCep;
	bool bgainflag;

	int stepsize;

	float fpole;
	float fcepLifter;
	//add by zliu add pitch
	bool bPitch;
	bool bHLDA;
	int	 tgtDim;

	//add by llu
	bool bonline;
	int  cfmt;

	FEATURE_BASEINFO()
	{
		targetKind[0]='\0';

		smpPeriod=625;					// 采样周期，帧长
		framePeriod=100000;

		// MfccBased
		zeroGlobalMean=false;
		chlNum=24;						// 滤波器数目（通道数），倒谱数
		cepNum=12;

		cepLifter=22;					// 倒谱提升，窗长
		winSize=250000;

		lowPass=-1.f;					// 低/高截至频率
		highPass=-1.f;

		WarpAlpha=1.f;					// 2005.03.01 plu :	
		WarpLCutoff=-1.f;
		WarpUCutoff=-1.f;

		// LpcBased
		lpcOrder=12;						// LPC阶数
		compressFact=0.33f;				// Compression Factor fo PLP 

		// EnergyBased
		normEnergy=false;
		energyScale=1.f;
		silFloor=50.f;

		// RastaBased
		bDoRasta=true;
		bDoCep=true;
		bgainflag=true;

		stepsize=10;					// 10ms

		fpole=0.94f;
		fcepLifter=0.6f;	

		bonline = 0;
		cfmt=0;
	};
};


struct FEATURE_MFCCBASEINFO
{
	char  targetKind[20];

	int   smpPeriod;				// 采样周期，帧长
	int   framePeriod;

	// MfccBased
	bool  zeroGlobalMean;
	int   chlNum;					// 滤波器数目（通道数），倒谱数
	int	  cepNum;

	int	  cepLifter;				// 倒谱提升，窗长
	int	  winSize;

	float lowPass;					// 低/高截至频率
	float highPass;

	float WarpAlpha;				// 频谱卷曲参数
	float WarpLCutoff;				// WARP 低端截止频率 
	float WarpUCutoff;				// WARP 高端截止频率

	// EnergyBased
	bool  normEnergy;
	float energyScale;
	float silFloor;

	// robust Processing
	bool  doCMN;
	bool  doCVN;
	bool  doRASTA;
	float RASTACoff;
	bool  doFeatWarp;

	FEATURE_MFCCBASEINFO()
	{
		targetKind[0]='\0';

		smpPeriod=625;					// 采样周期，帧长
		framePeriod=100000;

		// MfccBased
		zeroGlobalMean=false;
		chlNum=24;						// 滤波器数目（通道数），倒谱数
		cepNum=12;

		cepLifter=22;					// 倒谱提升，窗长
		winSize=250000;

		lowPass=-1.f;					// 低/高截至频率
		highPass=-1.f;

		WarpAlpha=1.f;					// 2005.03.01 plu :	
		WarpLCutoff=-1.f;
		WarpUCutoff=-1.f;

		// EnergyBased
		normEnergy=false;
		energyScale=1.f;
		silFloor=50.f;

		// robust processing
		doCMN = false;
		doCVN = false;
		doRASTA = false;
		doFeatWarp = false;
		RASTACoff = 0.98f;	// or 0.94
	};
};
/* above from feat_src */

#define STRINGLENGTH 256
#ifdef USE_STL
  typedef char myString[STRINGLENGTH];
#else
  typedef char mystring[STRINGLENGTH];
#endif


#ifdef INT_PROB

#define PROBTYPE int
#define LOGPROBMIN10 (-976562) // LOGZERO/1024
#define RCVTSCALE(x) (float)((x)/1024.)
#define CVTSCALE(x) (int)( ((x)<LOGPROBMIN10) ? LOGZERO : ((x)*1024.))
#define CVTBIT(x) (((x)<LOGPROBMIN10) ? LOGZERO : ((x)<<10))

#else

#define PROBTYPE float
#define RCVTSCALE(x) (x)
#define CVTSCALE(x) (x)
#define CVTBIT(x) (x)

#endif



#endif


