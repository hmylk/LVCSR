/******************************DOCUMENT*COMMENT***********************************
*D
*D 文件名称            : PLP.cpp
*D ------------------------------------------------------------------------------ 
*D 版本号       修改日期       修改人     改动内容
*D ------------------------------------------------------------------------------ 
*D*******************************************************************************/
#include <MATH.H>
#include <STRING.H>
#include "PLP_NCC.h"


PLP_NCC::PLP_NCC()
{
	m_chlMap=NULL;
	m_chlWeight=m_hamWin=m_cepWin=m_fBank=NULL;

	m_plpData=NULL;

	m_Auditory=m_AutoCorrelation=m_LP=m_EQL=NULL;
	m_CosineMatrix=NULL;		

	pitch=NULL;//add pitch info by zliu
	voiced_Degree=NULL;//add by qqzhang
	pitchTrack=NULL;

	m_bBaseSet = false;
	m_bInitialize=false;		
	Matrix = NULL;	
	m_ffts = NULL;

	m_cepCoef=NULL;
	iniMean=NULL;
	iniVar=NULL;
	newMean=NULL;
	newVar=NULL;
	cmslen=NULL;
	HLDAMatrixDir = NULL;
}

PLP_NCC::~PLP_NCC()
{
	if (m_chlMap)		delete []m_chlMap,m_chlMap=NULL;
	if (m_chlWeight)	delete []m_chlWeight,m_chlWeight=NULL;
	if (m_hamWin)		delete []m_hamWin,m_hamWin=NULL;
	if (m_cepWin)		delete []m_cepWin,m_cepWin=NULL;
	if (m_fBank)		delete []m_fBank,m_fBank=NULL;

	if (m_plpData)		delete []m_plpData,m_plpData=NULL;

	if (m_Auditory)			delete []m_Auditory,m_Auditory=NULL;
	if (m_AutoCorrelation)	delete []m_AutoCorrelation,m_AutoCorrelation=NULL;	
	if (m_LP)				delete []m_LP,m_LP=NULL;
	if (m_EQL)				delete []m_EQL,m_EQL=NULL;
	if (m_CosineMatrix)		
	{
		delete [] m_CosineMatrix;
		m_CosineMatrix = NULL;
		/*// plu 2004.05.28_21:39:30
		char *tmp=(char *)m_CosineMatrix;
		delete []tmp;
		*/// plu 2004.05.28_21:39:30
	}
	if (m_ffts) {
		delete  [] m_ffts,m_ffts=NULL;
	}
	if (m_cepCoef) {
		delete [] m_cepCoef,m_cepCoef=NULL;
	}
	if(pitch)	delete[] pitch,pitch=NULL;//add pitch info by zliu
	if(voiced_Degree)	delete voiced_Degree;
	if(pitchTrack) delete pitchTrack;
	if(Matrix) delete[] Matrix,Matrix=NULL;
	if(HLDAMatrixDir)  delete[] HLDAMatrixDir,HLDAMatrixDir=NULL;
	//add by llu
	if(m_baseInfo.bonline)
	{
		delete  []iniMean;
		delete  []iniVar;
		delete  []newMean;
		delete	[]newVar;
		delete	[]cmslen;
	}

#ifdef UseIPP
	ippsFFTFree_R_32f(ctxFFT);
	delete []vfft1;
#endif

}

void PLP_NCC::WriteBuffer(char *buffer,int *length)

{
	if (!m_plpData)
	{
		printf("error when WriteFile(): m_plpData=NULL");	return;
	}

	int ii;
	float *ft;
	for (ii = 0, ft = m_plpData; ii < m_FrameNum; ii++, ft += m_BaseDim)
	{
		ippsMulC_32f_I(10.f,ft,m_BaseDim);
		ippsThreshold_32f_I(ft,m_BaseDim,(float)(-DYN_RANGE),ippCmpLess);
		ippsThreshold_32f_I(ft,m_BaseDim,(float)DYN_RANGE,ippCmpGreater);
	}



	if (m_baseInfo.tgtDim%m_BaseDim !=0) {
		printf("Error!!! tgtdim to the basedim\n");
		exit(1);
	}


	float *tgt;
	tgt = new float [m_FrameNum*m_baseInfo.tgtDim];

	const int DELWIN=2,ACCWIN=2;
	const float SIGMAD2=10.0,SIGMAA2=10.0;  
	int i,j,k;
	for (i=0;i<m_FrameNum;i++)
	{	   
		memcpy(tgt+m_baseInfo.tgtDim*i,m_plpData+i*m_BaseDim,m_BaseDim*sizeof(float));
	}
	int delta;
	for(delta = 0; delta < m_baseInfo.tgtDim/m_BaseDim-1; delta++){
		for (i=0;i<m_FrameNum;i++) 
		{
			for (j=0;j<m_BaseDim;j++) 
			{
				double sum=0;
				for (k=1;k<=DELWIN;k++)
				{
					float *ffeat=tgt+min(i+k,m_FrameNum-1)*m_baseInfo.tgtDim+delta*m_BaseDim;
					float *bfeat=tgt+max(i-k,0)*m_baseInfo.tgtDim+delta*m_BaseDim;
					sum+=k*(ffeat[j]-bfeat[j]);
				}
				tgt[i*m_baseInfo.tgtDim+delta*m_BaseDim+m_BaseDim+j]=float(sum*(1.0/SIGMAD2));
			}
		}
	}
	if (delta > 0) 
	{
		//cmn
		if(!m_baseInfo.bonline)
		{
			int frm;
			float curpMean[2000], curpVar[2000];
			for (j = 0; j < m_baseInfo.tgtDim; j++) 
			{ 
				double res;
				curpMean[j] = 0.0; 
				curpVar[j]  = 0.0; 

				for (frm = 0; frm < m_FrameNum; frm++ )
				{
					res = tgt[frm*m_baseInfo.tgtDim + j];
					curpMean[j] += res; 
					curpVar[j] += res*res; 
				}

				double mean,var;
				mean = curpMean[j]/(double)m_FrameNum;
				if (m_FrameNum > 1)
					var = sqrt((curpVar[j] - mean*curpMean[j])/(double)(m_FrameNum-1));
				else 
					var = double(1.0);
				if(var == 0.0f) var = double(1.0);
				for ( frm=0; frm < m_FrameNum; frm++ )
				{
					res = tgt[frm*m_baseInfo.tgtDim + j];
					tgt[frm*m_baseInfo.tgtDim + j] = float((res - mean)/var);

				}
			}
		}
		//add by llu
		else
		{
			int frm;
			newMean=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			newVar=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			iniMean=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			iniVar=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			float * pt = (float *)cmslen;
			pt++;
			memcpy(iniMean,pt,sizeof(float)*m_baseInfo.tgtDim);
			memcpy(iniVar,pt+m_baseInfo.tgtDim,sizeof(float)*m_baseInfo.tgtDim);

			for (frm = 0; frm < m_FrameNum; frm++ )
			{
				for (int j=0;j<m_baseInfo.tgtDim;j++) 
				{
					float x=tgt[frm*m_baseInfo.tgtDim + j];
					newMean[j]=iniMean[j]+0.1f*(x-iniMean[j]);
					iniVar[j]=iniVar[j]+0.1f*((x-iniMean[j])*(x-iniMean[j])-iniVar[j]);
					newVar[j]=(float)sqrt(iniVar[j]);
					tgt[frm*m_baseInfo.tgtDim + j]=(tgt[frm*m_baseInfo.tgtDim + j]-newMean[j])/(newVar[j]+1.0f)*4.0f;
					iniMean[j]=newMean[j];
				}
			}
		}
		//add over llu
	}

	if(m_baseInfo.bHLDA)
	{
		if (matrix_h != m_baseInfo.tgtDim) {
			printf("error dim !!!\n");
			return;
		}		



		short tframeSize = matrix_n*sizeof(float);

		FeatHead_ncc feathead;
		feathead.nFrameNum = m_FrameNum;
		feathead.nFramePeriod = m_baseInfo.framePeriod;
		feathead.nFrameSize = tframeSize;
		feathead.plpKind = m_plpKind;
		memcpy(buffer,&feathead,12);
		float *pBuf = (float *)((char *)buffer+12);
		*length = 12;


		int i,j,k;
		float inft[2000],outft[2000];
#ifdef UseIPP
		float *Matrix_copy = new float[matrix_h*matrix_n];
		for(k=0;k<matrix_h*matrix_n;k++)
			Matrix_copy[k]=(float)Matrix[k];
		mkl_set_num_threads(1);
#endif

		for (i=0; i<m_FrameNum; i++) {
			memcpy(inft,tgt+i*m_baseInfo.tgtDim,m_baseInfo.tgtDim*sizeof(float));

#ifndef UseIPP
			for (k=0;k<matrix_n;k++){
				outft[k] = 0;
				for(j = 0;j < matrix_h;j++){
					outft[k] += inft[j]*Matrix[k*matrix_h+j];
				}
			}
#else
			cblas_sgemv(CblasRowMajor,CblasNoTrans,matrix_n,matrix_h,1.0f,Matrix_copy,matrix_h,inft,1,0.0f,pBuf,1);
#endif


			//memcpy(pBuf,outft,sizeof(float) * matrix_n);
			pBuf += matrix_n;
            *length = *length + matrix_n*4;

		}		

	
		pBuf = NULL;





#ifdef UseIPP
		delete []Matrix_copy;
		Matrix_copy = NULL;
#endif

	}
	else
	{
		short tframeSize=(short)(m_baseInfo.tgtDim*sizeof(float));

	}
	delete []tgt;
}


