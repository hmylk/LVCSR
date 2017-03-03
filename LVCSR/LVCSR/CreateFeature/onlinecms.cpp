
#include <math.h> // used for float point version of online-cms testing
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include "onlinecms.h"
#include "../include/comm.h"
extern LogFile *ApiLog;

OnLineCms::OnLineCms(int rTimes, char *CMS_bin)
{ 
	int len=0;
	void* cms=NULL;
	if (CMS_bin[0]!='\0')
	{
		FILE *fin=fopen(CMS_bin,"rb");
		if (!fin)
		{
			ApiLog->logoutf("can't open [%s] for read\n", CMS_bin);
			exit(1);
		}
		fseek(fin,0,SEEK_END);
		len = ftell(fin);
		fseek(fin,0,SEEK_SET);
		cms = malloc(len);
		fread(cms, 1, len, fin);
		fclose(fin);
	}
	else
	{
		int tempsize = 100;// 解决 plp 做hlad 下39维特征下的GMM模型下子图系统特征异常问题 [12/31/2015 hongmi]
		len = ((tempsize*2)+1)*4;
		cms =malloc(len);
		memcpy(cms,&tempsize,4);	
	}
	
	cms_vsz=*(int*)cms;
	if (len/sizeof(float) != 2*cms_vsz+1)
	{
		ApiLog->logoutf("len:%d != 2*cms_vsz:%d+1!\n", len/sizeof(float), cms_vsz);
		exit(1);
	}
	MeanInit = (float *)cms+1;
	VarInit  = (float *)cms+cms_vsz+1;
	int new_cms_vsz = (cms_vsz+3)&~3;
	newMean=(float *)Malloc32(sizeof(float)*new_cms_vsz*2);
	memset(newMean, 0, sizeof(float)*new_cms_vsz*2);
	newVar=(float *)Malloc32(sizeof(float)*new_cms_vsz*2);
	memset(newVar,  0, sizeof(float)*new_cms_vsz*2);
	iniMean=(float *)Malloc32(sizeof(float)*new_cms_vsz);
	iniVar=(float *)Malloc32(sizeof(float)*new_cms_vsz);
	tmpObv=(float *)Malloc32(sizeof(float)*new_cms_vsz);
	tmpMean=(float *)Malloc32(sizeof(float)*new_cms_vsz);
	tmpVar=(float *)Malloc32(sizeof(float)*new_cms_vsz);

	
	

	// 1:the initial value just being used one time
	// and it will be used forever
	// -1 :the initial value is used in utterance level
	// 0: do not use the initial values
    resetTimes=rTimes;

	/*
	FILE *fin;

// added by pjl: read cms matrix from chunkfile
	ChunkFile *chunk = new ChunkFile(chunkfile);
	if(chunk->FindChunkInFile(CMSMATRIX,1))
	{
   		fin = chunk->GetPtrChunkInFile(CMSMATRIX,1);
	}
    else
    {
		   tprintf7("Can't find CMS matrix file in chunk file [%s]!!\n",chunkfile);
	}
	int len = chunk->GetSizeChunk(CMSMATRIX,1);
	void *ptr=Malloc(len); 
	fread(ptr,1,len,fin);
	fclose(fin);
	delete chunk;
	*/
/*
	int *pt=(int *)CModelTable::GetCMSMatrix();
	cms_vsz=*pt;
#define _NEW_VECSIZE_
#ifdef _NEW_VECSIZE_
	int new_cms_vsz = (cms_vsz+3)&~3;
	MeanInit=(float *)Malloc32(sizeof(float)*new_cms_vsz);
    VarInit=(float *)Malloc32(sizeof(float)*new_cms_vsz);
#else
    MeanInit=(float *)Malloc32(sizeof(float)*cms_vsz);
    VarInit=(float *)Malloc32(sizeof(float)*cms_vsz);
#endif
	float *pvalue=(float *)(pt+1);
	memcpy(MeanInit,pvalue,sizeof(float)*cms_vsz);
	memcpy(VarInit,pvalue+cms_vsz,sizeof(float)*cms_vsz);
//	Free(ptr);
#ifdef _NEW_VECSIZE_
    newMean=(float *)Malloc32(sizeof(float)*new_cms_vsz);
    newVar=(float *)Malloc32(sizeof(float)*new_cms_vsz);
    iniMean=(float *)Malloc32(sizeof(float)*new_cms_vsz);
    iniVar=(float *)Malloc32(sizeof(float)*new_cms_vsz);
    tmpObv=(float *)Malloc32(sizeof(float)*new_cms_vsz);
    tmpMean=(OBVTYPE *)Malloc32(sizeof(OBVTYPE)*new_cms_vsz);
    tmpVar=(OBVTYPE *)Malloc32(sizeof(OBVTYPE)*new_cms_vsz);
#else
    newMean=(float *)Malloc32(sizeof(float)*cms_vsz);
    newVar=(float *)Malloc32(sizeof(float)*cms_vsz);
    iniMean=(float *)Malloc32(sizeof(float)*cms_vsz);
    iniVar=(float *)Malloc32(sizeof(float)*cms_vsz);
    tmpObv=(float *)Malloc32(sizeof(float)*cms_vsz);
    tmpMean=(OBVTYPE *)Malloc32(sizeof(OBVTYPE)*cms_vsz);
    tmpVar=(OBVTYPE *)Malloc32(sizeof(OBVTYPE)*cms_vsz);
#endif
	*/
}

