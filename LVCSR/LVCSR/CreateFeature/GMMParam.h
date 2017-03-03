/******************************DOCUMENT*COMMENT***********************************
*D
*D �ļ�����            : GMMParam.h
*D
*D ��Ŀ����            : 
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
*D 1.1.0001     2004.07.05     plu        �����ļ�
*D*******************************************************************************/
#ifndef _GMMPARAM_H_
#define _GMMPARAM_H_

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <malloc.h>

/*// plu 2004.07.05_14:48:43
#include "comm.h"
*/// plu 2004.07.05_14:48:43

#define STRINGLENGTH 256
#ifndef PI
#define PI 3.14159265359
#endif
#ifndef TPI
#define TPI (2.0*PI)
#endif

//typedef char string[STRINGLENGTH];   // [6/30/2007 moved by syq]

#ifndef aaaTONE_PARAM_INDEXaaa
typedef struct _MixPDF {
        float *mean;
        float *cov;
        double mat;
}   MixPDF;

typedef struct _monogmm {
    float *weight;   
    struct _MixPDF *mpdf;
	int mixNum;
}MonoGMM;

typedef struct _statis {
	float *meanAcc;
	float *varAcc;
	float mocc;
}Statis;

typedef struct _gmmacc {
	Statis *macc;
	float gocc;
}GmmAcc;

typedef struct _Header{
   int gmmNum;
   int mixNum;
   int vecSize;
}Header;
#endif

typedef struct _gclust{
	short *ele;
	int mnum;
	float occ;
	float prob;
	float sprob;
	float *mean;
	float *var;
}Gclust;

class iGmmParam {
   
protected:

   struct _monogmm *newsvec;
   struct _gmmacc *gAcc;
   int sampSize,vecSize,gmmNum,mixNum;
   /*// plu 2004.07.05_16:20:30
   int frameNum;
   */// plu 2004.07.05_16:20:30
   char fhead[256];
   char outDir[256];
   char accfile[256];
   char featDir[256];
   float log2pi;
   int num,targetNum;
   int *targetList;
   float minocc,wFloor;
   float **tmpV;
   float *varFloor;
   float totalProb;
   float **clustDist;
   int oldmixnum;
   FILE *fscore;
   
public:
   Header *header;
   int vecSize4; // added by HJ 2004-6-28
   float *prob;
   struct _monogmm *svec;

   Gclust *clust;
   Gclust *tclust;

   iGmmParam(char *model_file);
   ~iGmmParam(void);


   void savmodel(char *model_file);
   void readmodel(char *model_file);
   void Initial(int gidx);
   void AdjustVecSize(int nframeNum);
   float CalculateProb(int gidx, float *obv, int framNum);
};
#endif // _GMMPARAM_H_