void PLP_NCC::WriteBaseFeatFile(char *filename)
{
	if (!m_plpData)
	{
		printf("error when WriteFile(): m_plpData=NULL");	return;
	}

	short tframeSize=(short)(m_BaseDim*sizeof(float));

	
	FILE *fout;
	fout=fopen(filename,"wb");

	if (fout==NULL) {
		printf("!!!! Error: open %s failed\n", filename);
		return;
	}

//	ASSERT2(fout,"Error open %s for read.",filename);
	fwrite(&m_FrameNum,sizeof(int),1,fout);
	//fwrite(&m_baseInfo.framePeriod,sizeof(int),1,fout);
	//fwrite(&tframeSize,sizeof(short),1,fout);
	//fwrite(&m_plpKind,sizeof(short),1,fout);

	//printf("m_FrameNum=%d, m_BaseDim=%d\n", m_FrameNum, m_BaseDim);

	fwrite(m_plpData, sizeof(float), m_FrameNum*m_BaseDim, fout);
	fclose(fout);  
	return;
}

float * PLP_NCC::GetFeatureDate(int *frm,short *tframeSize)
{
	if (!m_plpData)
	{
		printf("error when WriteFile(): m_plpData=NULL");	return NULL;
	}
	*frm=m_FrameNum;
	*tframeSize=(short)(m_BaseDim*sizeof(float));

 
	return m_plpData;
}