void OnLineCms::ResetInitCmsValue(int delayNum)
{
	// initialize initial mean and initial variance
	// do it for each incoming utterance
	if (resetTimes == 1) 
	{
		memcpy(iniMean,MeanInit,sizeof(float)*cms_vsz);
		memcpy(iniVar,VarInit,sizeof(float)*cms_vsz);
		resetTimes=0;
	}
	else if (resetTimes==-1) 
	{
		//if (cms_vsz>0)
		//{
			memcpy(iniMean,MeanInit,sizeof(float)*cms_vsz);
			memcpy(iniVar,VarInit,sizeof(float)*cms_vsz);
		//}
	}
	else;;
	
	//if (cms_vsz>0)
	//{
		int new_cms_vsz = (cms_vsz+3)&~3;
		memset(newMean, 0, sizeof(float)*new_cms_vsz*2);
		memset(newVar,  0, sizeof(float)*new_cms_vsz*2);
	//}
	cms_delayNum = delayNum;
	
	return;
	
}

void OnLineCms::OnlineCmsCal(float *obv, int vecSize, int vecSize4, int len)
{
    if (cms_vsz != vecSize)
	{
		ApiLog->logoutf("Error!!! tmpDim:%d not equal to the cmslen:%d\n", vecSize, cms_vsz);
		return;
	}
	for (int i=0;i<len;i++) 
	{
		for (int j=0;j<vecSize;j++) 
		{
			float x=obv[j];
			newMean[j]=iniMean[j]+0.1f*(x-iniMean[j]);
			iniVar[j]=iniVar[j]+0.1f*((x-iniMean[j])*(x-iniMean[j])-iniVar[j]);
			newVar[j]=(float)sqrt(iniVar[j]);
			obv[j]=(x-newMean[j])/(newVar[j]+1.0f)*4.0f;
			iniMean[j]=newMean[j];
        }
		obv += vecSize4;
	}
}


void OnLineCms::OnlineCmsCal(MultiBuffer<float> *obv, int vecSize, int vecSize4, int len)
{
    if (cms_vsz != vecSize)
	{
		ApiLog->logoutf("Error!!! tmpDim:%d not equal to the cmslen:%d\n", vecSize, cms_vsz);
		return;
	}
	for (int i=0;i<len;i++)
	{
		for (int j=0;j<vecSize;j++) 
		{
			float x=obv->get(i,j);
			newMean[j]=iniMean[j]+0.1f*(x-iniMean[j]);
			iniVar[j]=iniVar[j]+0.1f*((x-iniMean[j])*(x-iniMean[j])-iniVar[j]);
			newVar[j]=(float)sqrt(iniVar[j]);
			obv->get(i,j)=(x-newMean[j])/(newVar[j]+1.0f)*4.0f;
			iniMean[j]=newMean[j];
        }
	}
}

