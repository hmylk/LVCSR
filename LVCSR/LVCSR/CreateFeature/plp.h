/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : PLP.H
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
#ifndef _HTK_PLP_H_
#define _HTK_PLP_H_

#include <stdio.h>
//#include <conio.h>
#include <stdlib.h>
#include <math.h>
#include "comm.h"
#include "PitchTracker.h"

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

/*// plu 2004.06.30_15:15:13
struct PLP_BASEINFO	    // basic information needed by MFCC class
{
    char  targetKind[256];
    int   smpPeriod,framePeriod;			// 采样周期，帧长
    int   chlNum,cepNum;					// 滤波器数目（通道数），倒谱数
	int	  cepLifter,winSize;				// 倒谱提升，窗长
    float lowPass,highPass;					// 低/高截至频率
    float energyScale,silFloor;				// 能量归一化，Silence floor in dBs
	int   lpcOrder;							// LPC阶数
	float compressFact;						// Compression Factor fo PLP 

	bool  zeroGlobalMean,zeroFrameMean,normEnergy;
};
*/// plu 2004.06.30_15:15:13

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
class PLP 
{
protected:
	bool m_bBaseSet,m_bInitialize;		

	/*// plu 2004.06.30_15:15:18
	PLP_BASEINFO m_baseInfo;
	*/// plu 2004.06.30_15:15:18
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

	////////////////////////////////////////////////////////////////////
	float Mel(int k) 
	{ 
		/*// plu 2004.05.30_14:16:38
		return (float)(1127.f*log(1.0+(float)k*m_fres)); 
		*/// plu 2004.05.30_14:16:38
	   return (float)(1127.f * log(1.f + (k-1)*m_fres));
	}

	void  ZeroGlobalMean(short *data,int smpCount);
	void  InitFBankAndPLP();
	float ApplyFFT(short *wave,float *vfft);
	void  ConvertFrame();

	void  Realft(float *s);
	void  FFT(float *s,int inverted=0);

	/*// plu 2004.06.30_16:05:06
	void  NormEnergy();
	*/// plu 2004.06.30_16:05:06

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

public:
    
	PLP();
	~PLP();

	//void	SetBaseInfo(PLP_BASEINFO p_info);
	void SetBaseInfo(FEATURE_BASEINFO p_info);
	virtual void Initialize(int &BaseDim);

	virtual void AddWaveData(short *data,int p_smpNum,int *residue=NULL);
	virtual void WriteFile(char *filename);
	virtual float *GetFeatureDate(int *frm,short *tframeSize);
};

#endif // _HTK_PLP_H_