//#define FIX_FEAT
void PLP_NCC::WriteFile(char *outFile)
{
	if (!m_plpData)
	{
		printf("error when WriteFile(): m_plpData=NULL");	return;
	}
#ifndef UseIPP
	int ii,kk;
	float *ft;
	for (ii=0,ft=m_plpData;ii<m_FrameNum;ii++,ft+=m_BaseDim) 
	{
		for (kk=0;kk<m_BaseDim;kk++) 
		{
			//if (k == m_BaseDim-1) {

			// }else{
			ft[kk] *= 10.f;	// 2004.11.04 plu : plp参数的量级太小，加倍之
			if (ft[kk]>DYN_RANGE) ft[kk]=DYN_RANGE;
			if (ft[kk]<-DYN_RANGE) ft[kk]= -DYN_RANGE;
#ifdef FIX_FEAT
			short tmp = (short)(ft[kk]*512);
			ft[kk] = (float)tmp/512;
#endif
			//}
		}
	}
#else
	int ii;
	float *ft;
/*	for (ii = 0, ft = m_plpData; ii < m_FrameNum; ii++, ft += m_BaseDim)
	{
		ippsMulC_32f_I(10.f,ft,m_BaseDim);
		ippsThreshold_32f_I(ft,m_BaseDim,(float)(-DYN_RANGE),ippCmpLess);
		ippsThreshold_32f_I(ft,m_BaseDim,(float)DYN_RANGE,ippCmpGreater);
	} */

#endif

	if (m_baseInfo.tgtDim%m_BaseDim !=0) {
		printf("Error!!! tgtdim to the basedim\n");
		exit(1);
	}


	float *tgt;
	tgt = new float [m_FrameNum*m_baseInfo.tgtDim];

	const int DELWIN=2,ACCWIN=2;
	const float SIGMAD2=10.0,SIGMAA2=10.0;  
	int i,j,k;
	for (i=0;i<m_FrameNum;i++)
	{	   
		memcpy(tgt+m_baseInfo.tgtDim*i,m_plpData+i*m_BaseDim,m_BaseDim*sizeof(float));
	}
	int delta;
	for(delta = 0; delta < m_baseInfo.tgtDim/m_BaseDim-1; delta++){
		for (i=0;i<m_FrameNum;i++) 
		{
			for (j=0;j<m_BaseDim;j++) 
			{
				double sum=0;
				for (k=1;k<=DELWIN;k++)
				{
					float *ffeat=tgt+min(i+k,m_FrameNum-1)*m_baseInfo.tgtDim+delta*m_BaseDim;
					float *bfeat=tgt+max(i-k,0)*m_baseInfo.tgtDim+delta*m_BaseDim;
					sum+=k*(ffeat[j]-bfeat[j]);
				}
				tgt[i*m_baseInfo.tgtDim+delta*m_BaseDim+m_BaseDim+j]=float(sum*(1.0/SIGMAD2));
			}
		}
	}
	if (delta > 0) 
	{
		//cmn
		if(!m_baseInfo.bonline)
		{
			int frm;
			float curpMean[2000], curpVar[2000];
			for (j = 0; j < m_baseInfo.tgtDim; j++) 
			{ 
				double res;
				curpMean[j] = 0.0; 
				curpVar[j]  = 0.0; 

				for (frm = 0; frm < m_FrameNum; frm++ )
				{
					res = tgt[frm*m_baseInfo.tgtDim + j];
					curpMean[j] += res; 
					curpVar[j] += res*res; 
				}

				double mean,var;
				mean = curpMean[j]/(double)m_FrameNum;
				if (m_FrameNum > 1)
					var = sqrt((curpVar[j] - mean*curpMean[j])/(double)(m_FrameNum-1));
				else 
					var = double(1.0);
				if(var == 0.0f) var = double(1.0);
				for ( frm=0; frm < m_FrameNum; frm++ )
				{
					res = tgt[frm*m_baseInfo.tgtDim + j];
					tgt[frm*m_baseInfo.tgtDim + j] = float((res - mean)/var);

				}
			}
		}
		//add by llu
		else
		{
			int frm;
			newMean=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			newVar=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			iniMean=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			iniVar=(float *)malloc(sizeof(float)*m_baseInfo.tgtDim);
			float * pt = (float *)cmslen;
			pt++;
			memcpy(iniMean,pt,sizeof(float)*m_baseInfo.tgtDim);
			memcpy(iniVar,pt+m_baseInfo.tgtDim,sizeof(float)*m_baseInfo.tgtDim);

			for (frm = 0; frm < m_FrameNum; frm++ )
			{
				for (int j=0;j<m_baseInfo.tgtDim;j++) 
				{
					float x=tgt[frm*m_baseInfo.tgtDim + j];
					newMean[j]=iniMean[j]+0.1f*(x-iniMean[j]);
					iniVar[j]=iniVar[j]+0.1f*((x-iniMean[j])*(x-iniMean[j])-iniVar[j]);
					newVar[j]=(float)sqrt(iniVar[j]);
					tgt[frm*m_baseInfo.tgtDim + j]=(tgt[frm*m_baseInfo.tgtDim + j]-newMean[j])/(newVar[j]+1.0f)*4.0f;
					iniMean[j]=newMean[j];
				}
			}
		}
		//add over llu
	}

	if(m_baseInfo.bHLDA)
	{
		if (matrix_h != m_baseInfo.tgtDim) {
			printf("error dim !!!\n");
			return;
		}		

#ifndef NOT_WRITE_FEATURE
		FILE *fout = fopen(outFile,"wb");
		ASSERT2(fout,"Error open %s for read.",outFile);
#endif

		short tframeSize = matrix_n*sizeof(float);
#ifndef WriteWholeFeat

#ifndef NOT_WRITE_FEATURE
		fwrite(&m_FrameNum,sizeof(int),1,fout);
		//fwrite(&m_baseInfo.framePeriod,sizeof(int),1,fout);
		//fwrite(&tframeSize,sizeof(short),1,fout);
		//fwrite(&m_plpKind,sizeof(short),1,fout);
#endif
		
#else
		FeatHead_ncc feathead;
		feathead.nFrameNum = m_FrameNum;
		feathead.nFramePeriod = m_baseInfo.framePeriod;
		feathead.nFrameSize = tframeSize;
		feathead.plpKind = m_plpKind;
		float *outBuffer = new float[m_FrameNum * matrix_n];
		float *pBuf = outBuffer;
#endif

		int i,j,k;
		float inft[2000],outft[2000];
#ifdef UseIPP
		float *Matrix_copy = new float[matrix_h*matrix_n];
		for(k=0;k<matrix_h*matrix_n;k++)
			Matrix_copy[k]=(float)Matrix[k];
		mkl_set_num_threads(1);
#endif

		for (i=0; i<m_FrameNum; i++) {
			memcpy(inft,tgt+i*m_baseInfo.tgtDim,m_baseInfo.tgtDim*sizeof(float));

#ifndef UseIPP
			for (k=0;k<matrix_n;k++){
				outft[k] = 0;
				for(j = 0;j < matrix_h;j++){
					outft[k] += inft[j]*Matrix[k*matrix_h+j];
				}
			}
#else
#ifdef WriteWholeFeat
			cblas_sgemv(CblasRowMajor,CblasNoTrans,matrix_n,matrix_h,1.0f,Matrix_copy,matrix_h,inft,1,0.0f,pBuf,1);
#else
			cblas_sgemv(CblasRowMajor,CblasNoTrans,matrix_n,matrix_h,1.0f,Matrix_copy,matrix_h,inft,1,0.0f,outft,1);
#endif
#endif

#ifndef WriteWholeFeat

#ifndef NOT_WRITE_FEATURE
			fwrite(outft,sizeof(float),matrix_n,fout);	
#endif

#else
			//memcpy(pBuf,outft,sizeof(float) * matrix_n);
			pBuf += matrix_n;
#endif
		}		
#ifdef WriteWholeFeat
		fwrite(&feathead,sizeof(FeatHead_ncc),1,fout);
		fwrite(outBuffer,sizeof(float),matrix_n * m_FrameNum,fout);
		delete []outBuffer;
		outBuffer = NULL;
		pBuf = NULL;
#endif

#ifndef NOT_WRITE_FEATURE
		fclose(fout);
#endif
		
#ifdef UseIPP
		delete []Matrix_copy;
		Matrix_copy = NULL;
#endif

	}
	else
	{
		short tframeSize=(short)(m_baseInfo.tgtDim*sizeof(float));
#ifndef NOT_WRITE_FEATURE
		FILE *fout;
		fout=fopen(outFile,"wb");
		ASSERT2(fout,"Error open %s for read.",outFile);
		fwrite(&m_FrameNum,sizeof(int),1,fout);
		//fwrite(&m_baseInfo.framePeriod,sizeof(int),1,fout);
		//fwrite(&tframeSize,sizeof(short),1,fout);
		//fwrite(&m_plpKind,sizeof(short),1,fout);
		fwrite(tgt,sizeof(float),m_FrameNum*m_baseInfo.tgtDim,fout);
		fclose(fout);
#endif
	}
	delete []tgt;
}


