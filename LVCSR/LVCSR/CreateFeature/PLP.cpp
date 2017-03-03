/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : PLP.cpp
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
*D 1.1.0001     2004.05.27     plu        创建文件
*D 1.1.0002     2004.06.30     plu        用PLP的C0代替归一化能量
*D*******************************************************************************/
#include <math.h>
#include <string.h>
#include "plp.h"
//#include "comm.h"


PLP::PLP()
{
	m_chlMap=NULL;
	m_chlWeight=m_hamWin=m_cepWin=m_fBank=NULL;

	m_plpData=NULL;

	m_Auditory=m_AutoCorrelation=m_LP=m_EQL=NULL;
	m_CosineMatrix=NULL;		

	pitch=NULL;//add pitch info by zliu
	pitchTrack=NULL;

	m_bBaseSet = false;
	m_bInitialize=false;		
}

PLP::~PLP()
{
	if (m_chlMap)		delete []m_chlMap;
    if (m_chlWeight)	delete []m_chlWeight;
	if (m_hamWin)		delete []m_hamWin;
	if (m_cepWin)		delete []m_cepWin;
	if (m_fBank)		delete []m_fBank;
    
	if (m_plpData)		delete []m_plpData;

	if (m_Auditory)			delete []m_Auditory;
	if (m_AutoCorrelation)	delete []m_AutoCorrelation;	
	if (m_LP)				delete []m_LP;
	if (m_EQL)				delete []m_EQL;
	if (m_CosineMatrix)		
	{
		// plu 2004.05.28_21:39:30
		char *tmp=(char *)m_CosineMatrix;
		delete []tmp;
		// plu 2004.05.28_21:39:30
	}
	
	if(pitch)	delete []pitch;//add pitch info by zliu
	if(pitchTrack) delete pitchTrack;
	if(m_ffts) delete []m_ffts;   // 070812 zliu remind
	if(m_cepCoef) delete []m_cepCoef;
}


void PLP::WriteFile(char *outFile)
{
   //ASSERT1(m_plpData,"error when WriteFile(): m_plpData=NULL");
	if (!m_plpData)
	{
		printf("error when WriteFile(): m_plpData=NULL");	return;
	}
   FILE *fout; int i,k; float *ft;

   short tframeSize=(short)(m_BaseDim*sizeof(float));

   fout=fopen(outFile,"wb");
   ASSERT3(fout,"Error open %s for write.",outFile);

   fwrite(&m_FrameNum,sizeof(int),1,fout);
   fwrite(&m_baseInfo.framePeriod,sizeof(int),1,fout);
   fwrite(&tframeSize,sizeof(short),1,fout);
   fwrite(&m_plpKind,sizeof(short),1,fout);
   for (i=0,ft=m_plpData;i<m_FrameNum;i++,ft+=m_BaseDim) 
   {
      for (k=0;k<m_BaseDim;k++) 
	  {
		  if (k = m_BaseDim-1) {

		  }else{
			  ft[k] *= 10.f;	// 2004.11.04 plu : plp参数的量级太小，加倍之
				if (ft[k]>DYN_RANGE) ft[k]=DYN_RANGE;
				if (ft[k]<-DYN_RANGE) ft[k]= -DYN_RANGE;
		  }
      }
      fwrite(ft,sizeof(float),m_BaseDim,fout);
   }
   fclose(fout);
}
float *PLP::GetFeatureDate(int *frm,short *tframeSize)
{
   //if (m_FrameNum < 3) return NULL;
   //ASSERT2(m_plpData,"error when WriteFile(): m_plpData=NULL");
   *frm = m_FrameNum;

   *tframeSize=(short)(m_BaseDim*sizeof(float));
   float *ft;
   int i, k;
   for (i=0,ft=m_plpData;i<m_FrameNum;i++,ft+=m_BaseDim) 
   {
      for (k=0;k<m_BaseDim;k++) 
	  {
         if (ft[k]>DYN_RANGE) ft[k]=DYN_RANGE;
         if (ft[k]<-DYN_RANGE) ft[k]= -DYN_RANGE;
      }
//      fwrite(ft,sizeof(float),BaseDim,fout);
   }
   return m_plpData;
}

