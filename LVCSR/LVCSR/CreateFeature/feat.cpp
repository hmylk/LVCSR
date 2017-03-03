/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : feat.cpp
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
*D 1.1.0001     2003.11.24             创建文件
*D 1.1.0002     2004.08.25     plu        按照GMM的需要将增加vecSize4
*D*******************************************************************************/
#include "feat.h"
#include <math.h>
#include <string.h>

// int vsz: size of vector (such as 36)
Feature2::Feature2(int p_vsz,bool p_bCMS,bool p_bvarNorm) 
{ 
   vectSize=p_vsz;    bCMS=p_bCMS;    bvarNorm=p_bvarNorm;

   featureSet=NULL; fFeat=NULL;	    newMean=newVar=NULL;
   frameNumAll=-1;

    if (bvarNorm)   
    {
		newMean=(float *)malloc(sizeof(float)*vectSize);
		newVar=(float *)malloc(sizeof(float)*vectSize);
		ASSERT2(newMean,"error alloc space for newMean!");
		ASSERT2(newVar,"error alloc space for newVar!");
    }
    else if (bCMS)
    {
		newMean=(float *)malloc(sizeof(float)*vectSize);
		ASSERT2(newMean,"error alloc space for newMean!");
    }
}

Feature2::~Feature2(void) 
{
   if (fFeat)		fclose(fFeat);
   if (featureSet)	free(featureSet);
   if (newMean)		free(newMean);		
   if (newVar)		free(newVar);
}

float * Feature2::GetObv(int fm) 
{ 
    ASSERT2(featureSet,"error:no feature can be used. pls call OpenMfcc and CalNewFeature firstly");
    ASSERT4(fm<=frameNum,"error: input fm=%d frameNum=%d too big!",fm,frameNum);
    /*// plu 2004.08.26_17:35:47
    return featureSet+fm*vectSize; 
    */// plu 2004.08.26_17:35:47
	return featureSet+fm*vecSize4;
}

void Feature2::CompDelta(int offset,const int winSize,const double sigma) 
{
   for (int i=0;i<frameNum;i++) 
   {
      for (int j=0;j<BaseDim;j++) 
	  {
         double sum=0;
         for (int k=1;k<=winSize;k++) 
		 {
            float *ffeat=featureSet;
			ffeat+=min(i+k,frameNum-1)*vectSize+offset;
            float *bfeat=featureSet;
			bfeat+=max(i-k,0)*vectSize+offset;
            sum+=k*(ffeat[j]-bfeat[j]);
         }
         featureSet[i*vectSize+offset+BaseDim+j]=(float)(sum*sigma);
      }
   }
}

void Feature2::ReadMfcc(char *file) 
{
   if (fFeat) fclose(fFeat);
   ReadOpen(fFeat,file);  
   strcpy(mfccFile,file);
 
   fread(&header,sizeof(aFeatHeader),1,fFeat);
   mfcHeaderSize=ftell(fFeat);

   frameNumAll=header.frameNum;		    // total number of frame in mfcc file 
   ASSERT2(frameNumAll>1,"too few : frameNumAll=1 !!");

   BaseDim=header.BaseDim/sizeof(float);    // basic dim of mfcc

   // test dim
   ASSERT5(BaseDim==vectSize || BaseDim*2==vectSize ||
       BaseDim*3==vectSize,"can't deal with %s! BaseDim=%d, VectSize=%d",file,BaseDim,vectSize);
}

void Feature2::SetParam(int &start,int &end) 
{
   if (start<0) start=0;
   ASSERT4(start<frameNumAll,"error : start=%d > frameNumAll=%d",start,frameNumAll);

   if (end<0 || end>=frameNumAll) end=frameNumAll-1;
   frameStart=start; 
   frameNum=end-start+1;
   ASSERT2(frameNum>1,"too few : frameNum=1 !!");
}

void Feature2::CalNewFeature(int &start,int &end) 
{
    ASSERT2(frameNumAll>1,"pls call OpenMfcc() firstly!");

   int i,j;

   SetParam(start,end);

   if (featureSet) free(featureSet);
   featureSet=(float *)malloc(sizeof(float)*vectSize*frameNum);
   ASSERT2(featureSet,"CalNewFeature() err: alloc featureSet!");

   fseek(fFeat,mfcHeaderSize+frameStart*BaseDim*sizeof(float),SEEK_SET);
   for (i=0;i<frameNum;i++) 
   {
      float *buff=featureSet+i*vectSize;
      ASSERT5((int)fread(buff,sizeof(float),BaseDim,fFeat)==BaseDim,
              "Read feature failed at %s:(%d~%d)",mfccFile,frameStart,frameStart+frameNum);
   }

  if (BaseDim<vectSize) 
  {
      const int DELWIN=2,ACCWIN=2;
      const float SIGMAD2=10.0,SIGMAA2=10.0;

      CompDelta(0,DELWIN,1.0/SIGMAD2);
      if (BaseDim*3==vectSize) CompDelta(BaseDim,ACCWIN,1.0/SIGMAA2);
   }

   // do variance normalization
   if (bvarNorm) 
   {
     memset(newMean,0,sizeof(float)*vectSize);
     memset(newVar,0,sizeof(float)*vectSize);

      // on-line calculate mean & var
     for (i=0;i<frameNum;i++) 
     {
       float *obv=featureSet+i*vectSize,x;
       for (j=0;j<vectSize;j++) 
	   {
         x=obv[j]; newMean[j]+=x; newVar[j]+=x*x;
       }
     }
     for (j=0;j<vectSize;j++) 
	 {
        float m=newMean[j];
        newMean[j]/=(float)frameNum;
        newVar[j]=(frameNum>1)?(float)sqrt((newVar[j]-newMean[j]*m)/(float)(frameNum-1)):1.0f;
        if(newVar[j]<1.0e-8) WARNING1("zero variance");
     }
     for (i=0;i<frameNum;i++) 
	 {
       float *obv=featureSet+i*vectSize;
       for (j=0;j<vectSize;j++)
         obv[j]=(obv[j]-newMean[j])/newVar[j];
     } 
   } 
   else if (bCMS)
   {
     memset(newMean,0,sizeof(float)*vectSize);

      // on-line calculate mean & var
     for (i=0;i<frameNum;i++) 
       for (j=0;j<vectSize;j++) 
          newMean[j] += featureSet[i*vectSize+j];
     
     for (j=0;j<vectSize;j++)     newMean[j]/=(float)frameNum;
     for (i=0;i<frameNum;i++)
       for (j=0;j<vectSize;j++)
         featureSet[i*vectSize+j] -= -newMean[j];
    }
}