//void PLP_NCC::SetBaseInfo(PLP_BASEINFO p_info)
void PLP_NCC::SetBaseInfo(FEATURE_BASEINFO p_info)
{
	m_plpKind=0;
	if (strstr(p_info.targetKind,"MFCCPLP_NCC"))	
	{
		m_plpKind=FYT_MFCCPLP_NCC;
		if (strstr(p_info.targetKind,"_C0"))		m_plpKind|=HASENERGY;
	}
	else
	{
		printf("Error set targetkind in MFCCPLP_NCC!\n");	exit(-1);		
	}
	ASSERT2(m_plpKind&KINDMASK,"Unsupported target kind: %s",p_info.targetKind);

	m_baseInfo.smpPeriod   = p_info.smpPeriod;
	m_baseInfo.framePeriod = p_info.framePeriod;

	m_baseInfo.bonline = p_info.bonline;//add by llu

	m_baseInfo.lowPass  = p_info.lowPass;
	m_baseInfo.highPass = p_info.highPass;
	m_baseInfo.winSize  = p_info.winSize;
	m_baseInfo.chlNum   = p_info.chlNum;

	m_baseInfo.cepNum   = p_info.cepNum;
	m_baseInfo.lpcOrder = p_info.lpcOrder;
	m_baseInfo.cepLifter = p_info.cepLifter;    
	m_baseInfo.compressFact = p_info.compressFact;

	m_baseInfo.zeroGlobalMean = p_info.zeroGlobalMean;

	if (m_baseInfo.cepNum < 2 || m_baseInfo.cepNum > m_baseInfo.lpcOrder)
		WARNING2("ValidCodeParms: unlikely num cep coef %d",m_baseInfo.cepNum);

	m_bBaseSet=true;

	m_baseInfo.bPitch = p_info.bPitch;
	m_baseInfo.bHLDA = p_info.bHLDA;
	m_baseInfo.tgtDim = p_info.tgtDim;
	m_baseInfo.WarpAlpha=1.0f;
	m_baseInfo.WarpLCutoff=300.0f;
	m_baseInfo.WarpUCutoff=3000.0f;

	
	//	m_baseInfo.bonline = false;
}

void PLP_NCC::SetHLDAMatrixDir(char* matrixDir)
{
	HLDAMatrixDir =new char[256*sizeof(char)];
	HLDAMatrixDir[0]='\0';
	if (matrixDir==NULL||matrixDir[0]=='\0')
	{
		return;
	}
	strcpy(HLDAMatrixDir,matrixDir);
}

void PLP_NCC::Initialize(int &BaseDim) 
{
	//ASSERT1(m_bBaseSet,"pls call SetBaseInfo() firstly!");

	m_frameRate = m_baseInfo.framePeriod/m_baseInfo.smpPeriod;
	m_winSize   = m_baseInfo.winSize/m_baseInfo.smpPeriod;

	m_FFTNum=2; 	
#ifndef UseIPP
	while (m_FFTNum < m_winSize) m_FFTNum*=2;
#else
	nPLPOrder = 1;
	while (m_FFTNum < m_winSize)
	{
		m_FFTNum *= 2;
		nPLPOrder++;
	}
	CreateVector(&vfft1,m_FFTNum + 2);
#endif
	m_BaseDim=(m_plpKind&HASENERGY)?(m_baseInfo.cepNum+1):m_baseInfo.cepNum;
	//add by zliu 
	if (m_baseInfo.bPitch) {
		m_BaseDim += 2; //add pitch
	}
	BaseDim = m_BaseDim;
	// cep lifter
	if (m_baseInfo.cepLifter>0) 
	{
		CreateVector(&m_cepWin,m_baseInfo.cepNum);
		//ASSERT1(m_cepWin,"error alloc space for m_cepWin");

		float a=(float)(PI/(float)m_baseInfo.cepLifter);
		for (int i=1;i<=m_baseInfo.cepNum;i++)
			m_cepWin[i]=(float)(1.f+(float)m_baseInfo.cepLifter/2.0*sin( i * a));

		cepWinL = m_baseInfo.cepLifter;
		cepWinSize = m_baseInfo.cepNum;
	}

	// ham windows

	CreateVector(&m_hamWin,m_winSize);
	/*
	double a = TPI /(double)(m_winSize - 1);
	for (int i=1;i<=m_winSize;i++)
	m_hamWin[i] =(float)( 0.54 - 0.46 * cos(a*(i-1)));*/



#ifndef UseIPP
	double a = TPI /(double)(m_winSize - 1);
	for (int i=1;i<=m_winSize;i++)
		m_hamWin[i] =(float)( 0.54 - 0.46 * cos(a*(i-1)));
#else
	ippsSet_32f(1,m_hamWin + 1,m_winSize);
	ippsWinHamming_32f_I(m_hamWin + 1, m_winSize);
#endif


	//InitFBankAndPLP();
	//add by zliu
	if (m_baseInfo.bPitch) 
		pitchTrack = new PitchTrack(m_baseInfo.smpPeriod, m_baseInfo.winSize, m_baseInfo.framePeriod);
	if(m_baseInfo.bHLDA){	
		FILE *matrixfile=fopen(HLDAMatrixDir,"rb");
		if (!matrixfile)
		{
			printf("error opening eigen.bin file!\n");
			exit(1);
		}	
		fread(&matrix_h,sizeof(int),1,matrixfile);
		fread(&matrix_n,sizeof(int),1,matrixfile);
		Matrix = new double[matrix_h*matrix_n];		
		fread(Matrix,sizeof(double),matrix_h*matrix_n,matrixfile);
		fclose(matrixfile);		
	}

	m_bInitialize=true;
	//add by llu
	if(m_baseInfo.bonline){
		FILE *fin_lu=fopen("CMS.bin","rb");
		if ( fin_lu==NULL )
		{
			printf("can't open CMS.bin for read\n");
			exit(1);
		}
		fseek(fin_lu,0,SEEK_END);
		int len_lu = ftell(fin_lu);
		fseek(fin_lu,0,SEEK_SET);
		cmslen = malloc(len_lu); 
		fread(cmslen, 1, len_lu, fin_lu);
		fclose(fin_lu);
	}
	//add by llu
#ifdef UseIPP
	IppStatus status;
	//status = ippsFFTInitAlloc_R_32f(&ctxFFT,nPLPOrder,IPP_FFT_NODIV_BY_ANY,ippAlgHintNone);
	status = ippsFFTInitAlloc_R_32f(&ctxFFT,nPLPOrder,IPP_FFT_NODIV_BY_ANY,ippAlgHintFast);
#endif
}
//add by llu


