/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : mfcc.cpp
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0003
*D
*D 文件描述            : mfcc feature
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.11.24             创建文件
*D 1.1.0002     2005.03.01     plu        增加对VTLN的支持
*D 1.1.0003     2007.01.19     plu        same as HTK : kLow=(int)((baseInfo.lowPass*baseInfo.smpPeriod*(1.0e-7)*mfccFFTNum)+2.5);
*D*******************************************************************************/

#include <memory.h>
#include <string.h>
//#include "comm.h"
#include "logmath.h"
#include "mfcc.h"
#define UseIPP

//MFCC::MFCC(char *filename)
MFCC2::MFCC2()
{
    /*// plu 2004.06.30_16:39:49
    FILE *fp=NULL;
    fp=fopen(filename,"wb");
    ASSERT2(fp,"error in MFCC : can't open %s to write!",filename);
	//ASSERT1(fp);
    fclose(fp);
    strcpy(outFile,filename);
    */// plu 2004.06.30_16:39:49
    
    chlWeight=NULL; chlMap=NULL;    fBank=NULL;    
    cepWin=NULL;    hamWin=NULL;    mfccData=NULL;
	energybuff=NULL; powerSpec=NULL;

	pitch=NULL;//add pitch info by zliu
	pitchTrack=NULL;

	//ippuse
	fBankLength=NULL;
	fBankStart=NULL;
	cosTab=NULL;
	ek=NULL;
	ek_t1=NULL;

    bBaseSet=bCutSilSet=bInitialize=false;		
}

MFCC2::~MFCC2()
{
    if (chlWeight)  free(chlWeight);
    if (chlMap)	    free(chlMap);
    if (fBank)	    free(fBank);
    if (cepWin)	    free(cepWin);
    if (hamWin)	    free(hamWin);
    if (mfccData)   free(mfccData);
	if (energybuff) free(energybuff);

	if (powerSpec)	free(powerSpec);
	if(pitchTrack) delete pitchTrack;
	//ippuse
	if (fBankStart) free(fBankStart);
    if (fBankLength) free(fBankLength);
    if (cosTab) free(cosTab);
    if (ek) free(ek);
    if (ek_t1) free(ek_t1);
	ippsFFTFree_R_32f(ctxFFT);
}

/*// plu 2004.06.30_15:17:16
void MFCC::SetBaseInfo(MFCC_BASEINFO p_info)
*/// plu 2004.06.30_15:17:16
void MFCC2::SetBaseInfo(FEATURE_BASEINFO p_info)
{
    mfccKind=0;
	if (strstr(p_info.targetKind,"MFCCPLP"))
	{
		printf("Error set targetkind in MFCC!\n");	exit(-1);		
	}
    if (strstr(p_info.targetKind,"MFCC"))
	{
		mfccKind=FYT_MFCC;    
		if (strstr(p_info.targetKind,"_E"))		mfccKind|=HASENERGY;
	}
	
	else
	{
		printf("Error set targetkind in MFCC!\n");	exit(-1);				
	}
    ASSERT3(mfccKind&KINDMASK,"Unsupported target kind: %s",p_info.targetKind);

    baseInfo.smpPeriod   = p_info.smpPeriod;
    baseInfo.framePeriod = p_info.framePeriod;

    baseInfo.lowPass  = p_info.lowPass;
    baseInfo.highPass = p_info.highPass;
    baseInfo.winSize  = p_info.winSize;
    baseInfo.chlNum   = p_info.chlNum;
    baseInfo.cepNum   = p_info.cepNum;

    
    baseInfo.cepLifter = p_info.cepLifter;    

    baseInfo.zeroGlobalMean = p_info.zeroGlobalMean;
    /*// plu 2004.06.30_14:35:43
    baseInfo.zeroFrameMean  = p_info.zeroFrameMean;
    */// plu 2004.06.30_14:35:43
    if (mfccKind&HASENERGY) 
		baseInfo.normEnergy = p_info.normEnergy;
    else
		baseInfo.normEnergy = false;

    if (baseInfo.normEnergy)
    {
		baseInfo.energyScale = p_info.energyScale;
		baseInfo.silFloor    = p_info.silFloor;
    }
    
	// 2005.03.01 plu : 
	warpAlpha = p_info.WarpAlpha;
	warpLowCut = p_info.WarpLCutoff;
	warpUpCut = p_info.WarpUCutoff;

    if (warpAlpha < 0.5f || warpAlpha > 2.f)
	{
		printf("ValidCodeParms: unlikely warping factor %f\n", warpAlpha);
		return;
	}

   if (warpAlpha != 1.0) 
   {
      if ( fabs(warpLowCut)<1.0e-7 || fabs(warpUpCut)<1.0e-7 ||
          warpLowCut > warpUpCut)
      {
		  printf("ValidCodeParms: invalid warping cut-off frequencies %f %f \n",warpLowCut, warpUpCut);
		  return;
	  }

      if ( fabs(warpUpCut)<1.0e-7 && warpLowCut != 0.0) 
	  {
         warpUpCut = warpLowCut;
         printf("ValidCodeParms: setting warp cut-off frequencies to %f %f\n",
                 warpLowCut, warpUpCut);
		 return;
      }
      if ( fabs(warpLowCut)<1.0e-7 && warpUpCut != 0.0) 
	  {
         warpUpCut = warpLowCut ;
         printf("ValidCodeParms: setting warp cut-off frequencies to %f %f\n",
                 warpLowCut, warpUpCut);
		 return;
      }
   }    

	bBaseSet=true;
	// add by zliu
	baseInfo.bPitch = p_info.bPitch;

}