//void PLP::SetBaseInfo(PLP_BASEINFO p_info)
void PLP::SetBaseInfo(FEATURE_BASEINFO p_info)
{
    m_plpKind=0;
    if (strstr(p_info.targetKind,"MFCCPLP"))	
	{
		m_plpKind=FYT_MFCCPLP;
		if (strstr(p_info.targetKind,"_C0"))		m_plpKind|=HASENERGY;
	}
	else
	{
		printf("Error set targetkind in MFCCPLP!\n");	exit(-1);		
	}
    ASSERT3(m_plpKind&KINDMASK,"Unsupported target kind: %s",p_info.targetKind);

    m_baseInfo.smpPeriod   = p_info.smpPeriod;
    m_baseInfo.framePeriod = p_info.framePeriod;

    m_baseInfo.lowPass  = p_info.lowPass;
    m_baseInfo.highPass = p_info.highPass;
    m_baseInfo.winSize  = p_info.winSize;
    m_baseInfo.chlNum   = p_info.chlNum;

    m_baseInfo.cepNum   = p_info.cepNum;
	m_baseInfo.lpcOrder = p_info.lpcOrder;
    m_baseInfo.cepLifter = p_info.cepLifter;    
	m_baseInfo.compressFact = p_info.compressFact;

    m_baseInfo.zeroGlobalMean = p_info.zeroGlobalMean;
    /*// plu 2004.06.30_17:01:49
    if (m_plpKind&HASENERGY) 
		m_baseInfo.normEnergy = true;
    else
		m_baseInfo.normEnergy = false;

    if (m_baseInfo.normEnergy)
    {
		m_baseInfo.energyScale = p_info.energyScale;
		m_baseInfo.silFloor    = p_info.silFloor;
    }
    */// plu 2004.06.30_17:01:49

	if (m_baseInfo.cepNum < 2 || m_baseInfo.cepNum > m_baseInfo.lpcOrder)
         WARNING2("ValidCodeParms: unlikely num cep coef %d",m_baseInfo.cepNum);
 
    m_bBaseSet=true;
	// add by zliu
	m_baseInfo.bPitch = p_info.bPitch;
}


void PLP::Initialize(int &BaseDim) 
{
	ASSERT2(m_bBaseSet,"pls call SetBaseInfo() firstly!");

	m_frameRate = m_baseInfo.framePeriod/m_baseInfo.smpPeriod;
	m_winSize   = m_baseInfo.winSize/m_baseInfo.smpPeriod;

	m_FFTNum=2;    
	while (m_FFTNum < m_winSize) m_FFTNum*=2;

	m_BaseDim=(m_plpKind&HASENERGY)?(m_baseInfo.cepNum+1):m_baseInfo.cepNum;
   //add by zliu 
   if (m_baseInfo.bPitch) {
	   m_BaseDim += 1; //add pitch
   }
   BaseDim = m_BaseDim;
   
	// cep lifter
	if (m_baseInfo.cepLifter>0) 
	{
		CreateVector(&m_cepWin,m_baseInfo.cepNum);
		ASSERT2(m_cepWin,"error alloc space for m_cepWin");

		float a=(float)(PI/(float)m_baseInfo.cepLifter);
		for (int i=1;i<=m_baseInfo.cepNum;i++)
			m_cepWin[i]=(float)(1.f+(float)m_baseInfo.cepLifter/2.0*sin( i * a));

		cepWinL = m_baseInfo.cepLifter;
		cepWinSize = m_baseInfo.cepNum;
	}

   // ham windows
	CreateVector(&m_hamWin,m_winSize);
	double a = TPI /(double)(m_winSize - 1);
	for (int i=1;i<=m_winSize;i++)
		m_hamWin[i] =(float)( 0.54 - 0.46 * cos(a*(i-1)));

	InitFBankAndPLP();
   //add by zliu
	if (m_baseInfo.bPitch) 
		pitchTrack = new PitchTrack(m_baseInfo.smpPeriod, m_baseInfo.winSize, m_baseInfo.framePeriod);
	
	m_bInitialize=true;
}

