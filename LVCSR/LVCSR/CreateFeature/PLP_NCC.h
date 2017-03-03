/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : PLP.H
*D
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D*******************************************************************************/
#ifndef _HTK_PLP_NCC_H_
#define _HTK_PLP_NCC_H_

#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include "comm.h"
#include "PitchTracker.h"

//#define UseIPP
//#define WriteWholeFeat	// write whole features to file, otherwise is slow in multi-process   [4/26/2013 Xuyang Wang]
#include "ipps.h"
#ifdef UseIPP
#include "ipps.h"	// add for ipp [4/10/2013 Xuyang Wang]
#include "mkl.h"
#include "omp.h"
#endif

///////////////////////////////////////////////////////////////////////////////////////
#ifndef		PI
#define		PI	3.14159265358979
#endif
#define TPI				6.28318530717959     /* PI*2 */
#define LOGZERO         (-1.0e+10)
#define LSMALL			(-0.5E10)		/* log values < LSMALL are set to LZERO */
#define MINEARG			(-708.3)		/* lowest exp() arg  = log(MINLARG) */
#define MINLARG			2.45E-308		/* lowest log() arg  = exp(MINEARG) */


#define FWORD 8   /* size of a full word = basic alignment quanta */

typedef double *DVector;   /* double vector[1..size]   */
typedef double **DMatrix;  /* double matrix[1..nrows][1..ncols] */


struct FeatHead_ncc
{
	int nFrameNum;
	int nFramePeriod;
	short nFrameSize;
	short plpKind;
	/*
	FeatHead()
	{
	nFrameNum = 0;
	nFramePeriod = 0;
	nFrameSize = 0;
	plpKind = FYT_MFCCPLP;
	}*/

};



/*********************************CLASS*COMMENT***********************************
*C
*C 类名称              : PLP
*C
*C 类描述              : 
*C
*C
*C 类修改记录
*C ------------------------------------------------------------------------------ 
*C 修改日期       修改人     改动内容
*C ------------------------------------------------------------------------------ 
*C 2004.05.27     plu        创建类
*C*******************************************************************************/
class PLP_NCC
{
protected:
	bool m_bBaseSet,m_bInitialize;		

	FEATURE_BASEINFO m_baseInfo;

	short m_plpKind;
	int   m_BaseDim,m_FFTNum,m_frameRate,m_FrameNum;

	// plp 
	float *m_Auditory,*m_AutoCorrelation,*m_LP;		// auditory,autocorrelation and lp vector for plp
	float *m_EQL;					// equal loudness curve
	double **m_CosineMatrix;		// cosine matrix for IDFT

	// used in FFT
	int   m_winSize,m_kLow,m_kHigh;
	short *m_chlMap;
	float m_fres,*m_chlWeight,*m_hamWin,*m_fBank;

	// used in ccepstrum lifter
	float *m_cepWin;

	int cepWinSize;            /* Size of current cepstral weight window */
	int cepWinL;               /* Current liftering coeff */

	float *m_cepCoef;
	float *m_ffts;

	float *m_plpData;				// final plp feature

	  PitchTrack *pitchTrack;//add pitch inf by zliu
	  int *pitch;
	  float *voiced_Degree; // add float *voicedDegree by qqzhang
	double *Matrix;
	int matrix_h,matrix_n;	 

    void* cmslen; 
	float *iniMean, *iniVar;
    float *newMean, *newVar;//add by llu

	char *HLDAMatrixDir;
	//for ippuse
#ifdef UseIPP
	int nPLPOrder;
	float *vfft1;// CCS format has m_WinSize+2 data [4/10/2013 Xuyang Wang]
	IppsFFTSpec_R_32f* ctxFFT;
	//IppsDFTSpec_R_32f* ctxDFT;
#endif

	////////////////////////////////////////////////////////////////////
	float Mel(int k) 
	{ 
		/*// plu 2004.05.30_14:16:38
		return (float)(1127.f*log(1.0+(float)k*m_fres)); 
		*/// plu 2004.05.30_14:16:38
	   return (float)(1127.f * log(1.f + (k-1)*m_fres));
	}

	void  ZeroGlobalMean(short *data,int smpCount);
	//void  InitFBankAndPLP();
	void  InitFBankAndPLP(float p_warpAlpha);
	float ApplyFFT(short *wave,float *vfft);
	void  ConvertFrame();

	void  Realft(float *s);
	void  FFT(float *s,int inverted=0);

	float MatrixIDFT(float * as, float * ac, double ** cm);
	float Durbin(float * k, float * thisA, float * r, float E, int order);
	void  LPC2Cepstrum (float * a, float * c);
	void  WeightCepstrum (float * c, int start, int count, int cepLiftering);

	void CreateVector(float **buf, int size);
	int  VectorSize(float *vector);
	void CreateShortVec(short **x,int size);
	/*// plu 2004.07.18_15:36:24
	void CreateDMatrix(double ***,int nrows,int ncols);
	*/// plu 2004.07.18_15:36:24
	DMatrix CreateDMatrix(int nrows,int ncols);
	void ZeroVector(float* v);
	size_t MRound(size_t size);
float WarpFreq (float fcl, float fcu, float freq, float minFreq, float maxFreq , float alpha); //qqzhang 20120305


public:
    
	PLP_NCC();
	~PLP_NCC();

	//void	SetBaseInfo(PLP_BASEINFO p_info);
	void SetBaseInfo(FEATURE_BASEINFO p_info);
	void SetHLDAMatrixDir(char* matrixDir);
	virtual void Initialize(int &BaseDim);

	virtual void AddWaveData(short *data,int p_smpNum, bool initPitch = true);
	virtual void WriteFile(char *filename);

	virtual void WriteBaseFeatFile(char *filename);
	virtual float *GetFeatureDate(int *frm,short *tframeSize);
	virtual void WriteBuffer(char * buffer,int *length);

};

#endif // _HTK_PLP_H_