void MFCC2::Initialize(int &BaseDim2) 
{
   ASSERT2(bBaseSet,"pls call SetBaseInfo() firstly!");
   int a = baseInfo.framePeriod;
   int b = baseInfo.smpPeriod;
   frameRate=(baseInfo.framePeriod)/(baseInfo.smpPeriod);
   winSize=baseInfo.winSize/baseInfo.smpPeriod;
   mfccFFTNum=2;
   mfccFFTOrder=1;
   while (mfccFFTNum<winSize) 
   {
	   mfccFFTNum*=2;
	   mfccFFTOrder++;
   }

   BaseDim=(mfccKind&HASENERGY)?(baseInfo.cepNum+1):baseInfo.cepNum;
   //add by zliu 
   if (baseInfo.bPitch) {
	   BaseDim += 1; //add pitch
   }
   BaseDim2 = BaseDim;

   chlWeight=(float *)malloc(sizeof(float)*mfccFFTNum/2);
   chlMap=(int *)malloc(sizeof(int)*mfccFFTNum/2);
   fBank=(float *)malloc(sizeof(float)*baseInfo.chlNum);
   ASSERT2(chlWeight,"error alloc space for chlWeight!");
   ASSERT2(chlMap,"error alloc space for chlMap!");
   ASSERT2(fBank,"error alloc space for fBank!");

   //for ippuse
   fBankStart =(short *)malloc(sizeof(short)*(baseInfo.chlNum+1));
   fBankLength =(short *)malloc(sizeof(short)*(baseInfo.chlNum+1));
   ek=(float *)malloc(sizeof(float)*mfccFFTNum/2);
   ek_t1=(float *)malloc(sizeof(float)*mfccFFTNum/2);
   ASSERT2(fBankStart,"error alloc space for fBankStart!");
   ASSERT2(fBankLength,"error alloc space for fBankLength!");
   ASSERT2(ek,"erro alloc space fo ek!");
   ASSERT2(ek_t1,"erro alloc space for ek_t1!");
   mfnorm=(float)sqrt(2.0/(float)baseInfo.chlNum);

   int i;
   if (baseInfo.cepLifter>0) 
   {
      cepWin=(float *)malloc(sizeof(float)*baseInfo.cepNum);
      ASSERT2(cepWin,"error alloc space for cepWin");
      float tmp=(float)(PI/(float)baseInfo.cepLifter);
      for (i=0;i<baseInfo.cepNum;i++)
         cepWin[i]=(float)(1.f+(float)baseInfo.cepLifter/2.0*sin((i+1)*tmp));
   }

   hamWin=(float *)malloc(sizeof(float)*winSize);
   ASSERT2(hamWin,"error alloc space for hamWin!");
   ippsSet_32f(1, hamWin, winSize);
   ippsWinHamming_32f_I(hamWin, winSize);
   //end of ippuse
#ifndef UseIPP
   float tmp=(float)(PI*2.0/(float)(winSize-1));
   for (i=0;i<winSize;i++)
      hamWin[i]=(float)(0.54-0.46*cos(tmp*i));
#else
   i=baseInfo.cepNum;
   cosTab = (float *)malloc(sizeof(float)*i*baseInfo.chlNum);
   for (int j=0; j < i; j++) {
       float x = (float)((double)(j+1)*PI/(double)baseInfo.chlNum);
       for (int k=0; k < baseInfo.chlNum; k++)
           cosTab[j*baseInfo.chlNum+k] = (float)cos(x*(k+0.5));
   }
   IppStatus status;
   status = ippsFFTInitAlloc_R_32f(&ctxFFT,mfccFFTOrder,IPP_FFT_NODIV_BY_ANY,ippAlgHintNone);
#endif
//   InitFBank(1.f);
   InitFBank();
   //add by zliu
	if (baseInfo.bPitch) 
		pitchTrack = new PitchTrack(baseInfo.smpPeriod, baseInfo.winSize, baseInfo.framePeriod);
   bInitialize=true;
}