void PLP_NCC::InitFBankAndPLP(float p_warpAlpha) 
{
	//////////////////////////////////////////////////////
	//		init freq bank
	float mlow,mhigh,mwidth,*bnkCentre;
	int k;

	// lita added 2013/05/27
	
	if (m_chlMap)		{delete []m_chlMap; m_chlMap=NULL;}
	if (m_chlWeight)	{delete []m_chlWeight;m_chlWeight=NULL;}
	//if (m_hamWin)		delete []m_hamWin;
	//if (m_cepWin)		delete []m_cepWin;
	if (m_fBank)		delete []m_fBank,m_fBank=NULL;



	if (m_Auditory)			delete []m_Auditory,m_Auditory=NULL;
	if (m_AutoCorrelation)	delete []m_AutoCorrelation,m_AutoCorrelation=NULL;	
	if (m_LP)				delete []m_LP,m_LP=NULL;
	if (m_EQL)				delete []m_EQL,m_EQL=NULL;
	if (m_cepCoef)          delete []m_cepCoef,m_cepCoef=NULL;//add by hm for leak detect
	if (m_CosineMatrix)		
	{
		delete [] m_CosineMatrix,m_CosineMatrix=NULL;
	}
	if (m_ffts) 
	{
		delete  [] m_ffts,m_ffts=NULL;
	}
	// lita added 2013/05/27

	m_fres=(float)(1.0e7/(m_baseInfo.smpPeriod*m_FFTNum*700.0));
	m_kLow=2; 
	mlow=0;
	if (m_baseInfo.lowPass>=0.0) 
	{
		mlow=(float)(1127*log(1.0+m_baseInfo.lowPass/700.0));
		m_kLow=(int)((m_baseInfo.lowPass*m_baseInfo.smpPeriod*(1.0e-7)*m_FFTNum)+0.5);
		//HTK
		//m_kLow=(int)((m_baseInfo.lowPass*m_baseInfo.smpPeriod*(1.0e-7)*m_FFTNum)+2.5);

		if (m_kLow<2) m_kLow=2;
	}

	m_kHigh=m_FFTNum/2; 
	mhigh=Mel(m_FFTNum/2+1);
	if (m_baseInfo.highPass>=0.0) 
	{
		mhigh=(float)(1127*log(1.0+m_baseInfo.highPass/700.0));
		m_kHigh=(int)((m_baseInfo.highPass*m_baseInfo.smpPeriod*(1.0e-7)*m_FFTNum)+0.5);

		if (m_kHigh>m_FFTNum/2) m_kHigh=m_FFTNum/2;
	}

	//////////////////////
	int maxChan = m_baseInfo.chlNum+1;
	/* Create vector of fbank centre frequencies */
	CreateVector(&bnkCentre,maxChan);
	mwidth=mhigh-mlow;
	int chan;
	if ( 1 || fabs(p_warpAlpha-1.0)<1.0e-7 ) //changed by lita for pitch_ncc
		{
	for (chan=1; chan <= maxChan; chan++) 
		bnkCentre[chan] = ((float)chan/(float)maxChan)*mwidth + mlow;
	}
else
	{
		float minFreq = 700.0 * (exp (mlow / 1127.0) - 1.0 );
		float maxFreq = 700.0 * (exp (mhigh / 1127.0) - 1.0 );
		float cf;

		for (chan=1;chan<=maxChan;chan++)
		{
			cf = ((float)(chan)/(float)maxChan)*mwidth+mlow;

			cf = 700 * (exp (cf / 1127.0) - 1.0);
			//	 bnkCentre[chan]= 1127.0 * log (1.0 + cf / 700.0);
			//   printf("bnkCentre[%d]=%f WarpFreq=%f oriFreq=%f m_baseInfo.WarpLCutoff=%f, m_baseInfo.WarpUCutoff=%f, cf=%f, minFreq=%f, maxFreq=%f, p_warpAlpha=%f\n",chan,bnkCentre[chan],WarpFreq (m_baseInfo.WarpLCutoff, m_baseInfo.WarpUCutoff, cf, minFreq, maxFreq, p_warpAlpha),cf,m_baseInfo.WarpLCutoff, m_baseInfo.WarpUCutoff, cf, minFreq, maxFreq, p_warpAlpha);
			// bnkCentre[chan]= 1127.0 * log (1.0 + WarpFreq (300, 3000, cf, minFreq, maxFreq, p_warpAlpha) / 700.0);
			bnkCentre[chan]= 1127.0 * log (1.0 + WarpFreq (m_baseInfo.WarpLCutoff, m_baseInfo.WarpUCutoff, cf, minFreq, maxFreq, p_warpAlpha) / 700.0);
			//	 printf("bnkCentre[%d]=%f WarpFreq=%f oriFreq=%f m_baseInfo.WarpLCutoff=%f, m_baseInfo.WarpUCutoff=%f, cf=%f, minFreq=%f, maxFreq=%f, p_warpAlpha=%f\n",chan,bnkCentre[chan],WarpFreq (m_baseInfo.WarpLCutoff, m_baseInfo.WarpUCutoff, cf, minFreq, maxFreq, p_warpAlpha),cf,m_baseInfo.WarpLCutoff, m_baseInfo.WarpUCutoff, cf, minFreq, maxFreq, p_warpAlpha);
		}
	}
	/* Create loChan map, loChan[fftindex] -> lower channel index */	// m_chlMap ->  fb.loChan
	float melk;
	CreateShortVec(&m_chlMap,m_FFTNum/2);
	for (k=1,chan=1; k<=m_FFTNum/2; k++)
	{
		melk = Mel(k);
		if (k<m_kLow || k>m_kHigh) m_chlMap[k]=-1;
		else {
			while (bnkCentre[chan] < melk  && chan<=maxChan) ++chan;
			m_chlMap[k] = chan-1;		 
		}

	}

	/* Create vector of lower channel weights */   // m_chlWeight -> fb.loWt
	CreateVector(&m_chlWeight,m_FFTNum/2);
	for (k=1; k<=m_FFTNum/2; k++) 
	{
		chan = m_chlMap[k];
		if (k<m_kLow || k>m_kHigh) m_chlWeight[k]=0.0;
		else {
			if (chan>0) 
				m_chlWeight[k] = ((bnkCentre[chan+1] - Mel(k)) / (bnkCentre[chan+1] - bnkCentre[chan]));
			else
				m_chlWeight[k] = (bnkCentre[1]-Mel(k))/(bnkCentre[1] - mlow);
		}

	}

	CreateVector(&m_fBank,m_baseInfo.chlNum);	
	CreateVector(&m_ffts,m_FFTNum);

	//////////////////////////////////////////////////////////////////
	//		init PLP
	int i,j;
	double baseAngle;
	float f_hz_mid, fsub, fsq;
	int  nAuto, nFreq;

	// alloc space
	CreateVector(&m_cepCoef,m_baseInfo.cepNum+1);
	CreateVector(&m_Auditory,m_baseInfo.chlNum + 2);
	CreateVector(&m_EQL,m_baseInfo.chlNum);
	CreateVector(&m_AutoCorrelation,m_baseInfo.lpcOrder + 1);
	CreateVector(&m_LP,m_baseInfo.lpcOrder + 1);
	/*// plu 2004.07.18_15:37:23
	CreateDMatrix(&m_CosineMatrix,m_baseInfo.lpcOrder+1,m_baseInfo.chlNum+2);
	*/// plu 2004.07.18_15:37:23
	m_CosineMatrix = CreateDMatrix(m_baseInfo.lpcOrder+1,m_baseInfo.chlNum+2);

	for (i=1; i<=m_baseInfo.chlNum; i++) 
	{
		f_hz_mid = 700*(exp(bnkCentre[i]/1127)-1); /* Mel to Hz conversion */
		fsq = (f_hz_mid * f_hz_mid);
		fsub = fsq / (fsq + 1.6e5);
		m_EQL[i] = fsub * fsub * ((fsq + 1.44e6)  /(fsq + 9.61e6));
	}

	/* Builds up matrix of cosines for IDFT */
	nAuto = m_baseInfo.lpcOrder+1; 
	nFreq = m_baseInfo.chlNum+2;
	baseAngle =  PI / (double)(nFreq - 1);
	for (i=0; i<nAuto; i++) 
	{
		m_CosineMatrix[i+1][1] = 1.0;
		for (j=1; j<(nFreq-1); j++)
			m_CosineMatrix[i+1][j+1] = 2.0 * cos(baseAngle * (double)i * (double)j);

		m_CosineMatrix[i+1][nFreq] = cos(baseAngle * (double)i * (double)(nFreq-1));
	}

	delete []bnkCentre,bnkCentre=NULL;
}