void PLP::InitFBankAndPLP() 
{
	//////////////////////////////////////////////////////
	//		init freq bank
	float mlow,mhigh,mwidth,*bnkCentre;
	int k;

	m_fres=(float)(1.0e7/(m_baseInfo.smpPeriod*m_FFTNum*700.0));
	m_kLow=2; 
	mlow=0;
	if (m_baseInfo.lowPass>=0.0) 
	{
		mlow=(float)(1127*log(1.0+m_baseInfo.lowPass/700.0));
		m_kLow=(int)((m_baseInfo.lowPass*m_baseInfo.smpPeriod*(1.0e-7)*m_FFTNum)+0.5);
		//HTK
		//m_kLow=(int)((m_baseInfo.lowPass*m_baseInfo.smpPeriod*(1.0e-7)*m_FFTNum)+2.5);

		if (m_kLow<2) m_kLow=2;
	}

	m_kHigh=m_FFTNum/2; 
	mhigh=Mel(m_FFTNum/2+1);
	if (m_baseInfo.highPass>=0.0) 
	{
		mhigh=(float)(1127*log(1.0+m_baseInfo.highPass/700.0));
		m_kHigh=(int)((m_baseInfo.highPass*m_baseInfo.smpPeriod*(1.0e-7)*m_FFTNum)+0.5);

		if (m_kHigh>m_FFTNum/2) m_kHigh=m_FFTNum/2;
	}

	//////////////////////
	int maxChan = m_baseInfo.chlNum+1;
	/* Create vector of fbank centre frequencies */
	CreateVector(&bnkCentre,maxChan);
	mwidth=mhigh-mlow;
	int chan=1;
	for (chan=1; chan <= maxChan; chan++) 
		 bnkCentre[chan] = ((float)chan/(float)maxChan)*mwidth + mlow;

	/* Create loChan map, loChan[fftindex] -> lower channel index */	// m_chlMap ->  fb.loChan
	float melk;
	CreateShortVec(&m_chlMap,m_FFTNum/2);
	for (k=1,chan=1; k<=m_FFTNum/2; k++)
	{
		 melk = Mel(k);
      if (k<m_kLow || k>m_kHigh) m_chlMap[k]=-1;
      else {
         while (bnkCentre[chan] < melk  && chan<=maxChan) ++chan;
         m_chlMap[k] = chan-1;
      }
		 /*// plu 2004.05.30_14:18:09
		 while (bnkCentre[chan] < melk  && chan<=maxChan) ++chan;
		 m_chlMap[k] = chan-1;
		 */// plu 2004.05.30_14:18:09
	}

	/* Create vector of lower channel weights */   // m_chlWeight -> fb.loWt
	CreateVector(&m_chlWeight,m_FFTNum/2);
	for (k=1; k<=m_FFTNum/2; k++) 
	{
		chan = m_chlMap[k];
		if (k<m_kLow || k>m_kHigh) m_chlWeight[k]=0.0;
		else {
		 if (chan>0) 
			m_chlWeight[k] = ((bnkCentre[chan+1] - Mel(k)) / (bnkCentre[chan+1] - bnkCentre[chan]));
		 else
			m_chlWeight[k] = (bnkCentre[1]-Mel(k))/(bnkCentre[1] - mlow);
		}

		/*// plu 2004.05.30_14:28:26
		chan = m_chlMap[k];
		if (chan>0) 
		{
			float bnki1=bnkCentre[chan+1];
			m_chlWeight[k] = ((bnki1 - Mel(k)) / (bnki1 - bnkCentre[chan]));
		}
		else
		{
			float bnk0=bnkCentre[1];
			m_chlWeight[k] = (bnk0-Mel(k))/(bnk0 - mlow);
		}
		*/// plu 2004.05.30_14:28:26
	}
	
	CreateVector(&m_fBank,m_baseInfo.chlNum);	
	CreateVector(&m_ffts,m_FFTNum);

	//////////////////////////////////////////////////////////////////
	//		init PLP
	int i,j;
	double baseAngle;
	float f_hz_mid, fsub, fsq;
	int  nAuto, nFreq;

   // alloc space
	CreateVector(&m_cepCoef,m_baseInfo.cepNum+1);
	CreateVector(&m_Auditory,m_baseInfo.chlNum + 2);
	CreateVector(&m_EQL,m_baseInfo.chlNum);
	CreateVector(&m_AutoCorrelation,m_baseInfo.lpcOrder + 1);
	CreateVector(&m_LP,m_baseInfo.lpcOrder + 1);
	/*// plu 2004.07.18_15:37:23
	CreateDMatrix(&m_CosineMatrix,m_baseInfo.lpcOrder+1,m_baseInfo.chlNum+2);
	*/// plu 2004.07.18_15:37:23
	m_CosineMatrix = CreateDMatrix(m_baseInfo.lpcOrder+1,m_baseInfo.chlNum+2);

	for (i=1; i<=m_baseInfo.chlNum; i++) 
	{
		f_hz_mid = 700*(exp(bnkCentre[i]/1127)-1); /* Mel to Hz conversion */
		fsq = (f_hz_mid * f_hz_mid);
		fsub = fsq / (fsq + 1.6e5);
		m_EQL[i] = fsub * fsub * ((fsq + 1.44e6)  /(fsq + 9.61e6));
   }

   /* Builds up matrix of cosines for IDFT */
   nAuto = m_baseInfo.lpcOrder+1; 
   nFreq = m_baseInfo.chlNum+2;
   baseAngle =  PI / (double)(nFreq - 1);
   for (i=0; i<nAuto; i++) 
   {
      m_CosineMatrix[i+1][1] = 1.0;
      for (j=1; j<(nFreq-1); j++)
         m_CosineMatrix[i+1][j+1] = 2.0 * cos(baseAngle * (double)i * (double)j);

      m_CosineMatrix[i+1][nFreq] = cos(baseAngle * (double)i * (double)(nFreq-1));
   }

	delete []bnkCentre;
}