// 2005.03.01 plu : 
/* EXPORT->WarpFreq: return warped frequency */
float MFCC2::WarpFreq (float fcl, float fcu, float freq, float minFreq, float maxFreq , float alpha)
{
   if ( fabs(alpha-1.0)<1.0e-7 )
      return freq;
   else {
      float scale = 1.f / alpha;
      float cu = fcu * 2 / (1 + scale);
      float cl = fcl * 2 / (1 + scale);

      float au = (maxFreq - cu * scale) / (maxFreq - cu);
      float al = (cl * scale - minFreq) / (cl - minFreq);
      
      if (freq > cu)
         return  au * (freq - cu) + scale * cu ;
      else if (freq < cl)
         return al * (freq - minFreq) + minFreq ;
      else
         return scale * freq ;
   }
}


// float gama: may be used in VTLN
//void MFCC::InitFBank(float gama) 
void MFCC2::InitFBank() 
{
   float mlow,mhigh,mwidth,*bnkCentre;
   int i,k;

   int chlNum=baseInfo.chlNum;
   fres=(float)(1.0e7/(baseInfo.smpPeriod*mfccFFTNum*700.0));
   kLow=2; mlow=0;
   if (baseInfo.lowPass>=0.0) 
   {
      mlow=(float)(1127*log(1.0+baseInfo.lowPass/700.0));
      kLow=(int)((baseInfo.lowPass*baseInfo.smpPeriod*(1.0e-7)*mfccFFTNum)+0.5);
      //HTK
	  //kLow=(int)((baseInfo.lowPass*baseInfo.smpPeriod*(1.0e-7)*mfccFFTNum)+2.5);
      if (kLow<2) kLow=2;
   }
   kHigh=mfccFFTNum/2; mhigh=Mel(mfccFFTNum/2);
   if (baseInfo.highPass>=0.0) {
      mhigh=(float)(1127*log(1.0+baseInfo.highPass/700.0));
      kHigh=(int)((baseInfo.highPass*baseInfo.smpPeriod*(1.0e-7)*mfccFFTNum)+0.5);
      if (kHigh>mfccFFTNum/2) kHigh=mfccFFTNum/2;
   }

   bnkCentre=(float *)malloc(sizeof(float)*(baseInfo.chlNum+1));
   ASSERT2(bnkCentre,"error alloc space for bnkCentre!");
   mwidth=mhigh-mlow;

   // 2005.03.01 plu : 
   if ( fabs(warpAlpha-1.0)<1.0e-7 )
   {
	   for (i=0;i<=baseInfo.chlNum;i++)
			bnkCentre[i]=((float)(i+1)/(float)(baseInfo.chlNum+1))*mwidth+mlow;
   }
   else
   {
         float minFreq = (float)(700.0 * (exp (mlow / 1127.0) - 1.0 ));
         float maxFreq = (float)(700.0 * (exp (mhigh / 1127.0) - 1.0 ));
         float cf;
		 
 	     for (i=0;i<=baseInfo.chlNum;i++)
		 {
			 cf = ((float)(i+1)/(float)(baseInfo.chlNum+1))*mwidth+mlow;
         
			 cf = (float)(700.0 * (exp (cf / 1127.0) - 1.0));
         
			 bnkCentre[i]= (float)(1127.0 * log (1.0 + WarpFreq (warpLowCut, warpUpCut, cf, minFreq, maxFreq, warpAlpha) / 700.0));	 
		 }
   }
   
   //fres*=gama;
   for (i=k=0;k<mfccFFTNum/2;k++) {
      float mel=Mel(k);
      while (bnkCentre[i]<mel && i<baseInfo.chlNum) i++; // <=
      chlMap[k]=i-1;
   }
   for (k=0;k<mfccFFTNum/2;k++) {
      i=chlMap[k];
      if (i>=0) {
         float bnki1=bnkCentre[i+1];
	 chlWeight[k]=(bnki1-Mel(k))/(bnki1-bnkCentre[i]);
      } else {
         float bnk0=bnkCentre[0];
         chlWeight[k]=(bnk0-Mel(k))/(bnk0-mlow);
      }
   }
   free(bnkCentre);
   ippsSet_16s((short)-1,fBankStart,chlNum+1);
   ippsZero_16s(fBankLength,chlNum+1);
   for (k=kLow;k<=kHigh;k++) {
      i=chlMap[k-1];
      if (i < -1 || i >= chlNum) continue;
      if (i == -1) {
          if (fBankStart[chlNum] < 0)
              fBankStart[chlNum] = k-1;
          fBankLength[chlNum]++;
          continue;
      }
      if (fBankStart[i] < 0)
          fBankStart[i]=k-1;
      fBankLength[i]++;
   }
}