/******************************FUNCTION*COMMENT***********************************
*F
*F 函数名称            : PLP_NCC::AddWaveData
*F ------------------------------------------------------------------------------ 
*F 修改日期20131223       修改人songmeixu     改动内容pitchTrack->DoUtterance(tmpwav,m_FrameNum, &pitch, &voiced_Degree, initPitch);
*F ------------------------------------------------------------------------------ 
*F*******************************************************************************/
void PLP_NCC::AddWaveData(short *waveData,int smpNum, bool initPitch)
{
	//ASSERT1(m_bInitialize,"pls call Initialize() firstly!");
	//ASSERT1(waveData,"error in AddWaveData(): input wavedata=NULL!");
	//ASSERT1(smpNum>0,"error in AddWaveData(): input smpNum <= 0!");
   float p_warpAlpha=1.0;
   float average_pitch=0.0;
	float global_pitch=200.0;

	int count=0;
	m_FrameNum=(smpNum-m_winSize)/m_frameRate+1;
	if (m_baseInfo.bPitch){
		pitch=NULL;
		voiced_Degree=NULL;
		short *tmpwav = new short [smpNum];

		memcpy(tmpwav,waveData,sizeof(short)*smpNum);
		//	short *tmpwav=waveData;
		pitchTrack->DoUtterance(tmpwav,m_FrameNum, &pitch, &voiced_Degree, initPitch);
		
		for (int frame = 0; frame < m_FrameNum; frame++)
		{
			if(voiced_Degree[frame] > 0.70)
			{
				average_pitch=average_pitch+(float)pitch[frame];
				count++;
				//printf("voicedDegrees[%d]\t%f\tpitch[%d]\t%d\n",frame,voicedDegrees[frame],frame,pitches[frame]);
			}
		}
		if(count<3)
			average_pitch=global_pitch;
		else
			average_pitch=average_pitch/count;
		/////////////////////////////////////////////////////////////////
		//    p_warpAlpha=1-(0.002*(average_pitch - 150.0));
		//    calc VTLN fwarpAlpha from 0.7 to 1.4\
		//    according to pitch range from 68hz to 470hz (CallHK training set)
		/////////////////////////////////////////////////////////////////
		p_warpAlpha=1-(0.00175*(average_pitch - global_pitch));

		//p_warpAlpha=1.0/p_warpAlpha;

		//printf("p_warpAlpha=%f\n",p_warpAlpha);

		InitFBankAndPLP(p_warpAlpha);
		delete[] (tmpwav);
	}

	int i;

	// remove time-domain mean
	if (m_baseInfo.zeroGlobalMean) ZeroGlobalMean(waveData,smpNum);

	float *pCurFrame,energy;
	short *pCurWave;

	if (m_plpData)	
	{
		delete []m_plpData;	m_plpData=NULL;
	}

	m_plpData=new float[m_FrameNum*m_BaseDim];

	//ASSERT1(m_plpData!=NULL,"error alloc space for m_plpData");
	if (!m_plpData)
	{
		printf("error alloc space for m_plpData");
		return;
	}

	pCurFrame = m_plpData;	
	pCurWave  = waveData;
	for (i=0; i<m_FrameNum; i++)
	{
		//printf("frame = %d :\r",i);
		energy=ApplyFFT(pCurWave,m_ffts);
		ASSERT2(energy>=0,"log(%g)",energy);

		ConvertFrame();
		if (m_baseInfo.bPitch) {
			for (int j=0;j<m_BaseDim-2;j++)
				*pCurFrame++ = m_cepCoef[j+1];
			//*pCurFrame++ = (float)log(pitch[i]);
			*pCurFrame++ = pitch[i]/1000.0;
			*pCurFrame++ = voiced_Degree[i];
		}else{
			for (int j=0;j<m_BaseDim;j++)
				*pCurFrame++ = m_cepCoef[j+1];
		}


		pCurWave += m_frameRate;
	}
#ifndef SILENCE_MODE
	//printf("%d, %d\n", m_BaseDim,m_FrameNum);  
#endif

}

/******************************FUNCTION*COMMENT***********************************
*F
*F 函数名称            : PLP_NCC::ConvertFrame
*F ------------------------------------------------------------------------------ 
*F 修改日期       修改人     改动内容
*F ------------------------------------------------------------------------------ 
*F*******************************************************************************/
void PLP_NCC::ConvertFrame() 
{
	const float melFloor=1.0;

	// EXPORT-> Pre-emphasise filter bank output with the simulated 
	// equal-loudness curve and perform amplitude compression 
	int i;
	for (i=1; i<=m_baseInfo.chlNum; i++) 
	{
		if (m_fBank[i] < melFloor) m_fBank[i] = melFloor;

		m_Auditory[i+1] = m_fBank[i] * m_EQL[i]; /* Apply equal-loudness curve */
		m_Auditory[i+1] = pow((double) m_Auditory[i+1], (double) m_baseInfo.compressFact);
	}
	m_Auditory[1] = m_Auditory[2];  // Duplicate values at either end 
	m_Auditory[m_baseInfo.chlNum+2] = m_Auditory[m_baseInfo.chlNum+1];

	/* EXPORT->ASpec2LPCep: Perform IDFT to get autocorrelation values then 
	produce autoregressive coeffs. and cepstral transform them */
	float lpcGain, E;

	/* Do IDFT to get autocorrelation values */
	E = MatrixIDFT(m_Auditory, m_AutoCorrelation, m_CosineMatrix);

	m_LP[VectorSize(m_LP)] = 0.0;    /* init to make Purify et al. happy */

	/* do Durbin recursion to get predictor coefficients */
	lpcGain = Durbin(NULL,m_LP,m_AutoCorrelation,E,m_baseInfo.lpcOrder);
	if (lpcGain<=0) 
		WARNING1("ASpec2LPCep: Negative lpcgain");

	LPC2Cepstrum(m_LP,m_cepCoef);

	/* value forms C0 */
	m_cepCoef[VectorSize(m_cepCoef)] = (float) -log((double) 1.0/lpcGain); 

	if (m_baseInfo.cepLifter>0)
		WeightCepstrum(m_cepCoef, 1, m_baseInfo.cepNum, m_baseInfo.cepLifter);	
}

