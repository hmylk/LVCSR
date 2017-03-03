/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : RastaPlp.h
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
*D 1.1.0001     2004.05.31     plu        创建文件
*D 1.1.0002     2004.06.30     plu        用PLP的C0代替归一化能量
*D*******************************************************************************/

//
// Rasta.H
//
// C++ encapsulation of rasta processing chain (interface to old C code)
// Part of feacalc project.
//
// 1997jul28 dpwe@icsi.berkeley.edu
// $Header: /n/abbott/da/drspeech/src/feacalc/RCS/Rasta.H,v 1.7 2002/03/18 21:10:38 dpwe Exp $


#ifndef _RASTAPLP_H_
#define _RASTAPLP_H_

#include "comm.h"
#include "PitchTracker.h"

#define MINLARG			2.45E-308		/* lowest log() arg  = exp(MINEARG) */
#define LOGZERO         (-1.0e+10)

/////////////////////////////////////////////////////////////////////////
#define FRQAXIS_MEL  (1)
#define FRQAXIS_TRPZ (2)
#define FRQAXIS_TRNG (3)

#define freq2mel(f) (2595.0*log10(1.0+f/700.0))
#define mel2freq(m) (700.0*(pow(10.0,m/2595.0)-1.0))
#define HZ2BARK(a) (6. * log(((double)(a)/600.0)+sqrt((a)*(a)/360000.0+1.0)))

#define M_PI 3.1415926535

#ifndef MIN
#define MIN(a,b)	((a)<(b))?(a):(b)
#define MAX(a,b)	((a)>(b))?(a):(b)
#endif /* !MIN */

/* 	Constant values 	*/

#define SPEECHBUFSIZE 512
	/* This is only used to buffer input samples, and
	is noncritical (in other words, doesn't need to be changed
	when you change sample rate, window size, etc.) */

#define COMPRESS_EXP .33
		/* Cube root compression of spectrum is used for
	intensity - loudness conversion */

#define LOG_BASE 10. 
		/* Used for dB-related stuff */

#ifndef MAXFLOAT
#define MAXFLOAT 1e37
#endif /* !MAXFLOAT */

#define LOG_MAXFLOAT ((float)log((double) MAXFLOAT ))

#define TINY 1.0e-45
  
#define MAXHIST 25
  
#define TYP_ENHANCE 0.6

#define TYP_MODEL_ORDER 8

#define NOTHING_FLOAT 0.0

#define ONE	1.0

#define POLE_DEFAULT .94

#define HAMMING_COEF .54

#define HANNING_COEF .50

#define RECT_COEF	1.0

#define FIR_COEF_NUM 5

#define IIR_COEF_NUM 1

#define UPPER_CUTOFF_FRQ 29.0215311628

#define MIN_UPPER_CUTOFF 2.0
#define MAX_UPPER_CUTOFF 100.0

#define LOWER_CUTOFF_FRQ 0.955503546532
#define LOWER_CUTOFF_NOTSET -999

#define MIN_LOWER_CUTOFF 0.0
#define MAX_LOWER_CUTOFF 10.0

/* Limits for parameter checking  */

#define MINFILTS 1

#define MAXFILTS 100

#define MIN_NFEATS 1 

#define MAX_NFEATS 100 

/* window and step sizes in milliseconds */
#define MIN_WINSIZE 1.0

#define MAX_WINSIZE 1000.

#define MIN_STEPSIZE 1.0

#define MAX_STEPSIZE 1000.

#define MIN_SAMPFREQ 1000

#define MAX_SAMPFREQ 50000

#define MIN_POLEPOS 0.0

#define MAX_POLEPOS 1.0

#define MIN_ORDER 1

#define MAX_ORDER 100

#define MIN_LIFT .01

#define MAX_LIFT 100.

#define MIN_WINCO HANNING_COEF

#define MAX_WINCO RECT_COEF

enum {
    DOMAIN_LIN = 0,
    DOMAIN_LOG, 
    DOMAIN_CEPSTRA
};

struct RANGE
{
  int start;
  int end;
};

struct FVEC
{
  float *values;
  int length;
};


struct FMAT
{
  float **values;
  int nrows;
  int ncols;
};

struct FHistory
{
	float filtIN[MAXFILTS][MAXHIST];   // RASTA filter input buffer              
	float filtOUT[MAXFILTS][MAXHIST];  // RASTA filter output buffer             

	bool eos;						   // for Abbot I/O.  If true, we've hit the end of an
									   // utterance and need to reset the state in the
									   // rasta filter, highpass filter, and noise estimation 
};


/*// plu 2004.06.30_15:14:41
struct RASTAPLP_BASEINFO
{
    char  targetKind[256];

	bool	bDoRasta;						// true = do rasta filter
											// false = just do PLP

	int		doMain;							// = DOMAIN_LOG			log_rasta
											// = DOMAIN_CEPSTRA		rasta_cepstra

	bool    bgainflag;						// do lpc gain
	
    int		smpPeriod,framePeriod;			// 采样周期，帧长
	int		winSize;						// 窗长 /10000 = ms

	int		stepsize;						// 窗移 (ms)	default = 10

	int		cepNum;							// 倒谱数目

	int		lpcOrder;						// LPC阶数 used in PLP = 8/12

    / // Replaced by ProjectTools 2004.06.30_15:14:41/ plu 2004.06.10_15:48:28
    float	fcupper;	// frequency for placing upper zero in rasta filt 
    float	fclower;	// 3dB freq for d.c. pole/zero pair in rasta filt 
     // Replaced by ProjectTools 2004.06.30_15:14:41// plu 2004.06.10_15:48:28
	
	float    fpole;							// pole coef : 0.94-0.98

	float   fcepLifter;			// cepstra lifter coef	= 0.6f
};
*/// plu 2004.06.30_15:14:41