void ShowMean(float *bufData,int BaseDim,int FrameNum)
{
	float tmpMean[13];

	memset(tmpMean,0,sizeof(float)*13);
	int i=0;
	for (i=0;i<FrameNum;i++)
		for (int j=0;j<BaseDim;j++)
			tmpMean[j] += bufData[i*BaseDim+j];

	for (i=0;i<BaseDim;i++)
	{
		tmpMean[i] /= (float)FrameNum;
		printf("%10.6f",tmpMean[i]);
	}

	printf("\n\n");
}

void ShowVar(float *bufData,int BaseDim,int FrameNum)
{
	float tmpMean[13],tmpVar[13][13];

	memset(tmpMean,0,sizeof(float)*13);
	memset(tmpVar,0,sizeof(float)*13*13);
	int i=0;
	for (i=0;i<FrameNum;i++)
		for (int j=0;j<BaseDim;j++)
		{
			tmpMean[j] += bufData[i*BaseDim+j];
			//tmpVar[j] += bufData[i*BaseDim+j]*bufData[i*BaseDim+j];
		}

	for (i=0;i<BaseDim;i++)
		tmpMean[i] /= (float)FrameNum;
		//tmpVar[i] /= (float)FrameNum;	tmpVar[i] -= tmpMean[i]*tmpMean[i];
		//printf("%10.6f",tmpVar[i]);

	for (i=0;i<FrameNum;i++)
		for (int j=0;j<BaseDim;j++)
		{
			for (int k=0;k<BaseDim;k++)
				tmpVar[j][k] += (bufData[i*BaseDim+j] - tmpMean[j])*(bufData[i*BaseDim+k] - tmpMean[k]);
		}
	for (i=0;i<BaseDim;i++)
	{
		for (int j=0;j<BaseDim;j++)
		{
			tmpVar[i][j] /= (float)FrameNum;
			printf("%10.5f",tmpVar[i][j]);
		}
		printf("\n");
	}
	printf("\n\n");
}

