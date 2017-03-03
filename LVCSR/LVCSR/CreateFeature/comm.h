/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : comm.h
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0002
*D
*D 文件描述            :
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.09.18             创建文件
*D 1.1.0002     2005.03.01     plu        增加对VTLN的支持：
*D*******************************************************************************/

#ifndef __FEAT_COMMON_MODULE__   // __COMMON_MODULE__
#define __FEAT_COMMON_MODULE__
#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <process.h>
#else
#include <unistd.h>
#endif

typedef unsigned short ushort;   // #define ushort	unsigned short syq 070706 change

#define STRINGLENGTH 256

typedef char stringbuf1[STRINGLENGTH];

#ifdef WIN32
#define ASSERT2(CND,MSG)	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__);exit(3);}
#define ASSERT3(CND,MSG,PTM) if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM);exit(3);}
#define ASSERT4(CND,MSG,PTM1,PTM2) 	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM1,##PTM2);exit(3);}
#define ASSERT5(CND,MSG,PTM1,PTM2,PTM3)  if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM1,##PTM2,##PTM3);exit(3);}
#define ASSERT6(CND,MSG,PTM1,PTM2,PTM3,PTM4) 	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,##PTM1,##PTM2,##PTM3,##PTM4);exit(3);}

#define WARNING1(MSG) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__)
#define WARNING2(MSG,PTM) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,##PTM)
#define WARNING3(MSG,PTM1,PTM2) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,##PTM1,##PTM2)
#define WARNING4(MSG,PTM1,PTM2,PTM3) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,##PTM1,##PTM2,##PTM3)
#else
#define ASSERT2(CND,MSG)	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__);exit(3);}
#define ASSERT3(CND,MSG,PTM) if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM);exit(3);}
#define ASSERT4(CND,MSG,PTM1,PTM2) 	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM1,#PTM2);exit(3);}
#define ASSERT5(CND,MSG,PTM1,PTM2,PTM3)  if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM1,#PTM2,#PTM3);exit(3);}
#define ASSERT6(CND,MSG,PTM1,PTM2,PTM3,PTM4) 	if (!(CND)) {fprintf(stderr,"\007ERROR(%s:%d)\n" #MSG "\n",__FILE__,__LINE__ ,#PTM1,#PTM2,#PTM3,#PTM4);exit(3);}

#define WARNING1(MSG) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__)
#define WARNING2(MSG,PTM) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,#PTM)
#define WARNING3(MSG,PTM1,PTM2) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,#PTM1,#PTM2)
#define WARNING4(MSG,PTM1,PTM2,PTM3) fprintf(stderr,"\007WARNING(pid=%d,%s:%d)." #MSG "\n",getpid(),__FILE__,__LINE__,#PTM1,#PTM2,#PTM3)
#endif

#define ReadOpen(X,Y)       ASSERT3(X=fopen(Y,"rb"),"Cannot open %s for read.",Y)
#define WriteOpen(X,Y)      ASSERT3(X=fopen(Y,"wb"),"Cannot open %s for write",Y)
#define AppendOpen(X,Y)     ASSERT3(X=fopen(Y,"ab"),"Cannot open %s for append",Y)
#define TextReadOpen(X,Y)   ASSERT3(X=fopen(Y,"rt"),"Cannot open %s for text read",Y)
#define TextWriteOpen(X,Y)  ASSERT3(X=fopen(Y,"wt"),"Cannot open %s for text write",Y)
#define TextAppendOpen(X,Y) ASSERT3(X=fopen(Y,"at"),"Cannot open %s for text append",Y)

#ifndef USE_STL
#define min(X,Y)  (((X)<(Y))?X:Y)
#define max(X,Y)  (((X)>(Y))?X:Y)
#endif


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
	int	 tgtDim;
	bool bonline;
	int  cfmt;
    bool bHLDA;
	

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
		bPitch=false;

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
#endif