/******************************FUNCTION*COMMENT***********************************
*F
*F 函数名称            : PLP::AddWaveData
*F
*F 函数类型            : void
*F
*F 函数返回值          :
*F     无返回值
*F
*F 函数参数            :
*F     short *waveData
*F     int smpNum
*F
*F 函数描述            :
*F
*F
*F 函数修改记录
*F ------------------------------------------------------------------------------ 
*F 修改日期       修改人     改动内容
*F ------------------------------------------------------------------------------ 
*F 2004.05.28     plu        创建函数
*F*******************************************************************************/
void PLP::AddWaveData(short *waveData,int smpNum,int *residue)
{
    ASSERT2(m_bInitialize,"pls call Initialize() firstly!");
    ASSERT2(waveData,"error in AddWaveData(): input wavedata=NULL!");
    ASSERT2(smpNum>0,"error in AddWaveData(): input smpNum <= 0!");
    
    m_FrameNum=(smpNum-m_winSize)/m_frameRate+1; // 只少不多
	if (residue) *residue = smpNum-(m_FrameNum-1)*m_frameRate-m_winSize;
	if (m_FrameNum < 3) {m_FrameNum=0;return;}   // 080519 jgao found bug of feat less than 3 frames
 	if (m_baseInfo.bPitch){
		pitch=NULL;
		short *tmpwav = new short [smpNum];
		
//		for(int i=0; i < smpNum; i++){
//			tmpwav[i]=waveData[i];
//		}	
		memcpy(tmpwav,waveData,sizeof(short)*smpNum);
	//	short *tmpwav=waveData;
		pitchTrack->DoUtterance(tmpwav,m_FrameNum, &pitch);
		delete []tmpwav;
	}
	
	int i;
     
	// remove time-domain mean
    if (m_baseInfo.zeroGlobalMean) ZeroGlobalMean(waveData,smpNum);
  
	float *pCurFrame,energy;
	short *pCurWave;

	if (m_plpData)	
	{
		delete []m_plpData;	m_plpData=NULL;   // [6/26/2007 added by syq] make mem to be controled by user (so could be used when decoding so long)
	}

	if (m_FrameNum>0)
	{
		m_plpData=new float[m_FrameNum*m_BaseDim];
		
		//ASSERT1(m_plpData!=NULL,"error alloc space for m_plpData");
		if (!m_plpData)
		{
			printf("error alloc space for m_plpData");
			return;
		}
	}

    pCurFrame = m_plpData;	
	pCurWave  = waveData;
    for (i=0; i<m_FrameNum; i++)
    {
//		printf("frame = %d :\r",i);   syq 070628 add
        energy=ApplyFFT(pCurWave,m_ffts);
		ASSERT3(energy>=0,"log(%g)",energy);

        ConvertFrame();
//		changed by zliu
//		for (int j=0;j<m_BaseDim;j++)
//			*pCurFrame++ = m_cepCoef[j+1];
		if (m_baseInfo.bPitch) {
			for (int j=0;j<m_BaseDim-1;j++)
				*pCurFrame++ = m_cepCoef[j+1];
			//*pCurFrame++ = (float)log(pitch[i]);
			*pCurFrame++ = pitch[i]/100.0;
		}else{
			for (int j=0;j<m_BaseDim;j++)
				*pCurFrame++ = m_cepCoef[j+1];
		}


		/*// plu 2004.05.31_17:55:03
		for (int j=0;j<m_baseInfo.cepNum;j++)
		{
			*pCurFrame++ = m_cepCoef[j+1];
			printf("[%2d] %.3f\n",j,m_cepCoef[j+1]);
		}
		printf("[%2d] %.3f\n",j,m_cepCoef[j+1]);
		if (m_plpKind&HASENERGY)  
			*pCurFrame++=(float)(energy<=MINLARG)?LOGZERO:log(energy);
		*/// plu 2004.05.31_17:55:03

		pCurWave += m_frameRate;
    }

    /*// plu 2004.06.30_16:04:48
    if ( (m_plpKind&HASENERGY) && m_baseInfo.normEnergy )         
		NormEnergy();
    */// plu 2004.06.30_16:04:48

//    printf("%d, %d\n", m_BaseDim,m_FrameNum);   syq 070628 add
}