void MFCC2::AddWaveData(short *waveData, int smpNum, int *residue)
{
    ASSERT2(bInitialize,"pls call Initialize() firstly!");
    ASSERT2(waveData,"error in AddWaveData(): input wavedata=NULL!");
    ASSERT2(smpNum>0,"error in AddWaveData(): input smpNum <= 0!");
    
    FrameNum=(smpNum-winSize)/frameRate+1; // 只少不多
	if (residue) *residue = smpNum-(FrameNum-1)*frameRate-winSize;
	//if (FrameNum < 3) {FrameNum=0;return;}  // 080519 jgao found bug of feat less than 3 frames

	if (baseInfo.bPitch){
		pitch=NULL;
		short *tmpwav = new short [smpNum];
		
//		for(int i=0; i < smpNum; i++){
//			tmpwav[i]=waveData[i];
//		}	
		memcpy(tmpwav,waveData,sizeof(short)*smpNum);
	//	short *tmpwav=waveData;
		pitchTrack->DoUtterance(tmpwav,FrameNum, &pitch);
		delete []tmpwav;
	}

    // remove time-domain mean
    if (baseInfo.zeroGlobalMean) ZeroGlobalMean(waveData,smpNum);
                 
    int i;                                                                    
    float *pCurFrame,*ffts,energy;
    short *pCurWave;

    ffts=(float *)malloc(sizeof(float)*mfccFFTNum);
	if (mfccData)	
	{
		free(mfccData);	mfccData=NULL;   // 070907 added by syq leakage bug part 1 !
	}
    mfccData=(float *)malloc(sizeof(float)*(FrameNum*BaseDim+baseInfo.chlNum));
    ASSERT2(ffts,"error alloc space for ffts!");
    ASSERT2(mfccData,"error alloc space for mfccData");
	if(energybuff) free(energybuff);
	energybuff=(float *)malloc(sizeof(float)*FrameNum);
	ASSERT2(energybuff,"error alloc space for energybuff");
	powerSpec=(float *)malloc(sizeof(float)*(FrameNum*mfccFFTNum/2));
	ASSERT2(powerSpec,"error alloc space for powerSpec!");

    pCurFrame=mfccData;	pCurWave=waveData;
    for (i=0; i<FrameNum; i++)
    {
        energybuff[i]=ApplyFFT(pCurWave,ffts);
		ippsPowerSpectr_32fc((Ipp32fc *)ffts,powerSpec+i*mfccFFTNum/2, mfccFFTNum/2);
		if (baseInfo.bPitch){
			FinalCompute(ffts,energybuff[i],pitch[i],pCurFrame);
		}else{
			FinalCompute(ffts,energybuff[i],pCurFrame);
		}

        pCurFrame += BaseDim;      pCurWave += frameRate;
    }
    free(ffts);    

    if ( (mfccKind&HASENERGY) && baseInfo.normEnergy )         NormEnergy();
    //if ( mfccKind&ZEROCEPMEAN )	   ZeroCepMean();

/*// plu 2007.01.18_20:58:25
printf("Cepstral mean:\n");
ShowMean(mfccData,BaseDim,FrameNum);

printf("Cepstral var:\n");
ShowVar(mfccData,BaseDim,FrameNum);

	// 2007.01.17 plu : add by lp
	//ZeroCepMean(mfccData,baseInfo.cepNum,FrameNum,BaseDim);
	//CepMeanVarNorm(mfccData,baseInfo.cepNum,FrameNum,BaseDim);
	//CepMeanVarNorm(mfccData,BaseDim,FrameNum,BaseDim);

printf("Cepstral mean after MVN:\n");
ShowMean(mfccData,BaseDim,FrameNum);

printf("Cepstral var after MVN:\n");
ShowVar(mfccData,BaseDim,FrameNum);

    float *DeltaData=(float *)malloc(sizeof(float)*(FrameNum*BaseDim+baseInfo.chlNum));
    ASSERT1(DeltaData,"error alloc space for mfccData");
	memset(DeltaData,0,sizeof(float)*BaseDim*FrameNum);

	CompDelta(mfccData,DeltaData,2,1.0/10.0);

printf("Delta mean\n");
ShowMean(DeltaData,BaseDim,FrameNum);

printf("Delta Var:\n");
ShowVar(DeltaData,BaseDim,FrameNum);

	memcpy(mfccData,DeltaData,sizeof(float)*BaseDim*FrameNum);
	memset(DeltaData,0,sizeof(float)*BaseDim*FrameNum);
	CompDelta(mfccData,DeltaData,2,1.0/10.0);

printf("2th-Delta mean\n");
ShowMean(DeltaData,BaseDim,FrameNum);

printf("2th-Delta Var:\n");
ShowVar(DeltaData,BaseDim,FrameNum);


	memcpy(mfccData,DeltaData,sizeof(float)*BaseDim*FrameNum);
	memset(DeltaData,0,sizeof(float)*BaseDim*FrameNum);
	CompDelta(mfccData,DeltaData,2,1.0/10.0);

printf("3th-Delta mean\n");
ShowMean(DeltaData,BaseDim,FrameNum);

printf("3th-Delta Var:\n");
ShowVar(DeltaData,BaseDim,FrameNum);


	free(DeltaData);
*/// plu 2007.01.18_20:58:25
	
//	printf("%d, %d\n", BaseDim,FrameNum);   // syq disable
}



/******************************FUNCTION*COMMENT***********************************
*F
*F 函数名称            : MFCC::ZeroCepMean
*F
*F 函数类型            : void
*F
*F 函数返回值          :
*F     无返回值
*F
*F 函数参数            :
*F     float *data
*F     int vSize
*F     int n
*F     int step
*F
*F 函数描述            :
*F     CMS
*F
*F
*F 函数修改记录
*F ------------------------------------------------------------------------------ 
*F 修改日期       修改人     改动内容
*F ------------------------------------------------------------------------------ 
*F 2007.01.17                创建函数
*F*******************************************************************************/
void MFCC2::ZeroCepMean(float *data, int vSize, int n, int step)
{
   double sum;
   float *fp,mean;
   int i,j;

   for (i=0; i<vSize; i++){
      /* find mean over i'th component */
      sum = 0.0;
      fp = data+i;
      for (j=0;j<n;j++){
         sum += *fp; fp += step;
      }
      mean = (float)(sum / (double)n);
      /* subtract mean from i'th components */
      fp = data+i;
      for (j=0;j<n;j++){
         *fp -= mean; fp += step;
      }
   }
}

void MFCC2::CepMeanVarNorm(float *data, int vSize, int n, int step)
{
   double sum;
   float *fp,mean;
   int i,j;

   // 2007.01.18 plu : 
   float fVar,fsumVar;

   for (i=0; i<vSize; i++){
      /* find mean over i'th component */
      sum = 0.0;
	  fsumVar = 0.f;
      fp = data+i;
      for (j=0;j<n;j++){
         sum += *fp; 
		 fsumVar += (*fp)*(*fp);
		 fp += step;
      }
      mean = (float)(sum / (double)n);
	  fVar = (float)sqrt((fsumVar / (double)n) - mean*mean);
      /* subtract mean from i'th components */
      fp = data+i;
      for (j=0;j<n;j++)
	  {
         *fp = (*fp-mean)/ fVar;
		 fp += step;
      }
   }
}


