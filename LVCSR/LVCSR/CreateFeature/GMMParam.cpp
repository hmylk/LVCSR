/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : GMMParam.cpp
*D
*D 项目名称            : 
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
*D 1.1.0001     2004.07.05     plu        创建文件
*D*******************************************************************************/
#include "ippsr.h"
#include "GMMParam.h"

iGmmParam::iGmmParam(char *model_file)
{
   FILE *fin;
   header=(Header *)malloc(sizeof(Header));
   fin=fopen(model_file,"rb");
   if (!fin) {
	   printf("error: open file %s\n", model_file);
	   return;
   }
   fread(header,sizeof(Header),1,fin);
   mixNum=header->mixNum;
   vecSize=header->vecSize;
   gmmNum=header->gmmNum;
   vecSize4=(vecSize+3)&(~3);

   log2pi = (float)log(TPI);
   svec=(MonoGMM*)malloc(sizeof(MonoGMM)*(gmmNum+1));
   fclose(fin);

   int i;
   for(i=0;i<gmmNum+1;i++) {
		Initial(i);
   }
   readmodel(model_file);   
   //feat = NULL;
   tmpV = NULL;
   prob = NULL;
   varFloor=(float *)malloc(sizeof(float)*vecSize);
   /*// plu 2004.07.05_16:20:33
   frameNum=0;
   */// plu 2004.07.05_16:20:33
   totalProb=0.0;
   oldmixnum=mixNum;
   num=0;
}

iGmmParam::~iGmmParam(void)
{
   int i;
   for(i=0;i<gmmNum+1;i++){
		int m;
		for(m=0;m<mixNum;m++){
			/*// plu 2004.07.05_14:49:48
			Free32(svec[i].mpdf[m].mean);
			Free32(svec[i].mpdf[m].cov);
			*/// plu 2004.07.05_14:49:48
			free(svec[i].mpdf[m].mean);
			free(svec[i].mpdf[m].cov);

		}
		free(svec[i].mpdf);
		free(svec[i].weight);

   }
   
   free(header);
   free(svec);
   if(prob)free(prob);
   if(tmpV){
	 int m;
	 for(m=0;m<oldmixnum;m++)
		 if(tmpV[m])free(tmpV[m]);
	 free(tmpV);
	 tmpV=NULL;
   }
   free(varFloor);
}

void iGmmParam::Initial(int gidx)
{
	int m;

	svec[gidx].mpdf=(MixPDF*)malloc(mixNum*sizeof(MixPDF));
	svec[gidx].weight=(float*)malloc(mixNum*sizeof(float));
	
	svec[gidx].weight[0]=1.0;

	for(m=0;m<mixNum;m++){
		/*// plu 2004.07.05_14:50:09
		svec[gidx].mpdf[m].mean=(float *)Malloc32(vecSize4*sizeof(float), true);
		svec[gidx].mpdf[m].cov=(float *)Malloc32(vecSize4*sizeof(float), true);
		*/// plu 2004.07.05_14:50:09
		svec[gidx].mpdf[m].mean=(float *)malloc(vecSize4*sizeof(float));
		svec[gidx].mpdf[m].cov=(float *)malloc(vecSize4*sizeof(float));

		memset(svec[gidx].mpdf[m].mean,0,vecSize4*sizeof(float));
		memset(svec[gidx].mpdf[m].cov,0,vecSize4*sizeof(float));
	}
}

float iGmmParam::CalculateProb(int gidx, float *obv, int framNum)
{
	// 2004.07.05 plu : 
	if (!tmpV || !prob)
	{
		printf("pls call iGmmParam::AdjustVecSize() first!\n");	exit(-1);
	}

	/*ippsLogGauss_32f_D2(obv,IPP_APPEND(vecSize,4),
						svec[gidx].mpdf[0].mean,
						svec[gidx].mpdf[0].cov,
						vecSize,
						prob,
						framNum,
						svec[gidx].mpdf[0].mat);*/
	
	ippsLogGauss_32f_D2(obv,vecSize4,
						svec[gidx].mpdf[0].mean,
						svec[gidx].mpdf[0].cov,
						vecSize,
						prob,
						framNum,
						(float)svec[gidx].mpdf[0].mat);
	
	memcpy(tmpV[0],prob,sizeof(float)*framNum);
	int m;
	for(m=1;m<mixNum;m++)
	{
		ippsLogGauss_32f_D2(obv,IPP_APPEND(vecSize,4),
						svec[gidx].mpdf[m].mean,
						svec[gidx].mpdf[m].cov,
						vecSize,
						tmpV[m],
						framNum,
						(float)svec[gidx].mpdf[m].mat);
		ippsLogAdd_32f(tmpV[m],prob,framNum,ippAlgHintNone);
	}
	float totalProb=0.0;
	int i;
	for(i=0;i<framNum;i++){
		totalProb+=prob[i];	
	}
	return totalProb/framNum;
}

void iGmmParam::AdjustVecSize(int nframeNum)
{
	if(nframeNum>num){
		num = nframeNum;
		if(prob)free(prob);
		prob=(float *)malloc(sizeof(float)*num);
		if(!tmpV){
			tmpV=(float **)malloc(sizeof(float *)*mixNum);
			memset(tmpV,0,sizeof(float *)*mixNum);
		}
		int m;
		for(m=0;m<mixNum;m++){
			if(tmpV[m])free(tmpV[m]);
			tmpV[m]=(float *)malloc(sizeof(float)*num);
		}
	}
}

void iGmmParam::readmodel(char *model_file)
{
	int i,m;
	FILE *fin;
	MixPDF *pdf;

	fin=fopen(model_file,"rb");

	fread(header,sizeof(Header),1,fin);
	for(i=0;i<gmmNum;i++){
		fread(svec[i].weight,sizeof(float),mixNum,fin);
		for (m=0;m<mixNum;m++) {
			pdf=&(svec[i].mpdf[m]);
			fread(pdf->mean,sizeof(float),vecSize4,fin);
			fread(pdf->cov,sizeof(float),vecSize4,fin);
			fread(&(pdf->mat),sizeof(double),1,fin);
		}
	}
	fclose(fin);
}

void iGmmParam::savmodel(char *model_file) 
{
	int i,m;
	FILE *fout;
	MixPDF *pdf;
	fout=fopen(model_file,"wb");
	fwrite(header,sizeof(Header),1,fout);
	for(i=0;i<gmmNum;i++){
		fwrite(svec[i].weight,sizeof(float),mixNum,fout);
		for(m=0;m<mixNum;m++){
			pdf=&(svec[i].mpdf[m]);
			fwrite(pdf->mean,sizeof(float),vecSize,fout);
			fwrite(pdf->cov,sizeof(float),vecSize,fout);
			fwrite(&(pdf->mat),sizeof(double),1,fout);
		}
	}
	fclose(fout);
}