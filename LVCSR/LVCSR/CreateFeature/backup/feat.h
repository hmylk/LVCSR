/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : feat.h
*D
*D 项目名称            : HCCL-LVCSR
*D
*D 版本号              : 1.1.0001
*D
*D 文件描述            :
*D
*D
*D 文件修改记录
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D 1.1.0001     2003.11.24             创建文件
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
*C 类名称              : Feature
*C
*C 类描述              : 从MFCC文件生成一阶二阶差分，或做CMS
*C
*C
*C 类修改记录
*C ------------------------------------------------------------------------------ 
*C 修改日期       修改人     改动内容
*C ------------------------------------------------------------------------------ 
*C 2003.11.25             创建类
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