void MFCC2::CompDelta(float *CepData,float *DeltaData,const int winSize,const double sigma) 
{
   for (int i=0;i<FrameNum;i++) {
      for (int j=0;j<BaseDim;j++) {
         double sum=0;
         for (int k=1;k<=winSize;k++) {
            float *ffeat=CepData+min(i+k,FrameNum-1)*BaseDim;
            float *bfeat=CepData+max(i-k,0)*BaseDim;
            sum+=k*(ffeat[j]-bfeat[j]);
         }
         DeltaData[i*BaseDim+j]= (float)(sum*sigma);
      }
   }
}


void MFCC2::ZeroGlobalMean(short *data,int smpCount) {
   double mean=0.0; int i;
   for (i=0;i<smpCount;i++) mean+=data[i];
   mean/=(double)smpCount;
   for (i=0;i<smpCount;i++) {
      double y=(float)data[i]-mean;
      if (y>32767) y=32767;
      if (y<-32767) y= -32767;
      data[i]=(short)((y>0)?y+0.5:y-0.5);
   }
}

// short *wave: input wave data
// float *vfft: output fft data
float MFCC2::ApplyFFT(short *wave,float *vfft)                   
{
   float energy=0.0; int i;
#ifndef UseIPP
   for (i=0;i<winSize;i++) vfft[i]=(float)wave[i];
   for (i=winSize;i<mfccFFTNum;i++) vfft[i]=0;

   /*// plu 2004.06.30_14:35:41
   if (baseInfo.zeroFrameMean) {
      float sum=0.0;
      for (i=0;i<winSize;i++) sum+=vfft[i];
      sum/=winSize;
      for (i=0;i<winSize;i++) vfft[i]-=sum;
   }
   */// plu 2004.06.30_14:35:41

   for (i=0;i<winSize;i++)	energy+=vfft[i]*vfft[i];

   for (i=winSize-1;i>0;i--)	vfft[i] -= vfft[i-1]*0.97f;
   vfft[0] *= 0.03f;

   for (i=0;i<winSize;i++)	vfft[i] *= hamWin[i];

   RealFFT(vfft,mfccFFTNum);
#else
   ippsConvert_16s32f(wave,vfft,winSize);
   ippsZero_32f(&vfft[winSize],mfccFFTNum-winSize);                            

   
   ippsDotProd_32f(vfft,vfft,winSize,&energy);

   //if (baseInfo.preEmphasize>0.0)
   {
      ippsPreemphasize_32f(vfft,winSize,0.97);
      vfft[0]*=(float)(1.0-0.97);                                   
   }
   
   ippsMul_32f_I(hamWin,vfft,winSize);

   ippsFFTFwd_RToPerm_32f(vfft,vfft,ctxFFT,NULL);
#endif
   return energy;
}
void MFCC2::FinalCompute(float *vfft,float energy,int pitch,float *mfcc) 
{
   FinalCompute(vfft,energy,mfcc);
   if (baseInfo.bPitch) {//add by zliu
	  // mfcc[baseInfo.cepNum+1] =(float)log(pitch);//log to int
	   mfcc[baseInfo.cepNum+1] =pitch/100.0;//log to int
   }
}

