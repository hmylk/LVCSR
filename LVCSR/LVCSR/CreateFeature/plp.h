/******************************DOCUMENT*COMMENT***********************************
*D
*D �ļ�����            : PLP.H
*D
*D ��Ŀ����            : HCCL-LVCSR
*D
*D �汾��              : 1.1.0002
*D
*D �ļ�����            :
*D
*D
*D �ļ��޸ļ�¼
*D ------------------------------------------------------------------------------ 
*D �汾��       �޸�����       �޸���     �Ķ�����
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2004.05.27     plu        �����ļ�
*D 1.1.0002     2004.06.30     plu        ��PLP��C0�����һ������
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
    int   smpPeriod,framePeriod;			// �������ڣ�֡��
    int   chlNum,cepNum;					// �˲�����Ŀ��ͨ��������������
	int	  cepLifter,winSize;				// ��������������
    float lowPass,highPass;					// ��/�߽���Ƶ��
    float energyScale,silFloor;				// ������һ����Silence floor in dBs
	int   lpcOrder;							// LPC����
	float compressFact;						// Compression Factor fo PLP 

	bool  zeroGlobalMean,zeroFrameMean,normEnergy;
};
*/// plu 2004.06.30_15:15:13

/*********************************CLASS*COMMENT***********************************
*C
*C ������              : PLP
*C
*C ������              : 
*C
*C
*C ���޸ļ�¼
*C ------------------------------------------------------------------------------ 
*C �޸�����       �޸���     �Ķ�����
*C ------------------------------------------------------------------------------ 
*C 2004.05.27     plu        ������
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