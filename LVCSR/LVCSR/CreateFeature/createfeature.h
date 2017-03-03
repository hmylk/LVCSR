
#ifndef _CREATEFEATURE_H_
#define _CREATEFEATURE_H_

struct FeatureHeader {
	int frameNum;
	int framePeriod;
	short frameSize;
	short mfccKind;
};

#include "mfcc.h"
#include "plp.h"
#include "PLP_NCC.h"
#include "RastaPlp.h"
#include "feat.h"
#include "GMMParam.h"
#include "../LVCSR_API.h"
#include "../include/comm.h"
#include "config.h"
#include "onlinecms.h"
#include "MultiBuffer.h"

extern LogFile *ApiLog;

struct SEGMENT_CONTENT
{
	short *dataBuf;
	//	short *dataBuf_sil;			// 包含头尾sil的buf
	unsigned int nsmpNum;
	//	int nsmpNum_sil;			// 包含头尾sil的sample num	
	SOUND_TYPE myType;
	//	SEGMENT_CONTENT *pPre,*pNext;
	SEGMENT_CONTENT()
	{
		dataBuf=NULL;
		//		dataBuf_sil=NULL;
		//		pPre=pNext=NULL;
		myType=MALE;//MALE;
	}
};

class CreateFeature
{
public:
	void InitFeatureCfg(char *systemDir, char *cfgFileName, bool bOnlineMode=false);
	void AddWaveData(short *data,int p_smpNum);
	float *GetFeature(short *waveData, int smpNum, FeatureHeader *header, bool bBaseOnly=false, TSR_STREAM_FLAG flag = TSR_STREAM_ALL, int *residue=NULL);
	void WriteFile(char *outFileName, float *tgtfeature, FeatureHeader header, bool bAlign4 = false, bool bAppend = false);
	bool FindSegType(short *waveData, int smpNum, SOUND_TYPE *myType);
	void CleanFeature();
	CreateFeature() { }
	~CreateFeature(){};
	int CalResidue(int smpNum, int *frameNum=NULL);
	int CalRemainder(int &smpNum);
	int GetCacheLen(){return cms_cacheLen;}
	void WriteBuffer(char *buffer,int *length);
	void WriteBaseFeatFile(char* outFileName);
	
protected:
	MFCC2		*m_mfcc;
	PLP			*m_plp;
	PLP_NCC     *m_plp_ncc;
	RASTA_PLP	*m_rasta;
	FEATURE_BASEINFO fea_info;

	int m_targetKind;
	bool bHLDA;
	int iDeltaNum; //add by hongmi
	double *Matrix;
	int matrix_h,matrix_n;
	// lita [4/9/2010]
	bool bonlinecms; // from llu
	OnLineCms *cmsobj;
	void* cmslen;
	float *src;
	float *tgt;
	float *hldatgt;
	// frame lag for online delta
	int   BaseDim;
	int   FrameLag;//bHLDA?6:4;//
	float *Buffer;
	int   FrameCnt;
	int   FrameBack;
	MultiBuffer<float> *mBuffer;
	short *BufferWav;
	int   SampleCnt;
	MultiBuffer<short> *mBufferWav;

	int cms_strategy;//cms method num
	int cms_frameNum;//current computed cms frame num
	int cms_delayNum;//the delay frame num for cms
	int cms_cacheLen;//Cache some data in TSRBuffer.cpp for init the cms

	int rmZeroValue_strategy;//Use different methods to remove the zero value. 0 means no removing. 1 means qingqing's method.
	float saveLastFrameEng;//Save the energy of last frame.
	
//	void SetBaseInfo(FEATURE_BASEINFO p_info);
//	void AddWaveData(short *data,int p_smpNum);
    void WriteFile(char *outFileName);
	float VoiceCoef;   // [6/26/2007 copyed from Segment by syq]
	MFCC2 *mfcc;
	iGmmParam *gmm;
	int m_nGmmNum;
	Feature2 *feat;
	aConfigEnv *envs;
	int   SampleRate;
	int   frameRate;
	int   winSize;

	char* HLDA_MatrixDir;

	void SetEnv(aConfigEnv* &envs,char *config);
	char *GetEnv(char *env);
	void ReadConfig(char *line,int& num);
	void ReadConfig(char *line,int& num1,int& num2);
	void ReadConfig(char *line,bool& bln);
	void ReadConfig(char *line,float& num);
	void ReadConfig(char *line,float& num1,float& num2);
	void ReadConfig(char *line,float& num1,float& num2,float& num3);
	void ReadConfig(char *line,float& n1,float& n2,float& n3,float& n4);
	void ReadConfig(char *line,char *str);
	void ReadConfig(char *line,char *str1,char *str2);
	void CleanInfo();
	void SetBaseInfo(FEATURE_BASEINFO p_info);

	void LinearSmoothPitchArray(float* src, int num);
	void LinearSmoothNCCArray(float *src, int num);
	void MedianSmoothArray(float* src, int num);
	int AddFrameNum(int num);
	float* pitchesOri;
	float* nccsOri ;
};

#endif // _CREATEFEATURE_H_