/******************************FUNCTION*COMMENT***********************************
*F
*F 函数名称            : PLP::ConvertFrame
*F
*F 函数类型            : void
*F
*F 函数返回值          :
*F     无返回值
*F
*F 函数参数            :
*F     float *vfft
*F     float energy
*F     float *plpData
*F
*F 函数描述            :
*F
*F
*F 函数修改记录
*F ------------------------------------------------------------------------------ 
*F 修改日期       修改人     改动内容
*F ------------------------------------------------------------------------------ 
*F 2004.05.28     plu        创建函数
*F*******************************************************************************/
void PLP::ConvertFrame() 
{
	const float melFloor=1.0;

	// EXPORT-> Pre-emphasise filter bank output with the simulated 
    // equal-loudness curve and perform amplitude compression 
	const float melfloor = 1.0;
	int i;
	for (i=1; i<=m_baseInfo.chlNum; i++) 
	{
		if (m_fBank[i] < melFloor) m_fBank[i] = melFloor;

		m_Auditory[i+1] = m_fBank[i] * m_EQL[i]; /* Apply equal-loudness curve */
		m_Auditory[i+1] = pow((double) m_Auditory[i+1], (double) m_baseInfo.compressFact);
	}
	m_Auditory[1] = m_Auditory[2];  // Duplicate values at either end 
	m_Auditory[m_baseInfo.chlNum+2] = m_Auditory[m_baseInfo.chlNum+1];

	/* EXPORT->ASpec2LPCep: Perform IDFT to get autocorrelation values then 
           produce autoregressive coeffs. and cepstral transform them */
	float lpcGain, E;

	/* Do IDFT to get autocorrelation values */
	E = MatrixIDFT(m_Auditory, m_AutoCorrelation, m_CosineMatrix);
	
	m_LP[VectorSize(m_LP)] = 0.0;    /* init to make Purify et al. happy */

	/* do Durbin recursion to get predictor coefficients */
	lpcGain = Durbin(NULL,m_LP,m_AutoCorrelation,E,m_baseInfo.lpcOrder);
	if (lpcGain<=0) 
		WARNING1("ASpec2LPCep: Negative lpcgain");

	LPC2Cepstrum(m_LP,m_cepCoef);

	/* value forms C0 */
	m_cepCoef[VectorSize(m_cepCoef)] = (float) -log((double) 1.0/lpcGain); 

	if (m_baseInfo.cepLifter>0)
		 WeightCepstrum(m_cepCoef, 1, m_baseInfo.cepNum, m_baseInfo.cepLifter);	
}