void PLP_NCC::ZeroGlobalMean(short *data,int smpCount) 
{
	double mean=0.0; int i;
	for (i=0;i<smpCount;i++) 
		mean+=data[i];

	mean/=(double)smpCount;

	for (i=0;i<smpCount;i++) 
	{
		double y=(float)data[i]-mean;
		if (y>32767) y=32767;
		if (y<-32767) y= -32767;

		data[i] = (short)((y>0)?y + 0.5:y-0.5);
	}
}

/*// plu 2004.06.30_16:05:59
void PLP_NCC::NormEnergy()
{
float maxe,mine,*ft; int i;

maxe= *(ft=&m_plpData[m_baseInfo.cepNum]);
for (i=0;i<m_FrameNum;i++,ft+=m_BaseDim)
if (*ft>maxe) maxe= *ft;

mine=(float)(maxe-(m_baseInfo.silFloor*log(10.0))/10.0);

for (i=0,ft=&m_plpData[m_baseInfo.cepNum];i<m_FrameNum;i++,ft+=m_BaseDim) 
{
if (*ft<mine) *ft=mine;
*ft=1.0f-(maxe-(*ft))*m_baseInfo.energyScale;
}
}
*/// plu 2004.06.30_16:05:59

/////////////////////////////////////////////////////////////////////////////////
//			math calculation
/////////////////////////////////////////////////////////////////////////////////

// short *wave: input wave data
// float *vfft: output fft data
float PLP_NCC::ApplyFFT(short *wave,float *vfft)                   
{
	int k,j,i;
	float energy=0.0;
#ifndef UseIPP
	for (i=0;i<m_winSize;i++) vfft[i+1]=(float)wave[i];
	for (i=m_winSize;i<m_FFTNum;i++) vfft[i+1]=0;

	// 计算时域能量
	for (i=0;i<m_winSize;i++)	energy+=vfft[i+1]*vfft[i+1];

	// 预加重
	for (i=m_winSize-1;i>0;i--)	vfft[i+1] -= vfft[i]*0.97f;
	vfft[1] *= 0.03f;
	// ham window
	for (i=0;i<m_winSize;i++)	vfft[i+1] *= m_hamWin[i+1];
	Realft(vfft);
#else

	ippsConvert_16s32f(wave,vfft + 1,m_winSize);// the 0st index of vectors created by function createvector is the length of the vector [4/10/2013 Xuyang Wang]
	ippsZero_32f(&vfft[m_winSize + 1],m_FFTNum - m_winSize);                            

	ippsDotProd_32f(vfft + 1,vfft + 1,m_winSize,&energy);

	ippsPreemphasize_32f(vfft + 1,m_winSize,0.97);
	vfft[1] *= (float)(1.0-0.97);                                   

	ippsMul_32f_I(m_hamWin + 1,vfft + 1,m_winSize);

	ippsFFTFwd_RToCCS_32f(vfft + 1,vfft1,ctxFFT,NULL);
	memcpy(vfft + 1,vfft1,sizeof(float) * m_FFTNum);

#endif

	float ek,t1,t2;

	// Fill filterbank channels 
	//ZeroVector(m_fBank); 

	ZeroVector(m_fBank); 


	for (k = m_kLow; k <= m_kHigh; k++) {             /* fill bins */
		t1 = vfft[2*k-1]; t2 = vfft[2*k];
		ek = sqrt(t1*t1 + t2*t2);

		i = m_chlMap[k];
		t1 = m_chlWeight[k]*ek;
		if (i>0) m_fBank[i] += t1;
		if (i<m_baseInfo.chlNum) m_fBank[i+1] += ek - t1;
	}
	return energy;
}


/* Matrix IDFT converts from auditory spectrum into autocorrelation values */
float PLP_NCC::MatrixIDFT(float * as, float * ac, double ** cm)
{
	double acc;
	float E;
	int nAuto, nFreq;
	int i, j;

	nFreq = VectorSize(as);
	nAuto = VectorSize(ac);

	for (i=0; i<nAuto; i++) {
		acc = cm[i+1][1] * (double)as[1];
		for (j=1; j<nFreq; j++)
			acc += cm[i+1][j+1] * (double)as[j+1];

		if (i>0) 
			ac[i] = (float)(acc / (double)(2.0 * (nFreq-1)));
		else  
			E = (float)(acc / (double)(2.0 * (nFreq-1)));
	}     
	return E; /* Return zero'th auto value separately */
}


/* Durbins recursion to get LP coeffs for auto values */
float PLP_NCC::Durbin(float * k, float * thisA, float * r, float E, int order)
{
	float *newA;
	float ki;         /* Current Reflection Coefficient */
	int i,j;

	CreateVector(&newA,order);
	for (i=1;i<=order;i++) {
		ki = r[i];              /* Calc next reflection coef */
		for (j=1;j<i;j++)
			ki = ki + thisA[j] * r[i - j];
		ki = ki / E;   
		if (k!=NULL) k[i] = ki;
		E *= 1 - ki*ki;         /* Update Error */
		newA[i] = -ki;          /* Calc new filter coef */
		for (j=1;j<i;j++)
			newA[j] = thisA[j] - ki * thisA[i - j];
		for (j=1;j<=i;j++)   
			thisA[j] = newA[j];
	}
	delete []newA;
	return (E);
}

/* EXPORT->LPC2Cepstrum: transfer from lpc to cepstral coef */
void PLP_NCC::LPC2Cepstrum (float * a, float * c)
{
	int i,n,p;
	float sum;

	p=VectorSize(c);
	for (n=1;n<=p;n++)  { 
		sum = 0.0;
		for (i=1;i<n;i++) 
			sum = sum + (n - i) * a[i] * c[n - i];
		c[n] = -(a[n] + sum / n);
	}
}

/* EXPORT->WeightCepstrum: Apply cepstral weighting to c */
void PLP_NCC::WeightCepstrum (float * c, int start, int count, int cepLiftering)
{
	int i,j;
	j = start;
	for (i=1;i<=count;i++)     c[j++] *= m_cepWin[i];
}

