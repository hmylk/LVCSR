#ifndef aaaGMM_PARAM_INDEXaaa
#define aaaGMM_PARAM_INDEXaaa

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <fcntl.h>
#include <malloc.h>
#ifdef WIN32
#include <WINDOWS.H>
#endif
#include "mfcc.h"
//#include "yguo/DetectRingFax.h"
//#include "hsuo/verif.h"
#include "svfeat.h"

#define STRINGLENGTH 256
#ifndef PI
#define PI 3.14159265428
#endif
#ifndef TPI
#define TPI (2.0*PI)
#endif

#define ALIGN_4F(l) (((l)+3)&(~3))
typedef char string[STRINGLENGTH];
typedef char char256[STRINGLENGTH];

struct BICSEGINFO {
	float fMetricTh;
	float fBicBase;      //merging base number;
	float fBicStep;      //merging step;
	int   fBicCutSilence;  //silence floor coeff.;
	bool  bCutSilence;    //determine whether to cut silence;
	int   nWindowsize;     //distance measure window length;
	int   nWindowShift;    //distance measure window shift step;
	int   nIteration;		 //iteration merging times;
	int   nBicShortestSeg; //define the shortest segment allowed in BIC;
	int   PCMSampleRate;   //source wave sample Rate;
	float fBicThreshold; //BIC judgement threshold;
	int   nfinalBias;      //final Bias for writing data;
	bool  bWriteWav;      //whether to output final seg wave file;
	bool  bIsWriteBICseg; //whether to write bic seg wav;
	bool  bIsWriteMFCC;   //whether to write mfcc file;
	bool  bIsWriteWavHead;//whether to write wav header;
	int varsegminlimit;  //define the shortest segment allowed for var_seg to 2nd seg;

	BICSEGINFO (){
		fMetricTh=1.2f;
		fBicBase=0.1f;
		fBicStep=0.1f;
		fBicCutSilence=50;
		bCutSilence=true;
		nWindowsize=150;
		nWindowShift=50;
		nIteration=3;
		nBicShortestSeg=100;
		PCMSampleRate=-1;
		fBicThreshold=0.0f;
		nfinalBias=10;
		bWriteWav=true;
		bIsWriteBICseg=false;
		bIsWriteMFCC=false;
		bIsWriteWavHead=true;
		varsegminlimit=-1;
	}	
};

//extern CollectStore AllData;

class iBicSeg {
   
 protected:

   SFeature *feat;
   char outDir[256];
   char featDir[256];
   float log2pi;
   float *eng2,*mfccs,*powerSpec;
   int *startpoint;
   int *endpoint;

   BICSEGINFO bicbaseinfo;
 //  CDetectRingFax *Detector1;
 //  GMMAPI *Detector2;

   private:
   
   int segLen;
   float *unif;
   float mu;

   public:

   int frameNum;
 //  Header *header;

   int vecSize4;
   int sampSize,vecSize;
   MFCC2 *mfccobj;

   iBicSeg(int vecDim, BICSEGINFO bicinfo);
   ~iBicSeg(void);
   void SetCMS(int ssize,int csn);
   void ExtractFeature(short *data, int nSmNum, FEATURE_BASEINFO mfccinfo);

//   void SetHead(char *featdir);
   //test
//   int SegDataTest(float *feat, int nFeatFrames, int *pSegInfo);

//   int MetricSeg(char *featfile);
	//test

   int SegData(char *inwave, char *segoutdir, short *data = NULL, int nSample=-1);

   float ComputeDistance(float *f1,float *f2,int winsize1,int winsize2,float **sam);

   double ippsBhadis(float *m1,float *v1,float *m2,float *v2, int vecSize);

   float ComputeBIC(float *f1,float *f2,int winsize1,int winsize2,float fPenalty,float **sam);

};


#endif