void PLP::ZeroGlobalMean(short *data,int smpCount) 
{
	double mean=0.0; int i;
	for (i=0;i<smpCount;i++) 
		mean+=data[i];

	mean/=(double)smpCount;

	for (i=0;i<smpCount;i++) 
	{
		double y=(float)data[i]-mean;
		if (y>32767) y=32767;
		if (y<-32767) y= -32767;

		data[i] = (short)((y>0)?y + 0.5:y-0.5);
	}
}

/*// plu 2004.06.30_16:05:59
void PLP::NormEnergy()
{
   float maxe,mine,*ft; int i;

   maxe= *(ft=&m_plpData[m_baseInfo.cepNum]);
   for (i=0;i<m_FrameNum;i++,ft+=m_BaseDim)
      if (*ft>maxe) maxe= *ft;

   mine=(float)(maxe-(m_baseInfo.silFloor*log(10.0))/10.0);

   for (i=0,ft=&m_plpData[m_baseInfo.cepNum];i<m_FrameNum;i++,ft+=m_BaseDim) 
   {
      if (*ft<mine) *ft=mine;
      *ft=1.0f-(maxe-(*ft))*m_baseInfo.energyScale;
   }
}
*/// plu 2004.06.30_16:05:59

/////////////////////////////////////////////////////////////////////////////////
//			math calculation
/////////////////////////////////////////////////////////////////////////////////

// short *wave: input wave data
// float *vfft: output fft data
float PLP::ApplyFFT(short *wave,float *vfft)                   
{
	int k,j,i;
	float energy=0.0;

	for (i=0;i<m_winSize;i++) vfft[i+1]=(float)wave[i];
	for (i=m_winSize;i<m_FFTNum;i++) vfft[i+1]=0;

	/*// plu 2004.06.30_15:18:56
	// 去除帧内时域均值
	if (m_baseInfo.zeroFrameMean && ! m_baseInfo.zeroGlobalMean) 
	{
		float sum=0.0;
		for (i=0;i<m_winSize;i++) sum+=vfft[i+1];
		sum/=m_winSize;

		for (i=0;i<m_winSize;i++) vfft[i+1]-=sum;
	}
	*/// plu 2004.06.30_15:18:56

	// 计算时域能量
	for (i=0;i<m_winSize;i++)	energy+=vfft[i+1]*vfft[i+1];

	// 预加重
	for (i=m_winSize-1;i>0;i--)	vfft[i+1] -= vfft[i]*0.97f;
	vfft[1] *= 0.03f;

	// ham window
	for (i=0;i<m_winSize;i++)	vfft[i+1] *= m_hamWin[i+1];

	Realft(vfft);

	float ek,t1,t2;

	// Fill filterbank channels 
	ZeroVector(m_fBank); 
	for (k = m_kLow; k <= m_kHigh; k++) {             /* fill bins */
		t1 = vfft[2*k-1]; t2 = vfft[2*k];
		/*// plu 2004.05.28_20:11:08
		if (info.usePower)
		 ek = t1*t1 + t2*t2;
		else
		*/// plu 2004.05.28_20:11:08
		ek = sqrt(t1*t1 + t2*t2);
		
		i = m_chlMap[k];
		t1 = m_chlWeight[k]*ek;
		if (i>0) m_fBank[i] += t1;
		if (i<m_baseInfo.chlNum) m_fBank[i+1] += ek - t1;
	}

	return energy;
}