/* EXPORT-> FFT: apply fft/invfft to complex s */
/*
When called s holds nn complex values stored in the
sequence   [ r1 , i1 , r2 , i2 , .. .. , rn , in ] where
n = VectorSize(s) DIV 2, n must be a power of 2. On exit s
holds the fft (or the inverse fft if invert == 1) 
*/
void PLP_NCC::FFT(float * s, int invert)
{
	int ii,jj,n,nn,limit,m,j,inc,i;
	double wx,wr,wpr,wpi,wi,theta;
	double xre,xri,x;

	n=VectorSize(s);
	nn=n / 2; j = 1;
	for (ii=1;ii<=nn;ii++) {
		i = 2 * ii - 1;
		if (j>i) {
			xre = s[j]; xri = s[j + 1];
			s[j] = s[i];  s[j + 1] = s[i + 1];
			s[i] = xre; s[i + 1] = xri;
		}
		m = n / 2;
		while (m >= 2  && j > m) {
			j -= m; m /= 2;
		}
		j += m;
	};
	limit = 2;
	while (limit < n) {
		inc = 2 * limit; theta = TPI / limit;
		if (invert) theta = -theta;
		x = sin(0.5 * theta);
		wpr = -2.0 * x * x; wpi = sin(theta); 
		wr = 1.0; wi = 0.0;
		for (ii=1; ii<=limit/2; ii++) {
			m = 2 * ii - 1;
			for (jj = 0; jj<=(n - m) / inc;jj++) {
				i = m + jj * inc;
				j = i + limit;
				xre = wr * s[j] - wi * s[j + 1];
				xri = wr * s[j + 1] + wi * s[j];
				s[j] = s[i] - xre; s[j + 1] = s[i + 1] - xri;
				s[i] = s[i] + xre; s[i + 1] = s[i + 1] + xri;
			}
			wx = wr;
			wr = wr * wpr - wi * wpi + wr;
			wi = wi * wpr + wx * wpi + wi;
		}
		limit = inc;
	}
	if (invert)
		for (i = 1;i<=n;i++) 
			s[i] = s[i] / nn;
}

/* EXPORT-> Realft: apply fft to real s */
/*
When called s holds 2*n real values, on exit s holds the
first  n complex points of the spectrum stored in
the same format as for fft
*/
void PLP_NCC::Realft (float * s)
{
	int n, n2, i, i1, i2, i3, i4;
	double xr1, xi1, xr2, xi2, wrs, wis;
	double yr, yi, yr2, yi2, yr0, theta, x;

	n=VectorSize(s) / 2; n2 = n/2;
	theta = PI / n;
	FFT(s, 0);
	x = sin(0.5 * theta);
	yr2 = -2.0 * x * x;
	yi2 = sin(theta); yr = 1.0 + yr2; yi = yi2;
	for (i=2; i<=n2; i++) {
		i1 = i + i - 1;      i2 = i1 + 1;
		i3 = n + n + 3 - i2; i4 = i3 + 1;
		wrs = yr; wis = yi;
		xr1 = (s[i1] + s[i3])/2.0; xi1 = (s[i2] - s[i4])/2.0;
		xr2 = (s[i2] + s[i4])/2.0; xi2 = (s[i3] - s[i1])/2.0;
		s[i1] = xr1 + wrs * xr2 - wis * xi2;
		s[i2] = xi1 + wrs * xi2 + wis * xr2;
		s[i3] = xr1 - wrs * xr2 + wis * xi2;
		s[i4] = -xi1 + wrs * xi2 + wis * xr2;
		yr0 = yr;
		yr = yr * yr2 - yi  * yi2 + yr;
		yi = yi * yr2 + yr0 * yi2 + yi;
	}
	xr1 = s[1];
	s[1] = xr1 + s[2];
	s[2] = 0.0;
}


size_t PLP_NCC::MRound(size_t size)
{
	return ((size % FWORD) == 0)?size : (size/FWORD + 1) * FWORD;
}

DMatrix PLP_NCC::CreateDMatrix(int nrows,int ncols)
{
	size_t vsize;
	int *i,j;
	DVector *m;   
	char *p;

	size_t nAllocSize = (ncols+1)*sizeof(double)*nrows+(nrows+1)*sizeof(DVector);

	p = new char[MRound(nAllocSize)];
	//ASSERT1(p,"Error new space in CreateDMatrix()");
	i = (int *) p; *i = nrows;
	//vsize = DVectorElemSize(ncols);
	vsize = (ncols+1)*sizeof(double);
	m = (DVector *) p;
	p += MRound((nrows+1)*sizeof(DVector));
	for (j=1; j<=nrows; j++, p += vsize) {
		i = (int *) p; *i = ncols;
		m[j] = (DVector) p;
	}
	return m;
}

void  PLP_NCC::CreateShortVec(short **x,int size)
{   
	*x = new short[size+1];
//	ASSERT1(x,"Error new short in CreateShortVec");
	**x = (short)size;
}


void PLP_NCC::CreateVector(float **buf, int size)
{
	int *i;

	*buf = new float[size+1];
//	ASSERT1(buf,"Error new in CreateVector");

	i = (int *)(*buf); 
	*i = size;
	return;
}
int PLP_NCC::VectorSize(float *vector)
{
	int *i;

	i = (int *) vector;
	return *i;
}

void PLP_NCC::ZeroVector(float* v)
{
	int i,n;

	n=VectorSize(v);
	for (i=1;i<=n;i++) v[i]=0.0;
}


float PLP_NCC::WarpFreq (float fcl, float fcu, float freq, float minFreq, float maxFreq , float alpha)
{
	if ( fabs(alpha-1.0)<1.0e-7 )
		return freq;
	else {
		float scale = 1.0 / alpha;
		float cu = fcu * 2 / (1 + scale);
		float cl = fcl * 2 / (1 + scale);

		float au = (maxFreq - cu * scale) / (maxFreq - cu);
		float al = (cl * scale - minFreq) / (cl - minFreq);
		//  printf("cu %f cl %f au %f al %f\n", cu,cl,au,al);

		//	  if(freq <m_baseInfo.lowPass || freq >m_baseInfo.highPass+1)
		//	  {	  
		//		  printf("ori freq is out of range[60-3400]:%f\n", freq);
		//         exit(-1);
		//	  }

		if (freq > cu)
			return  au * (freq - cu) + scale * cu ;
		else if (freq < cl)
			return al * (freq - minFreq) + minFreq ;
		else
			return scale * freq ;

		//	float tmp=(1-alpha)*sin(freq)/(1-(1-alpha)*cos(freq));
		//	return freq+2/tan(tmp);	

		//	float tmp = pow(alpha,3*freq/8000)*freq;
		//	return tmp;
	}
}