void MFCC2::FinalCompute(float *vfft,float energy,float *mfcc) 
{
#ifndef UseIPP
   const float melFloor=1.0;
   float mfnorm,ek; int k,j,i;

   memset(fBank,0,sizeof(float)*baseInfo.chlNum);
   for (k=kLow;k<=kHigh;k++) {
      float t1=vfft[2*k-1-1],t2=vfft[2*k-1];
//      ek=(power)?t1*t1+t2*t2:sqrt(t1*t1+t2*t2);	// changed by lp
      ek=(float)sqrt(t1*t1+t2*t2);
      i=chlMap[k-1]; t1=chlWeight[k-1]*ek;
      if (i>=0) fBank[i]+=t1;
      if (i<baseInfo.chlNum-1) fBank[i+1]+=ek-t1;
   }

   mfnorm=(float)sqrt(2.0/(float)baseInfo.chlNum);
   switch (mfccKind&KINDMASK) {
   case FYT_MFCC:
      for (i=0;i<baseInfo.chlNum;i++) 
         fBank[i]=(float)log(max(fBank[i],melFloor));
      for (j=0;j<baseInfo.cepNum;j++)  {
         float x=(float)((j+1)*PI/(float)baseInfo.chlNum);
         mfcc[j]=0.0f;
         for (k=0;k<baseInfo.chlNum;k++) 
            mfcc[j]+=(float)(fBank[k]*cos(x*(k+0.5)));
         mfcc[j]*=mfnorm;
      }
      break;
   }

   if (baseInfo.cepLifter>0)
      for (i=0;i<baseInfo.cepNum;i++)
         mfcc[i]*=cepWin[i];

   if (mfccKind&HASENERGY)  
	   mfcc[baseInfo.cepNum]=(float)LOGMath2::Log(energy);
#else
   const float melFloor=1.0;
   int j,i;
   float tmp,tmp1;

   ippsZero_32f(fBank,baseInfo.chlNum);
   ippsMagnitude_32fc((Ipp32fc *)(&vfft[2*(kLow-1)]),&ek[kLow-1],kHigh-kLow+1);
 //  if (power)
 //      ippsMul_32f_I(&ek[kLow-1],&ek[kLow-1],kHigh-kLow+1); // ??? sqrt*sqrt
   ippsMul_32f(&ek[kLow-1],&chlWeight[kLow-1],&ek_t1[kLow-1],kHigh-kLow+1);
   ippsSum_32f(&ek[(int)fBankStart[baseInfo.chlNum]],(int)fBankLength[baseInfo.chlNum],&tmp,hint);
   ippsSum_32f(&ek_t1[(int)fBankStart[baseInfo.chlNum]],(int)fBankLength[baseInfo.chlNum],&tmp1,hint);
   fBank[0] = tmp- tmp1;
   for (i=0; i < baseInfo.chlNum-1; i++) {
       ippsSum_32f(&ek_t1[(int)fBankStart[i]],(int)fBankLength[i],&tmp,hint);
       fBank[i] += tmp;
       ippsSum_32f(&ek[(int)fBankStart[i]],(int)fBankLength[i],&tmp1,hint);
       fBank[i+1] += tmp1 - tmp;
   }
   ippsSum_32f(&ek_t1[(int)fBankStart[baseInfo.chlNum-1]],(int)fBankLength[baseInfo.chlNum-1],&tmp,hint);
   fBank[baseInfo.chlNum-1] += tmp;

   switch (mfccKind&KINDMASK) {
   case FYT_MFCC:
       ippsThreshold_32f_I(fBank,baseInfo.chlNum,melFloor,ippCmpLess);
       ippsLn_32f_I(fBank,baseInfo.chlNum);                                              
       for (j=0;j<baseInfo.cepNum;j++)
           ippsDotProd_32f(fBank,&cosTab[j*baseInfo.chlNum],baseInfo.chlNum, &mfcc[j]);            
       ippsMulC_32f_I(mfnorm,mfcc,baseInfo.cepNum);                                        
      break;
  /*
   case FYT_MFCCPLP:
        for (i=0;i<chlNum;i++) 
           fBank[i]=(float)pow(max((float)fBank[i],(float)melFloor),1.0/3.0);
        for (j=0;j<chlNum;j++)
            ippsDotProd_32f(fBank,&cosTab[j*chlNum],chlNum,&mfcc[j]);            
        ippsMulC_32f_I(mfnorm,mfcc,chlNum);                                        
        lpc->ComputeLPC(mfcc,mfccLPCa,chlNum);
        LPC2Cep(mfccLPCa,mfcc);
        for (i=LPC_ORDER;i<cepNum;i++) mfcc[i]=0;
        break;*/
   }
   if (baseInfo.cepLifter>0)
       ippsMul_32f_I(cepWin,mfcc,baseInfo.cepNum);                                       
   if (mfccKind&HASENERGY) mfcc[baseInfo.cepNum]=(float)LOGMath2::Log(energy); 
#endif
}

void MFCC2::NormEnergy()
{
   float maxe,mine,*ft; int i;

   maxe= *(ft=&mfccData[baseInfo.cepNum]);
   for (i=0;i<FrameNum;i++,ft+=BaseDim)
      if (*ft>maxe) maxe= *ft;

   mine=(float)(maxe-(baseInfo.silFloor*log(10.0))/10.0);

   for (i=0,ft=&mfccData[baseInfo.cepNum];i<FrameNum;i++,ft+=BaseDim) 
   {
      if (*ft<mine) *ft=mine;
      *ft=1.0f-(maxe-(*ft))*baseInfo.energyScale;
   }
}

//void MFCC::ZeroCepMean() 
//{
//   int i,j;
//   float sum=0,*ft;
//   for (i=0;i<baseInfo.cepNum;i++) {
//      for (ft=mfccData+i,j=0;j<FrameNum;j++,ft+=BaseDim) sum+=(*ft);
//      sum/=(float)FrameNum;
//      for (ft=mfccData+i,j=0;j<FrameNum;j++,ft+=BaseDim) (*ft)-=sum;
//   }
//}