/* Matrix IDFT converts from auditory spectrum into autocorrelation values */
float PLP::MatrixIDFT(float * as, float * ac, double ** cm)
{
   double acc;
   float E;
   int nAuto, nFreq;
   int i, j;

   nFreq = VectorSize(as);
   nAuto = VectorSize(ac);

   for (i=0; i<nAuto; i++) {
      acc = cm[i+1][1] * (double)as[1];
      for (j=1; j<nFreq; j++)
         acc += cm[i+1][j+1] * (double)as[j+1];

      if (i>0) 
         ac[i] = (float)(acc / (double)(2.0 * (nFreq-1)));
      else  
         E = (float)(acc / (double)(2.0 * (nFreq-1)));
   }     
   return E; /* Return zero'th auto value separately */
}


/* Durbins recursion to get LP coeffs for auto values */
float PLP::Durbin(float * k, float * thisA, float * r, float E, int order)
{
   float *newA;
   float ki;         /* Current Reflection Coefficient */
   int i,j;
 
   CreateVector(&newA,order);
   for (i=1;i<=order;i++) {
      ki = r[i];              /* Calc next reflection coef */
      for (j=1;j<i;j++)
         ki = ki + thisA[j] * r[i - j];
      ki = ki / E;   
      if (k!=NULL) k[i] = ki;
      E *= 1 - ki*ki;         /* Update Error */
      newA[i] = -ki;          /* Calc new filter coef */
      for (j=1;j<i;j++)
         newA[j] = thisA[j] - ki * thisA[i - j];
      for (j=1;j<=i;j++)   
         thisA[j] = newA[j];
   }
   delete []newA;
   return (E);
}

/* EXPORT->LPC2Cepstrum: transfer from lpc to cepstral coef */
void PLP::LPC2Cepstrum (float * a, float * c)
{
   int i,n,p;
   float sum;
   
   p=VectorSize(c);
   for (n=1;n<=p;n++)  { 
      sum = 0.0;
      for (i=1;i<n;i++) 
         sum = sum + (n - i) * a[i] * c[n - i];
      c[n] = -(a[n] + sum / n);
   }
}

/* EXPORT->WeightCepstrum: Apply cepstral weighting to c */
void PLP::WeightCepstrum (float * c, int start, int count, int cepLiftering)
{
   int i,j;
   j = start;
   for (i=1;i<=count;i++)     c[j++] *= m_cepWin[i];
}

/* EXPORT-> FFT: apply fft/invfft to complex s */
/*
   When called s holds nn complex values stored in the
   sequence   [ r1 , i1 , r2 , i2 , .. .. , rn , in ] where
   n = VectorSize(s) DIV 2, n must be a power of 2. On exit s
   holds the fft (or the inverse fft if invert == 1) 
*/
void PLP::FFT(float * s, int invert)
{
   int ii,jj,n,nn,limit,m,j,inc,i;
   double wx,wr,wpr,wpi,wi,theta;
   double xre,xri,x;
   
   n=VectorSize(s);
   nn=n / 2; j = 1;
   for (ii=1;ii<=nn;ii++) {
      i = 2 * ii - 1;
      if (j>i) {
         xre = s[j]; xri = s[j + 1];
         s[j] = s[i];  s[j + 1] = s[i + 1];
         s[i] = xre; s[i + 1] = xri;
      }
      m = n / 2;
      while (m >= 2  && j > m) {
         j -= m; m /= 2;
      }
      j += m;
   };
   limit = 2;
   while (limit < n) {
      inc = 2 * limit; theta = TPI / limit;
      if (invert) theta = -theta;
      x = sin(0.5 * theta);
      wpr = -2.0 * x * x; wpi = sin(theta); 
      wr = 1.0; wi = 0.0;
      for (ii=1; ii<=limit/2; ii++) {
         m = 2 * ii - 1;
         for (jj = 0; jj<=(n - m) / inc;jj++) {
            i = m + jj * inc;
            j = i + limit;
            xre = wr * s[j] - wi * s[j + 1];
            xri = wr * s[j + 1] + wi * s[j];
            s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
            s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
         }
         wx = wr;
         wr = wr * wpr - wi * wpi + wr;
         wi = wi * wpr + wx * wpi + wi;
      }
      limit = inc;
   }
   if (invert)
      for (i = 1;i<=n;i++) 
         s[i] = s[i] / nn;
}

