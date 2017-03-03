/******************************DOCUMENT*COMMENT***********************************
*D
*D �ļ�����            : feat.h
*D
*D ��Ŀ����            : HCCL-LVCSR
*D
*D �汾��              : 1.1.0001
*D
*D �ļ�����            :
*D
*D
*D �ļ��޸ļ�¼
*D ------------------------------------------------------------------------------ 
*D �汾��       �޸�����       �޸���     �Ķ�����
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.11.24             �����ļ�
*D*******************************************************************************/

#ifndef aaaSPEECH_SEG_FEATUREaaa
#define aaaSPEECH_SEG_FEATUREaaa
#include "comm.h"
typedef struct {
   int frameNum; 
   int framePeriod;
   short BaseDim;	    
   short mfccKind;
} aFeatHeader;

/*********************************CLASS*COMMENT***********************************
*C
*C ������              : Feature
*C
*C ������              : ��MFCC�ļ�����һ�׶��ײ�֣�����CMS
*C
*C
*C ���޸ļ�¼
*C ------------------------------------------------------------------------------ 
*C �޸�����       �޸���     �Ķ�����
*C ------------------------------------------------------------------------------ 
*C 2003.11.25             ������
*C*******************************************************************************/
class Feature2 
{
protected:
	aFeatHeader header;
	int frameNumAll;		// =header.frameNum
	int frameStart;	   
	int frameNum;			// number of frame to do CompDelta()

	int vectSize;
	int BaseDim;
	int mfcHeaderSize;

	float *featureSet;		// new feature (such as : mfcc + delta + deltata)
	float *newMean,*newVar;      

	bool bvarNorm,bCMS; 

	int vecSize4;			// 2004.08.26 plu : 

	stringbuf mfccFile;		// filename
	FILE *fFeat;

	virtual void SetParam(int &start,int &end);
	virtual void CompDelta(int offset,const int winSize,const double sigma);

public:
	virtual void ReadMfcc(char *file);
	virtual void CalNewFeature(int &start,int &end);
	virtual float * GetObv(int fm);

	void GetFrStartAndNum(int *p_Start,int *p_Num) 
	{ *p_Start=frameStart; *p_Num=frameNum; }

	int GetFrameNumber(void) { return frameNum;	}

	int GetFramePeriod(void)	{	return header.framePeriod;	}

	Feature2(int vsz,bool p_bCMS,bool p_bvarNorm=false);
	virtual ~Feature2(void);

	// 2004.07.06 plu : 
	void CalNewFeature(int pframeNum,int pBaseDim,float *pMfccData);
};


#endif