void OnLineCms::cmsIterationUpdate(float * obv, int vecSize, int * pCmsUpdateNum)
{
	int i, num = *pCmsUpdateNum+1;
	int numSub = num>=2?(num-1):1;
	for(i=0;i<vecSize;i++){
//		newMean[i] = (newMean[i]*num + obv[i])/(num+1);
//		newVar[i] = (newVar[i]*num + (obv[i]-newMean[i])*(obv[i]-newMean[i])) / (num+1);

		newMean[i+vecSize] += obv[i];
		newVar[i+vecSize] += (obv[i]*obv[i]);
		newMean[i] = newMean[i+vecSize]/num;
		newVar[i] = (newVar[i+vecSize] - newMean[i+vecSize]*newMean[i])/numSub;
		
		if(newVar[i] < 1.0e-8 && num > 2){
			ApiLog->logoutf("Warning!!! zero variance");
			newVar[i] = 1.0e-8;
		}
	}

/*	for(int j=0;j<vecSize;j++){
			float x=obv[j];
			newMean[j]=iniMean[j]+0.1f*(x-iniMean[j]);
			iniVar[j]=iniVar[j]+0.1f*((x-iniMean[j])*(x-iniMean[j])-iniVar[j]);
			newVar[j]=(float)sqrt(iniVar[j]);
//			obv[j]=(x-newMean[j])/(newVar[j]+1.0f)*4.0f;
			iniMean[j]=newMean[j];
	}
*/	*pCmsUpdateNum=num;

}

void OnLineCms::OnlineCmsDelay(float * obv, int vecSize, int vecSize4, int len, int procLen, int * pCmsUpdateNum)
//(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum, &cms_frameNum)
{
	int i,j,k, n,m;

	if(*pCmsUpdateNum < (cms_delayNum)){
		m = cms_delayNum;
		n = len + *pCmsUpdateNum;
		k = n < m ? n : m;
		for(i=*pCmsUpdateNum, j=0 ; i<k; i++,j++)
			cmsIterationUpdate(obv+j*vecSize4, vecSize, pCmsUpdateNum);
	}
	
	if(procLen <= len && procLen > 0){
		for(i=0;i<procLen;i++){
			if(len > i+cms_delayNum){
				cmsIterationUpdate(obv+(i+cms_delayNum)*vecSize4, vecSize, pCmsUpdateNum);
			}
//printf("%dth:", *pCmsUpdateNum);
			for(j=0;j<vecSize;j++){
//if(j<1)//3)
//	printf("	%f\n",obv[i*vecSize4+j]);
//				obv[i*vecSize4+j] = (obv[i*vecSize4+j] - newMean[j])/(newVar[j]+1.0f)*4.0f;
				obv[i*vecSize4+j] = (obv[i*vecSize4+j] - newMean[j])/sqrt(newVar[j]);

//printf("plp[%d][%d]=%f\tnewMean:%f\tnewVar:%f\n",*pCmsUpdateNum-51,j,obv[i*vecSize4+j],newMean[j],sqrt(newVar[j]));
			}
		}
	}
	
}


OnLineCms::~OnLineCms(void) 
{
    //if (VarInit) Free32(VarInit);
   	//if (MeanInit) Free32(MeanInit);
	if (MeanInit) free(MeanInit-1);
    if (newVar) Free32(newVar);
   	if (newMean) Free32(newMean);
   	if (iniMean) Free32(iniMean);
   	if (iniVar) Free32(iniVar);
   	if (tmpObv) Free32(tmpObv);
   	if (tmpVar) Free32(tmpVar);
   	if (tmpMean) Free32(tmpMean);
  }