/* EXPORT-> Realft: apply fft to real s */
/*
   When called s holds 2*n real values, on exit s holds the
   first  n complex points of the spectrum stored in
   the same format as for fft
*/
void PLP::Realft (float * s)
{
   int n, n2, i, i1, i2, i3, i4;
   double xr1, xi1, xr2, xi2, wrs, wis;
   double yr, yi, yr2, yi2, yr0, theta, x;

   n=VectorSize(s) / 2; n2 = n/2;
   theta = PI / n;
   FFT(s, 0);
   x = sin(0.5 * theta);
   yr2 = -2.0 * x * x;
   yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
   for (i=2; i<=n2; i++) {
      i1 = i + i - 1;      i2 = i1 + 1;
      i3 = n + n + 3 - i2; i4 = i3 + 1;
      wrs = yr; wis = yi;
      xr1 = (s[i1] + s[i3])/2.0; xi1 = (s[i2] - s[i4])/2.0;
      xr2 = (s[i2] + s[i4])/2.0; xi2 = (s[i3] - s[i1])/2.0;
      s[i1] = xr1 + wrs * xr2 - wis * xi2;
      s[i2] = xi1 + wrs * xi2 + wis * xr2;
      s[i3] = xr1 - wrs * xr2 + wis * xi2;
      s[i4] = -xi1 + wrs * xi2 + wis * xr2;
      yr0 = yr;
      yr = yr * yr2 - yi  * yi2 + yr;
      yi = yi * yr2 + yr0 * yi2 + yi;
   }
   xr1 = s[1];
   s[1] = xr1 + s[2];
   s[2] = 0.0;
}


size_t PLP::MRound(size_t size)
{
   return ((size % FWORD) == 0)?size : (size/FWORD + 1) * FWORD;
}

DMatrix PLP::CreateDMatrix(int nrows,int ncols)
{
   size_t vsize;
   int *i,j;
   DVector *m;   
   char *p;
   
   size_t nAllocSize = (ncols+1)*sizeof(double)*nrows+(nrows+1)*sizeof(DVector);

   p = new char[MRound(nAllocSize)];
   ASSERT2(p,"Error new space in CreateDMatrix()");
   i = (int *) p; *i = nrows;
   //vsize = DVectorElemSize(ncols);
   vsize = (ncols+1)*sizeof(double);
   m = (DVector *) p;
   p += MRound((nrows+1)*sizeof(DVector));
   for (j=1; j<=nrows; j++, p += vsize) {
      i = (int *) p; *i = ncols;
      m[j] = (DVector) p;
   }
   return m;
}

/*// plu 2004.07.18_15:37:06
void PLP::CreateDMatrix(double ***x,int nrows,int ncols)
{
	size_t vsize;
	int *i,j;
	DVector *m;   
	char *p;
   
	size_t nAllocSize1 = (ncols+1)*nrows + (nrows+1)*sizeof(DVector);
	p = new char[MRound(nAllocSize1)];
	ASSERT1(p,"Error new space in CreateDMatrix()");
	i = (int *) p; *i = nrows;
	vsize = sizeof(double)*(ncols+1);
	m = (DVector *) p;

	p += MRound((nrows+1)*sizeof(DVector));
	for (j=1; j<=nrows; j++, p += vsize) 
	{
		i = (int *) p; *i = ncols;
		m[j] = (DVector) p;
	}

	*x=m;
	return;
}
*/// plu 2004.07.18_15:37:06

void  PLP::CreateShortVec(short **x,int size)
{   
   *x = new short[size+1];
   ASSERT2(*x,"Error new short in CreateShortVec");   // 070630 syq from x
   **x = (short)size;
}


void PLP::CreateVector(float **buf, int size)
{
	int *i;

	*buf = new float[size+1];
	ASSERT2(*buf,"Error new in CreateVector");   // 070630 syq from buf

	i = (int *)(*buf); 
	*i = size;
	return;
}

int PLP::VectorSize(float *vector)
{
	int *i;

	i = (int *) vector;
	return *i;
}

void PLP::ZeroVector(float* v)
{
   int i,n;
   
   n=VectorSize(v);
   for (i=1;i<=n;i++) v[i]=0.0;
}