void MFCC2::WriteFile(char *outFileName)
{
   ASSERT2(mfccData,"error when WriteFile(): mfccData=NULL");
   FILE *fout; int i,k; float *ft;

   short tframeSize=(short)(BaseDim*sizeof(float));
   
   fout=fopen(outFileName,"wb");
   ASSERT3(fout,"error in MFCC : can't open %s to write!",outFileName);

   //fout=fopen(outFile,"wb");
   fwrite(&FrameNum,sizeof(int),1,fout);
   fwrite(&baseInfo.framePeriod,sizeof(int),1,fout);
   fwrite(&tframeSize,sizeof(short),1,fout);
   fwrite(&mfccKind,sizeof(short),1,fout);
   for (i=0,ft=mfccData;i<FrameNum;i++,ft+=BaseDim) 
   {
      for (k=0;k<BaseDim;k++) {
         if (ft[k]>DYN_RANGE) ft[k]=DYN_RANGE;
         if (ft[k]<-DYN_RANGE) ft[k]= -DYN_RANGE;
      }
      fwrite(ft,sizeof(float),BaseDim,fout);
   }
   fclose(fout);
}
float *MFCC2::GetFeatureDate(int *frm,short *tframeSize)
{
   //if (FrameNum < 3) return NULL;
   ASSERT2(mfccData,"error when WriteFile(): mfccData=NULL");

   *frm = FrameNum;
   *tframeSize=(short)(BaseDim*sizeof(float));
   float *ft;
   int i, k;
   for (i=0,ft=mfccData;i<FrameNum;i++,ft+=BaseDim) 
   {
      for (k=0;k<BaseDim;k++) {
         if (ft[k]>DYN_RANGE) ft[k]=DYN_RANGE;
         if (ft[k]<-DYN_RANGE) ft[k]= -DYN_RANGE;
      }
//      fwrite(ft,sizeof(float),BaseDim,fout);
   }
   return mfccData;
}


// bool inverted: if true do IFFT else do FFT
void MFCC2::DoFFT(float *s, int n,const bool inverted) {
   int ii,jj,nn,limit,m,j,inc,i;
   double wx,wr,wpr,wpi,wi,theta;
   double xre,xri,x;

   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j-1]; 
		 xri = s[j + 1-1];
         
		 s[j-1]     = s[i-1];  
		 s[j + 1-1] = s[i + 1-1];
         s[i-1]     = (float)xre; 
		 s[i + 1-1] = (float)xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) 
	  {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = 2.0*PI / (double)limit;
      if (inverted) theta= -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta);
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j-1] - wi * s[j + 1-1];
            xri = wr * s[j + 1-1] + wi * s[j-1];

            s[j-1]     = (float)(s[i-1] - xre); 
			s[j + 1-1] = (float)(s[i + 1-1] - xri);
            s[i-1]     = (float)(s[i-1] + xre); 
			s[i + 1-1] = (float)(s[i + 1-1] + xri);
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (inverted) {
      for (i = 1;i<=n;i++)
         s[i-1] = s[i-1] / nn;
   }
}

// float *s :	input time-domain data
// int	  n :   the number of *s
void MFCC2::RealFFT(float *s,int n) {
   int n2, i, i1, i2, i3, i4;
   double xr1, xi1, xr2, xi2, wrs, wis;
   double yr, yi, yr2, yi2, yr0, theta, x;

   DoFFT(s, n);
   n=n / 2; n2 = n/2;
   theta = PI / n;
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) 
   {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; 
	  wis = yi;
      xr1 = (s[i1-1] + s[i3-1])/2.0; 
	  xi1 = (s[i2-1] - s[i4-1])/2.0;
      xr2 = (s[i2-1] + s[i4-1])/2.0; 
	  xi2 = (s[i3-1] - s[i1-1])/2.0;

      s[i1-1] = (float)(xr1 + wrs * xr2 - wis * xi2);
      s[i2-1] = (float)(xi1 + wrs * xi2 + wis * xr2);
      s[i3-1] = (float)(xr1 - wrs * xr2 + wis * xi2);
      s[i4-1] = (float)(-xi1 + wrs * xi2 + wis * xr2);
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = s[1-1];
   s[1-1] = (float)(xr1 + s[2-1]);
   s[2-1] = 0.f;
}


// 2005.04.22 plu : 
float * MFCC2::GetObv(int fm) 
{ 
    ASSERT2(mfccData,"error:no feature can be used. pls call OpenMfcc and CalNewFeature firstly");
    ASSERT4(fm<=FrameNum,"error: input fm=%d frameNum=%d too big!",fm,FrameNum);
    /*// plu 2004.08.26_17:35:47
    return featureSet+fm*vectSize; 
    */// plu 2004.08.26_17:35:47
	return mfccData+fm*BaseDim;
}