/*********************************CLASS*COMMENT***********************************
*C
*C 类名称              : RASTA_PLP
*C
*C 类描述              : 
*C
*C
*C 类修改记录
*C ------------------------------------------------------------------------------ 
*C 修改日期       修改人     改动内容
*C ------------------------------------------------------------------------------ 
*C 2004.05.31     plu        创建类
*C*******************************************************************************/
class RASTA_PLP 
{
protected:
	bool m_bBaseSet,m_bInitialize;		

	int m_rastaKind;
	/*// plu 2004.06.30_15:15:01
	RASTAPLP_BASEINFO m_baseInfo;
	*/// plu 2004.06.30_15:15:01
	FEATURE_BASEINFO m_baseInfo;
    FHistory m_history;

	int   m_BaseDim,m_FrameNum;
	int	  m_FFTNum,m_frameRate;		// 512, 400

	float *m_ffts;
	FVEC *m_cepCoef;
	FVEC *m_specCoef;	
	float *m_FtrData;				// final feature

	float *m_hamWin;
	
	int   m_winSize,m_nyqhz;
	float m_nyqbar;

	int   m_first_good;
	int   m_lastfilt;			// 	lastfilt = m_baseInfo.nfilts - m_first_good;

	FVEC *m_pspecptr;		// power spectra		
	
	float m_eql[MAXFILTS];
	FVEC *m_post_audptr;		// array for post-auditory spectrum

	FVEC *m_audptr;					// array for auditory spectrum and non-linear auditory spectrum
	FVEC *m_outptr;			// array for spectrum after rasta filter
	FVEC *m_cbweightptr;

	RANGE m_frange[MAXFILTS];		// pspec filt band indices  
	RANGE m_cbrange[MAXFILTS];		// cb indices for weighting 
	
    FVEC *m_autoptr;	// array for autocor 
    FVEC *m_lpcptr;			// array for predictors 
    FVEC *m_outptrOLD;		// last time's output 


	/*// plu 2004.06.02_17:32:36
	int   m_nfilters;				// number of critical band filters used       
	bool  m_bGain;					// flag that says to use gain           
	float m_lift;					// cepstral lifter exponent              
	*/// plu 2004.06.02_17:32:36

	float m_polepos;				// rasta integrator pole mag (redundant with fclower) 

	int m_i_call;

	FVEC *m_fir_filt[MAXFILTS]; /* array for numerator of
											 impulse responses */

	FVEC *m_iir_filt[MAXFILTS]; /* array for denominator of
											 impulse responses */

	int  m_nfilts;

	PitchTrack *pitchTrack;//add pitch inf by zliu
	int *pitch;

	////////////////////////////////////////////////////////////////////
	float Process(short* samps,float *pCurFrame,int p_i); 

	void  rasta_filt();
	float filt(FHistory *hptr,bool init,float inval,int nfilt,FVEC* numer,FVEC* denom);

	void set_freq_axis(); 
	void get_mel_ranges(int pspeclength);
	void get_trapezoidal_ranges(int pspeclength,float hz_in_samp);
	void get_triangular_ranges(int pspeclength,float hz_in_samp);
	void get_mel_cbweights();
	void get_trapezoidal_cbweights(float hz_in_fsamp);
	void get_triangular_cbweights(float hz_in_fsamp);
	
	FVEC * get_rasta_fir();
	FVEC *get_integ();
	
	void powspec(float *totalE);
	void AudSpec(void);
	void nl_audspec(void);
	void inverse_nonlin();
	void post_audspec(FVEC *audspec);
	/*// plu 2004.06.30_16:24:21
	FVEC *spec2lpc();
	*/// plu 2004.06.30_16:24:21
	FVEC *spec2lpc(float * p_lpcgain);

	void band_to_auto(FVEC *bandptr,FVEC *autoptr);
	void auto_to_lpc(FVEC * autoptr, FVEC * lpcptr, float *lpcgain );
	void lpc_to_cep(FVEC * lpcptr, FVEC * cepptr, int nout);
	void lpc_to_spec(FVEC * lpcptr,FVEC * specptr );
	
	FVEC *alloc_fvec( int veclength );
	void fvec_check(char *funcname, const struct FVEC *vec, int index );
	int ourint(double x); 

	/*// plu 2004.06.10_15:44:21
	// 2004.06.10 plu : 
float cb[900];
long ibegen [69];
int adw_(long* npoint,long*  nfilt,float* cb, float* eql, long* ibegen,float* sf);
	*/// plu 2004.06.10_15:44:21

	FMAT * alloc_fmat(int nrows, int ncols);
	void free_fmat(FMAT* fm);
	void free_fvec(FVEC* fv) 
	{
		free(fv->values);    free(fv);	fv=NULL;
	}

	FVEC *m_binbarks;
	FMAT *m_wts;				// critical band (weight)

public:
    
	RASTA_PLP();
	~RASTA_PLP();

	//void	SetBaseInfo(RASTAPLP_BASEINFO p_info);
	void SetBaseInfo(FEATURE_BASEINFO p_info);

	virtual void Initialize(int &BaseDim);

	virtual void AddWaveData(short *data,int p_smpNum,int *residue=NULL);
	virtual void WriteFile(char *filename);
	virtual float *GetFeatureDate(int *frm,short *tframeSize);
};

#endif // _RASTAPLP_H_