// 2004.07.06 plu : 
void Feature2::CalNewFeature(int pframeNum,int pBaseDim,float *pMfccData)
{
   frameNumAll=pframeNum;
   ASSERT2(frameNumAll>1 ,"too few : frameNumAll=1 !!");

   BaseDim=pBaseDim;

   // test dim
   ASSERT4(BaseDim==vectSize || BaseDim*2==vectSize ||
       BaseDim*3==vectSize,"!!! BaseDim=%d, VectSize=%d!!!",BaseDim,vectSize);

   ///////////////////////////////////////////////////
   int i,j;

   int start,end;	start=end=-1;
   SetParam(start,end);

   if (featureSet) free(featureSet);
   /*// plu 2004.08.25_15:04:36
   featureSet=(float *)malloc(sizeof(float)*vectSize*frameNum);
   */// plu 2004.08.25_15:04:36
	vecSize4=(vectSize+3)&(~3);

   featureSet=(float *)malloc(sizeof(float)*vecSize4*frameNum);
   ASSERT2(featureSet,"CalNewFeature() err: alloc featureSet!");
   memset(featureSet,0,sizeof(float)*vecSize4*frameNum);

   float *oldData = pMfccData;
   for (i=0;i<frameNum;i++) 
   {
      /*// plu 2004.08.25_15:06:50
      float *buff=featureSet+i*vectSize;
      */// plu 2004.08.25_15:06:50
	  float *buff=featureSet+i*vecSize4;
	  memcpy(buff,oldData,BaseDim*sizeof(float));
	  oldData += BaseDim;
   }

  if (BaseDim<vectSize) 
  {
      const int DELWIN=2,ACCWIN=2;
      const float SIGMAD2=10.0,SIGMAA2=10.0;

      CompDelta(0,DELWIN,1.0/SIGMAD2);
      if (BaseDim*3==vectSize) CompDelta(BaseDim,ACCWIN,1.0/SIGMAA2);
   }

   // do variance normalization
   if (bvarNorm) 
   {
     memset(newMean,0,sizeof(float)*vectSize);
     memset(newVar,0,sizeof(float)*vectSize);

      // on-line calculate mean & var
     for (i=0;i<frameNum;i++) 
     {
       /*// plu 2004.08.25_15:06:57
       float *obv=featureSet+i*vectSize,x;
       */// plu 2004.08.25_15:06:57
		float *obv=featureSet+i*vecSize4,x;
		for (j=0;j<vectSize;j++) 
		{
			x=obv[j]; newMean[j]+=x; newVar[j]+=x*x;
		}
     }
     for (j=0;j<vectSize;j++) 
	 {
        float m=newMean[j];
        newMean[j]/=(float)frameNum;
        newVar[j]=(frameNum>1)?(float)sqrt((newVar[j]-newMean[j]*m)/(float)(frameNum-1)):1.0f;
        if(newVar[j]<1.0e-8) WARNING1("zero variance");
     }
     for (i=0;i<frameNum;i++) 
	 {
       /*// plu 2004.08.25_15:07:53
       float *obv=featureSet+i*vectSize;
       */// plu 2004.08.25_15:07:53
		float *obv=featureSet+i*vecSize4;
		for (j=0;j<vectSize;j++)
			obv[j]=(obv[j]-newMean[j])/newVar[j];
     } 
   } 
   else if (bCMS)
   {
     memset(newMean,0,sizeof(float)*vectSize);

      // on-line calculate mean & var
     for (i=0;i<frameNum;i++) 
       for (j=0;j<vectSize;j++) 
          /*// plu 2004.08.25_15:08:54
          newMean[j] += featureSet[i*vectSize+j];
          */// plu 2004.08.25_15:08:54
		  newMean[j] += featureSet[i*vecSize4+j];
     
     for (j=0;j<vectSize;j++)     newMean[j]/=(float)frameNum;
     for (i=0;i<frameNum;i++)
       for (j=0;j<vectSize;j++)
         /*// plu 2004.08.25_15:09:16
         featureSet[i*vectSize+j] -= newMean[j];
         */// plu 2004.08.25_15:09:16
         featureSet[i*vecSize4+j] -= newMean[j];
    }

   //return featureSet;
   return;
}