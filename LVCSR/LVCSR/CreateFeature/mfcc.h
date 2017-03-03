/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : mfcc.h
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0002
*D
*D 文件描述            : 
*D			pay attention: MFCC class just create basic mfcc or mfccplp 
*D						   feature.
*D						   other process, such as delta, deltadelta 
*D						   and cms, is implemented by Feature class
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.11.24             创建文件
*D 1.1.0002     2005.03.01     plu        增加对VTLN的支持
*D*******************************************************************************/

#ifndef aaaMFCC_MODULEaaa
#define aaaMFCC_MODULEaaa
#include <math.h>
#include "comm.h"
#include "ipps.h"
#include "ippsr.h"
#include "PitchTracker.h"

/*// plu 2007.01.18_20:00:48
#define DYN_RANGE   50
#define HASENERGY   64
*/// plu 2007.01.18_20:00:48

#define ippsMeanVarCol_32f_D2L ippsMeanVarColumn_32f_D2L
#define hint ippAlgHintAccurate

// there are too many parameters need to be pre-set. I put them here:
//      smpPeriod: 625,    framePeriod: 100000
//      chlNum:    24      cepNum:      12
//      cepLifer:  22


/*// plu 2004.06.30_14:23:00
typedef enum {
   Other=0, Background, Speech
} aSegType;
*/// plu 2004.06.30_14:23:00

/*// plu 2004.06.30_15:15:31
struct MFCC_BASEINFO	    // basic information needed by MFCC class
{
    stringbuf targetKind;
    int smpPeriod,framePeriod;
    int chlNum,cepNum,cepLifter,winSize;    
    / // Replaced by ProjectTools 2004.06.30_15:15:31/ plu 2004.06.30_14:35:53
    bool zeroGlobalMean,zeroFrameMean,normEnergy;
     // Replaced by ProjectTools 2004.06.30_15:15:31// plu 2004.06.30_14:35:53
    bool zeroGlobalMean,normEnergy;
    float lowPass,highPass;
    float energyScale,silFloor;
};
*/// plu 2004.06.30_15:15:31

class MFCC2 
{
   protected:
      /*// plu 2004.06.30_16:39:30
      stringbuf outFile;
      */// plu 2004.06.30_16:39:30
      bool bBaseSet,bCutSilSet,bInitialize;		
     
      /*// plu 2004.06.30_15:15:37
      MFCC_BASEINFO baseInfo;
      */// plu 2004.06.30_15:15:37
	  FEATURE_BASEINFO baseInfo;

      int   winSize,*chlMap,kLow,kHigh;
      float fres,*chlWeight,*hamWin,*cepWin,*fBank;

      short mfccKind;
      int BaseDim,mfccFFTNum,frameRate,FrameNum;

      float *mfccData, *energybuff, *powerSpec;


	  // 2005.03.01 plu : 
	  float warpAlpha,warpLowCut,warpUpCut;

	  PitchTrack *pitchTrack;//add pitch inf by zliu
	  int *pitch;

	  //for ippuse
	  int mfccFFTOrder; 
	  float mfnorm;
	  float *cosTab;
	  float *ek; 
	  float *ek_t1; 
	  short *fBankStart;
	  short *fBankLength;
	  IppsFFTSpec_R_32f* ctxFFT;

      float Mel(int k) 
	  { 
		  return (float)(1127.f*log(1.0+(float)k*fres)); 
	  }

      // add by lp (delete FFT class)
      void DoFFT(float *s, int n,const bool inverted=false);
      void RealFFT(float *s,int n);

      //void  InitFBank(float gama=1.f);
	  void  InitFBank();
      void  ZeroGlobalMean(short *data,int smpCount);
      float ApplyFFT(short *wave,float *vfft);
      void  LPC2Cep(float *a,float *c);
      void  FinalCompute(float *vfft,float energy,float *mfcc);
	  void  FinalCompute(float *vfft,float energy,int pitch, float *mfcc);

      void  NormEnergy();
//      void  ZeroCepMean();

	  // 2005.03.01 plu : 
	  float WarpFreq (float fcl, float fcu, float freq, float minFreq, float maxFreq , float alpha);

	  // 2007.01.17 plu : CMS
	  void ZeroCepMean(float *data, int vSize, int n, int step);

	  void CompDelta(float *CepData,float *DeltaData,const int winSize,const double sigma);

	  void CepMeanVarNorm(float *data, int vSize, int n, int step);



public:
    
      //MFCC(char *filename);
	  MFCC2();
      ~MFCC2();

      /*// plu 2004.06.30_15:15:56
      virtual void SetBaseInfo(MFCC_BASEINFO p_info);
      */// plu 2004.06.30_15:15:56
	  void SetBaseInfo(FEATURE_BASEINFO p_info);
      virtual void Initialize(int &BaseDim2);
    
      virtual void AddWaveData(short *data,int p_smpNum,int *residue=NULL);
      virtual void WriteFile(char *outFileName);
	  virtual float *GetFeatureDate(int *frm,short *tframeSize);

	  // 2004.07.06 plu : 
	  float * GetMfccData()		{	return mfccData;	}
	  float * GetPowerSpec()	{	return powerSpec;	}
	  float * GetEnergy()		{   return energybuff;  }
	  int GetFrameNumber(void)		{	return FrameNum;	}
	  int GetBaseDim(void)		{	return BaseDim;		}
	  bool GetfullMfcc(float **fm, unsigned* nframe);
	  virtual float * GetObv(int fm);

};

#endif

