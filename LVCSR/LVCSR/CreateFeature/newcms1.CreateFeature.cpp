
#include <string.h>
#include "CreateFeature.h"
// #include "mfcc.h"
// #include "plp.h"
// #include "rastaplp.h"
// #include "config.h"
// aConfigEnv *envs;
// static class MFCC2		*m_mfcc;
// static class PLP		*m_plp;
// static class RASTA_PLP	*m_rasta;
// FEATURE_BASEINFO fea_info;
// int				 m_targetKind;

//----------cfg-----------
void CreateFeature::SetEnv(aConfigEnv* &envs,char *config) {
   char *pt;
   while ((pt=strchr(config,'"'))) /*strcpy(pt,pt+1);*/memmove(pt,pt+1,STRINGLENGTH-1);
   if ((pt=strchr(config,'='))) {
      aConfigEnv *tmp=new aConfigEnv;
      tmp->next=envs; envs=tmp; 
      *pt='\0'; strcpy(tmp->env,config); 
      strcpy(tmp->def,pt+1); *pt='=';
   }
}

char *CreateFeature::GetEnv(char *env) {
   for (aConfigEnv *ev=envs;ev;ev=ev->next)
      if (!strcmp(ev->env,env)) return ev->def;
   ApiLog->logoutf2("find no %s in cfg, so try to getenv!\n", env);
   char *tmp=getenv(env); 
   if (!tmp)	return NULL;
   return tmp;
}

void CreateFeature::ReadConfig(char *line,int& num) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   sscanf(param,"%d",&num);
}

void CreateFeature::ReadConfig(char *line,int& num1,int& num2) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   sscanf(param,"%d %d",&num1,&num2);
}

void CreateFeature::ReadConfig(char *line,bool& bln) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   bln=(!strncmp(param,"true",4));
}

void CreateFeature::ReadConfig(char *line,float& num) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   sscanf(param,"%g",&num);
}

void CreateFeature::ReadConfig(char *line,float& num1,float& num2) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   sscanf(param,"%g,%g",&num1,&num2);
}

void CreateFeature::ReadConfig(char *line,float& num1,float& num2,float& num3) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   sscanf(param,"%g,%g,%g",&num1,&num2,&num3);
}

void CreateFeature::ReadConfig(char *line,float& n1,float& n2,float& n3,float& n4) {
	char *param;
	if ( !(param=GetEnv(line)) )	return;
   if (sscanf(param,"%g ,%g %g ,%g",&n1,&n2,&n3,&n4)<3) { n3=n1;n4=n2; }
}

void CreateFeature::ReadConfig(char *line,char *str) 
{
   str[0]='\0'; 
	char *param;
	if ( !(param=GetEnv(line)) )	return;

   sscanf(param,"%[^\n\r]s",str);
}

void CreateFeature::ReadConfig(char *line,char *str1,char *str2) 
{
	char *param;
	if ( !(param=GetEnv(line)) )	return;

	if (sscanf(param,"%s %s",str1,str2)<2) strcpy(str2,str1);
   if (!strcmp(str1,"null") || !strcmp(str1,"NULL")) str1[0]='\0';
   if (!strcmp(str2,"null") || !strcmp(str2,"NULL")) str2[0]='\0';
}

//----------------
void CreateFeature::CleanInfo(){
	if (m_mfcc) {
		delete m_mfcc;
	}
	if (m_plp) {
		delete m_plp;
	}
	if (m_rasta) {
		delete m_rasta;
	}
	if (mfcc) delete mfcc;
	if (gmm) delete gmm;
	if (feat) delete feat;
}
void CreateFeature::SetBaseInfo(FEATURE_BASEINFO p_info)
{
	ApiLog->logoutf3("//*****************************************//\n");

	if (strstr(p_info.targetKind,"MFCCPLP"))
	{
		m_targetKind = FYT_MFCCPLP;
		ApiLog->logoutf3(" Feature type is : MFCCPLP\n");
	}
	else if (strstr(p_info.targetKind,"MFCC"))
	{
		m_targetKind = FYT_MFCC;
		ApiLog->logoutf3(" Feature type is : MFCC\n");
	}
    else if (strstr(p_info.targetKind,"RASTAPLP"))
	{
		m_targetKind = FYT_RASTAPLP;
		ApiLog->logoutf3(" Feautre type is : RASTAPLP\n");
	}
	else
	{
		ApiLog->logoutf2("Error input targetKind!!!\n");	exit(-1);
	}
	if (p_info.bPitch) {
		ApiLog->logoutf3("Pitch added to the last vec\n");
	}
	ApiLog->logoutf3("targetKind=%s\n",p_info.targetKind);
    ApiLog->logoutf3("smpPeriod=%d\n",p_info.smpPeriod);
    ApiLog->logoutf3("framePeriod=%d\n",p_info.framePeriod);	
	
	switch(m_targetKind) {
	case FYT_MFCC:
		m_mfcc = new MFCC2;
		if (!m_mfcc)
		{
			ApiLog->logoutf("Error new MFCC!\n");	exit(-1);
		}
		m_mfcc->SetBaseInfo(p_info);
		m_mfcc->Initialize(BaseDim);

	    if (p_info.zeroGlobalMean)
	    	ApiLog->logoutf3("zeroGlobalMean=true\n");
		else
    		ApiLog->logoutf3("zeroGlobalMean=false\n");
		ApiLog->logoutf3("chlNum=%d\n",p_info.chlNum);
		ApiLog->logoutf3("cepNum=%d\n",p_info.cepNum);
		ApiLog->logoutf3("cepLifter=%d\n",p_info.cepLifter);
		ApiLog->logoutf3("winSize=%d\n",p_info.winSize);

		ApiLog->logoutf3("lowPass=%.0f\n",p_info.lowPass);
		ApiLog->logoutf3("highPass=%.0f\n",p_info.highPass);
		if (p_info.normEnergy)
		{
			ApiLog->logoutf3("normEnergy=true\n");
			ApiLog->logoutf3("energyScale=%.0f\n",p_info.energyScale);
			ApiLog->logoutf3("silFloor=%.0f\n",p_info.silFloor);
		}		
		else
			ApiLog->logoutf3("normEnergy=false\n");

		break;
	case FYT_MFCCPLP:
		m_plp = new PLP;
		if (!m_plp)
		{
			ApiLog->logoutf2("Error new MFCCPLP\n");	exit(-1);
		}
		m_plp->SetBaseInfo(p_info);	
		m_plp->Initialize(BaseDim);

		if (p_info.zeroGlobalMean)
    		ApiLog->logoutf3("zeroGlobalMean=true\n");
		else
    		ApiLog->logoutf3("zeroGlobalMean=false\n");
		ApiLog->logoutf3("chlNum=%d\n",p_info.chlNum);
		ApiLog->logoutf3("cepNum=%d\n",p_info.cepNum);
		ApiLog->logoutf3("cepLifter=%d\n",p_info.cepLifter);
		ApiLog->logoutf3("winSize=%d\n",p_info.winSize);

		ApiLog->logoutf3("lowPass=%.0f\n",p_info.lowPass);
		ApiLog->logoutf3("highPass=%.0f\n",p_info.highPass);

		ApiLog->logoutf3("lpcOrder=%d\n",p_info.lpcOrder);
		ApiLog->logoutf3("compressFact=%.3f\n",p_info.compressFact);

		break;
	case FYT_RASTAPLP:
		m_rasta = new RASTA_PLP;
		if (!m_rasta)
		{
			ApiLog->logoutf2("Error new RASTA_PLP\n");	exit(-1);
		}
		m_rasta->SetBaseInfo(p_info);
		m_rasta->Initialize(BaseDim);
		if (p_info.bDoRasta)
			ApiLog->logoutf3("bDoRasta=true\n");
		else
			ApiLog->logoutf3("bDoRasta=false\n");
		
		if (p_info.bDoCep)
			ApiLog->logoutf3("bDoCep=true\n");
		else
			ApiLog->logoutf3("bDoCep=false\n");
			
		if (p_info.bgainflag)
			ApiLog->logoutf3("bgainflag=true\n");
		else
			ApiLog->logoutf3("bgainflag=false\n");

		ApiLog->logoutf3("lpcOrder=%d\n",p_info.lpcOrder);
		
		ApiLog->logoutf3("stepsize=%d\n",p_info.stepsize);
		
		ApiLog->logoutf3("fpole=%.3f\n",p_info.fpole);
		ApiLog->logoutf3("fcepLifter=%.3f\n",p_info.fcepLifter);

		break;		
	}
	if(cms_strategy == 0){
		FrameLag = bHLDA?6:4;
	}else{
		FrameLag = bHLDA?(cms_delayNum+6):(cms_delayNum+4);
		cms_frameNum = 0;
	}
	int tgtDim = (bHLDA?4:3)*BaseDim;
	int tgtDim4 = (tgtDim + 3) & (~3);
	Buffer = new float[(FrameLag+2)*tgtDim4];
	mBuffer = new MultiBuffer<float>;
	FrameCnt = 0;
	BufferWav = new short[winSize];
	SampleCnt = 0;
	mBufferWav = new MultiBuffer<short>;
    ASSERT3(m_targetKind&KINDMASK,"Unsupported target kind: %s",p_info.targetKind);

	ApiLog->logoutf3("//*****************************************//\n");
}

void CreateFeature::AddWaveData(short *waveData,int smpNum)
{
	switch(m_targetKind) {
	case FYT_MFCC:
		m_mfcc->AddWaveData(waveData,smpNum);
		break;
	case FYT_MFCCPLP:
		m_plp->AddWaveData(waveData,smpNum);
		break;
	case FYT_RASTAPLP:
		m_rasta->AddWaveData(waveData,smpNum);
		break;
	}
}

void CreateFeature::WriteFile(char *outFileName)
{
	switch(m_targetKind) {
	case FYT_MFCC:
		m_mfcc->WriteFile(outFileName);
		break;
	case FYT_MFCCPLP:
		m_plp->WriteFile(outFileName);
		break;
	case FYT_RASTAPLP:
		m_rasta->WriteFile(outFileName);
		break;
	}	
}
void CreateFeature::InitFeatureCfg(char *systemDir, char *cfgFileName, bool bOnlineMode){
	SampleRate = TBNR_SampleRate();
	/////////////////////////////////////
	int nPathLen = strlen(systemDir);
	FILE *fCFG;
	fCFG = fopen(cfgFileName,"rt");
	if (!fCFG)
	{
		ApiLog->logoutf("Error open %s for read!\n",cfgFileName); exit(0);
	}
	/*static*/ stringbuf1 config="";
	envs = NULL;   // a bug when F5 run by jgao on 64bit PC, 080314 syq
	while (!feof(fCFG)) 
	{
		if (fscanf(fCFG,"\n %[^\n]s",config)<1) break;
		SetEnv(envs,config);
	}
	fclose(fCFG);

	ReadConfig("FYT_TargetKind",fea_info.targetKind); 
	
	//ReadConfig("FYT_SourceRate",fea_info.smpPeriod);
	fea_info.smpPeriod = 10000000 / SampleRate;   // syq 070807 add for different SampleRate
	ReadConfig("FYT_TargetRate",fea_info.framePeriod);
	ReadConfig("FYT_ZeroGlobalMean",fea_info.zeroGlobalMean);// remove time-domain mean 
	ReadConfig("FYT_WindowSize",fea_info.winSize);
	
	frameRate=fea_info.framePeriod/fea_info.smpPeriod;
	winSize=fea_info.winSize/fea_info.smpPeriod;
	ReadConfig("FYT_ChannelNum",fea_info.chlNum);
	ReadConfig("FYT_CepstrumLifter",fea_info.cepLifter);
	ReadConfig("FYT_CepstrumNum",fea_info.cepNum);
	ReadConfig("FYT_LowFreq",fea_info.lowPass);	// used by fft
	ReadConfig("FYT_HighFreq",fea_info.highPass);
	ReadConfig("FYT_NormEnergy",fea_info.normEnergy);
	ReadConfig("FYT_EnergyScale",fea_info.energyScale);
	ReadConfig("FYT_SilFloor",fea_info.silFloor);

	ReadConfig("FYT_LPCOrder",fea_info.lpcOrder);
	ReadConfig("FYT_CompressFact",fea_info.compressFact);
	ReadConfig("FYT_DoRASTA",fea_info.bDoRasta);
	ReadConfig("FYT_DoCep",fea_info.bDoCep);
	ReadConfig("FYT_DoGain",fea_info.bgainflag);
	ReadConfig("FYT_StepSize",fea_info.stepsize);
	ReadConfig("FYT_FPole",fea_info.fpole);
	ReadConfig("FYT_fCepLifter",fea_info.fcepLifter);

	ReadConfig("FYT_Pitch",fea_info.bPitch);
	ReadConfig("FYT_bHLDA",bHLDA);

	Matrix = NULL;
	m_mfcc=NULL; m_plp=NULL; m_rasta=NULL;
	if (bHLDA) {
		char HLDA_Matrix[256];
		strcpy(HLDA_Matrix, systemDir);
		ReadConfig("FYT_HLDA_Matrix", HLDA_Matrix + nPathLen);
		FILE *matrixfile=fopen(HLDA_Matrix,"rb");
		if (!matrixfile)
		{
			ApiLog->logoutf("error opening %s file! nPathLen=%d, systemDir=%s\n", HLDA_Matrix, nPathLen, systemDir); exit(1);
		}	
		fread(&matrix_h,sizeof(int),1,matrixfile);
		fread(&matrix_n,sizeof(int),1,matrixfile);
		Matrix = new double[matrix_h*matrix_n];		
		fread(Matrix,sizeof(double),matrix_h*matrix_n,matrixfile);
		fclose(matrixfile);
	}
	bonlinecms = false;// lita [4/9/2010]
	ReadConfig("FYT_ONLINE", bonlinecms);
	/*cmslen = NULL;
	if (bonlinecms) {
		ApiLog->logoutf3("use online cms!\n");
		char CMS_bin[256];
		strcpy(CMS_bin, systemDir);
		ReadConfig("FYT_CMS_bin", CMS_bin+ nPathLen);
		FILE *fin_lu=fopen(CMS_bin,"rb");
		if (!fin_lu)
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
	}//add by llu
	// lita [4/9/2010]
	else ApiLog->logoutf3("use offline cms!\n");*/
	if (bOnlineMode || bonlinecms)
	{
		ApiLog->logoutf3("use online cms!\n");
		char CMS_bin[256];
		strcpy(CMS_bin, systemDir);
		ReadConfig("FYT_CMS_bin", CMS_bin+ nPathLen);
		cmsobj = new OnLineCms(-1, CMS_bin);

		cms_strategy=0;
		ReadConfig("FYT_CMS_Strategy", cms_strategy);
		cms_delayNum=0;
		ReadConfig("FYT_CMS_Delay_Num", cms_delayNum);
	
		cmsobj->ResetInitCmsValue(cms_delayNum);
		bonlinecms = true;// lita [4/9/2010]

	}
	else ApiLog->logoutf3("use offline cms!\n"), cmsobj = NULL, bonlinecms = false;

	SetBaseInfo(fea_info);

	src = NULL;
	tgt = NULL;
	if (bHLDA) {
		hldatgt = NULL;
	}
	ApiLog->logoutf3("Begin to init feat and Gmm for Gender Classification:\n");
//	m_mfcc=NULL; m_plp=NULL; m_rasta=NULL;
	mfcc = NULL;
	mfcc = new MFCC2;   // [6/26/2007 copyed from Segment by syq]
	if (!mfcc)	{
		ApiLog->logoutf("Error new MFCC!\n");	exit(-3);
	}
	FEATURE_BASEINFO gmm_featInfo;
	memcpy(&gmm_featInfo,&fea_info,sizeof(FEATURE_BASEINFO));
	gmm_featInfo.cepNum=16;
	sprintf(gmm_featInfo.targetKind,"MFCC");
	//ApiLog->logoutf3("\t1.1,Begin mfcc SetBaseInfo;\n");
	mfcc->SetBaseInfo(gmm_featInfo);
	//ApiLog->logoutf3("\t1.2,Begin mfcc Initialize;\n");
	int BaseDim2;
	mfcc->Initialize(BaseDim2);
	//ApiLog->logoutf3("\t1.3,Finish mfcc Initialize;\n");
	char GmmModel[256];
	strcpy(GmmModel, systemDir);
	ReadConfig("GmmModel", GmmModel + nPathLen);
	//ApiLog->logoutf3("\tget GmmModel %s;\n", GmmModel);
	if (!strcmp(GmmModel,systemDir)) GmmModel[0]='\0';   // 080218 jgao found bug using pitch am
//	strcpy(GmmModel, cfgFileName);
// 	char *pt = NULL;
// 	char *pt1=strrchr(GmmModel,'/');
// 	if (pt1) pt = pt1;
// 	char *pt2=strrchr(GmmModel,'\\');
// 	if (pt2 && pt2 > pt1) pt = pt2;
// 	if (!pt) pt = GmmModel;
// 	ReadConfig("GmmModel", pt);
	ReadConfig("VoiceCoef", VoiceCoef);
	if(GmmModel[0]){
		//ApiLog->logoutf3("\t2,Begin to load GmmModel %s;\n", GmmModel);
		gmm = new iGmmParam(GmmModel);
		if (!gmm)	{
			ApiLog->logoutf("Error new iGmmParam!\n");	exit(-1);
		}
		else ApiLog->logoutf3("GmmModel \"%s\": has %d classes, %d mixture, vector size = %d\n", GmmModel, gmm->header->gmmNum, gmm->header->mixNum, gmm->header->vecSize);
		m_nGmmNum = gmm->header->gmmNum;
		switch(gmm->header->vecSize/mfcc->GetBaseDim()) {
		case 1:
			ApiLog->logoutf3("GmmModel vector is : MFCC \n");		break;
		case 2:
			ApiLog->logoutf3("GmmModel vector is : MFCC + delt\n");		break;
		case 3:
			ApiLog->logoutf3("GmmModel vector is : MFCC + delt + deltdelt\n");		break;
		default:
			ApiLog->logoutf2("Error : GMMModel_vecSize=%d  cepNum=%d\n",gmm->header->vecSize,gmm_featInfo.cepNum);
			gmm=NULL;   // exit(-2);   080303 syq add
		}
		//ApiLog->logoutf3("\t3,Begin to new Feature2;\n");
		feat = new Feature2(gmm->header->vecSize,false,false);
		if (!feat)	{
			ApiLog->logoutf2("Error new Feature!\n");	exit(-1);
		}
	}
	else gmm=NULL, feat=NULL;
	ApiLog->logoutf3("Finish init feat and Gmm for Gender Classification!\n");

}
void CreateFeature::CleanFeature(){

	CleanInfo();
	if (Matrix) {
		delete [] Matrix;
	}
	if (tgt) {
//		delete [] tgt;
	}
	if (hldatgt && bHLDA) {
//		delete [] hldatgt;   // [6/27/2007 added by syq] make mem to be controled by user (so could be used when decoding so long)
	}
	//if (envs) delete envs;
	for (aConfigEnv *ev=envs;ev;)
	{
		aConfigEnv *temp = ev;
		ev = ev->next;
		delete temp;
	}
//	if (src) delete []src;   // 内部mfcc或者plp管理

	// lita [4/9/2010]
	//if (bonlinecms) delete [] cmslen, cmslen = NULL;// llu [3/30/2010]
	if (cmsobj) delete(cmsobj), cmsobj = NULL;
	if (Buffer) delete []Buffer, Buffer = NULL;
	if (mBuffer) delete(mBuffer), mBuffer = NULL;
	if (BufferWav) delete []BufferWav, BufferWav = NULL;
	if (mBufferWav) delete(mBufferWav), mBufferWav = NULL;
}

float *CreateFeature::GetFeature(short *waveData,int smpNum,FeatureHeader *header,bool bBaseOnly,TSR_STREAM_FLAG flag,int *residue){

	// 070620 syq add
	header->framePeriod = fea_info.framePeriod;
	header->mfccKind = m_targetKind;
// 	if (waveData == NULL || smpNum <= 0) {
// 		header->frameNum = 0;
// 		return NULL;
// 	}
	int srcDim;
if (waveData && smpNum>0) { // deal with negative input
	if (!bonlinecms) FrameBack = 0;
	else if (bonlinecms&&(flag==TSR_STREAM_START||flag==TSR_STREAM_ALL)){
		cmsobj->ResetInitCmsValue(cms_delayNum), FrameCnt = 0, SampleCnt = 0, FrameBack = 0;//may be a bug?FrameBack shou no be 0?
		cms_frameNum = 0;
	}
	// if (src) delete [] src;
	mBufferWav->Initialize(1,BufferWav,SampleCnt,waveData,smpNum);
	waveData = NULL;
	bool bforce = true;
	smpNum = mBufferWav->framenum();
	mBufferWav->getBuffer(0,smpNum,bforce,waveData);
	switch(m_targetKind) {
	case FYT_MFCC:
		m_mfcc->AddWaveData(waveData,smpNum,residue);
		src=m_mfcc->GetFeatureDate(&header->frameNum,&header->frameSize);
		break;
	case FYT_MFCCPLP:
		m_plp->AddWaveData(waveData,smpNum,residue);
		src=m_plp->GetFeatureDate(&header->frameNum,&header->frameSize);
		break;
	case FYT_RASTAPLP:
		m_rasta->AddWaveData(waveData,smpNum,residue);
		src=m_rasta->GetFeatureDate(&header->frameNum,&header->frameSize);	
		break;
	}
	SampleCnt = CalResidue(smpNum);
	mBufferWav->getBuffer(smpNum-SampleCnt,SampleCnt,bforce,BufferWav);
	delete []waveData;
	srcDim = header->frameSize/sizeof(float);
	switch(m_targetKind) { // 091117 add
	case FYT_MFCCPLP:
	case FYT_RASTAPLP:
		int ii,kk;
		float *ft;
		for (ii=0,ft=src;ii<header->frameNum;ii++,ft+=srcDim) 
		{
			for (kk=0;kk<srcDim;kk++) 
			{
				//if (k == m_BaseDim-1) {
				// }else{
				ft[kk] *= 10.f;	// 2004.11.04 plu : plp参数的量级太小，加倍之
				if (ft[kk]>DYN_RANGE) ft[kk]=DYN_RANGE;
				if (ft[kk]<-DYN_RANGE) ft[kk]= -DYN_RANGE;
				//}
			}
		}
	break;
	}
	if (bBaseOnly) return src;
	if (flag==TSR_STREAM_ALL && (!src || header->frameNum<3) || flag==TSR_STREAM_START && (!src || header->frameNum<=0))
	{
		header->frameNum = 0;
		return NULL;
	}
}
else header->frameNum = 0, srcDim = BaseDim;

	header->frameSize = 3*srcDim*sizeof(float);
	int tgtDim;
	if (bHLDA) {
		tgtDim = 4*srcDim;
	}else{
		tgtDim = 3*srcDim;
	}
	int tgtDim4 = (tgtDim + 3) & (~3);
	// deal with last buffer
	int FrameLast=FrameBack;//min(FrameCnt,FrameLag+2);//The frames stored by last process for current process.
//	if(cms_strategy == 0)
//		int FrameToPad=FrameBack>2?min(6,FrameLast-2):0;//The frames needed moving from preious buffer to current buffer for computing delta part.
//	else
	int FrameToPad = FrameBack>2?min(cms_delayNum+6, FrameLast-2):0;
	tgt = (float*)Malloc32((header->frameNum+FrameToPad)*tgtDim4*sizeof(float), false);   // new float [header->frameNum*tmpDim]; syq 070706
	if (FrameToPad>0) memcpy(tgt, Buffer+(FrameLast-FrameToPad)*tgtDim4, tgtDim4*FrameToPad*sizeof(float));
	for (int i=0;i<header->frameNum;i++)
	{
		memcpy(tgt+tgtDim4*(i+FrameToPad),src+i*srcDim,srcDim*sizeof(float));
	}
	header->frameNum+=FrameToPad;
	FrameLast-=FrameToPad;

	const int DELWIN=2,ACCWIN=2;
	const float SIGMAD2=10.0,SIGMAA2=10.0;

	int delta;
	//MultiBuffer<float> *mBuffer = new MultiBuffer<float>(tgtDim4,Buffer,FrameLast,tgt,header->frameNum);
	mBuffer->Initialize(tgtDim4,Buffer,FrameLast,tgt,header->frameNum);
	for(delta = 0; delta < tgtDim/srcDim-1; delta++){
		for (int i=FrameLast;i<mBuffer->framenum();i++)
		{
			for (int j=0;j<srcDim;j++)
			{
				double sum=0;
				for (int k=1;k<=DELWIN;k++)
				{
					//float *ffeat=tgt+min(i+k,header->frameNum-1)*tgtDim4+delta*srcDim;
					//float *ffeat=(float*)(mBuffer+(i+k))+delta*srcDim;
					//float *bfeat=(float*)(mBuffer+(i-k))+delta*srcDim;
					float *ffeat=mBuffer->get(i+k)+delta*srcDim;
					float *bfeat=mBuffer->get(i-k)+delta*srcDim;
					//if (flag==TSR_STREAM_CONT && i-k<0)
					//	bfeat=tgt+min(i-k,0)*tgtDim4+delta*srcDim;
					//else bfeat=Buffer+max(FrameLast+i-k,0)*tgtDim4+delta*srcDim;
					sum+=k*(ffeat[j]-bfeat[j]);
				}
				//tgt[i*tgtDim4+delta*srcDim+srcDim+j]=float(sum*(1.0/SIGMAD2));
				//mBuffer[i,(delta+1)*srcDim+j]=float(sum*(1.0/SIGMAD2));
				//*((float*)(mBuffer+i)+(delta+1)*srcDim+j)=float(sum*(1.0/SIGMAD2));
				mBuffer->get(i,(delta+1)*srcDim+j)=float(sum*(1.0/SIGMAD2));
			}
		}
	}
	if(!bonlinecms)
	{
		for (int j = 0; j < tgtDim; j++) 
		{ 
			double res;
			double Meanacc = 0.0; 
			double Varacc  = 0.0; 
			
			int frm = 0;
			for (frm = 0; frm < header->frameNum; frm++ )
			{
				res = tgt[frm*tgtDim4 + j];
				Meanacc += res; 
				Varacc += res*res; 
			}
			
			double mean,var;
			mean = Meanacc/(double)header->frameNum;
			if (header->frameNum > 1)
				var = sqrt((Varacc - mean*Meanacc)/(double)(header->frameNum-1));
			else 
				var = double(1.0);
			if(var == 0.0f) var = double(1.0);
			for ( frm=0; frm < header->frameNum; frm++ )
			{
				res = tgt[frm*tgtDim4 + j];
				tgt[frm*tgtDim4 + j] = float((res - mean)/var);
			}
		}
	}
	else
	{
		// 由于Delta是在cms之前做的，所以要提前保存；
		int FrameTotal = FrameLast + header->frameNum;//total frames that will be calculated cms
		FrameBack  = min(FrameTotal, FrameLag+2);//save some frames for next data, 2 frames overlap with current cms computing.
		bool bforce = true;
		mBuffer->getBuffer(mBuffer->framenum()-FrameBack, FrameBack, bforce, Buffer);
		/*if (header->frameNum>=2+FrameLag)
			memcpy(Buffer, tgt+(header->frameNum-2-FrameLag)*tgtDim4, (2+FrameLag)*tgtDim4*sizeof(float));
		else
		{
			int FrameTotal = FrameLast + header->frameNum;
			int FrameKept  = min(FrameLast, 2+FrameLag-header->frameNum);
			memmove(Buffer, Buffer+(FrameLast-FrameKept)*tgtDim4, FrameKept*tgtDim4*sizeof(float));
			memcpy(Buffer+(2+FrameLag-header->frameNum)*tgtDim4, tgt, header->frameNum*tgtDim4*sizeof(float));
		}*/
		//header->frameNum = mBuffer->framenum();
		if(cms_strategy == 0){
			if (!(flag==TSR_STREAM_END||flag==TSR_STREAM_ALL)) header->frameNum-=FrameLag;//if not end, left Framelag without cms.
			FrameCnt += header->frameNum;
			if (header->frameNum < 0) return NULL;
			cmsobj->OnlineCmsCal(tgt, tgtDim, tgtDim4, header->frameNum);
		}else{
			if (!(flag==TSR_STREAM_END||flag==TSR_STREAM_ALL)){
				cmsobj->OnlineCmsDelay(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum-FrameLag, &cms_frameNum);
				header->frameNum-=FrameLag;
			}else{
				cmsobj->OnlineCmsDelay(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum, &cms_frameNum);
			}
		}
		/*// 081115 onlinecms add from llu
		//printf("use online cms!\n");
		//printf("cmslen=%d,tgtdim=%d,tgtdim4=%d\n",*(int*)cmslen,tgtDim,tgtDim4);
		if (*(int*)cmslen != tgtDim)
			printf("Error!!! tmpDim not equal to the cmslen\n");
		int j;
		//printf("frameNum = %d\n",header->frameNum);
		
		for (j = 0; j < tgtDim; j++) 
		{ 
		
// 		  float res;
// 		  float iniMean = *((float *)cmslen+j+1);
// 		  float iniVar  = *((float *)cmslen+tgtDim+j+1);
// 		  float mean,var;
			
			double res;
			double iniMean = *((float *)cmslen+j+1);
			double iniVar  = *((float *)cmslen+tgtDim+j+1);
			
			double mean,var;
			int frm;
			for ( frm=0; frm < header->frameNum; frm++ )
			{
				res = tgt[frm*tgtDim4 + j];
				mean = iniMean + 0.1f * (res-iniMean);
				iniVar = iniVar + 0.1f * ((res-iniMean)*(res-iniMean)-iniVar);
				var = sqrt(iniVar);
				tgt[frm*tgtDim4 + j] = float((res - mean)/(var+1.0f)*4.0f);
				iniMean = mean;
			}
		}*/
	}
	if(bHLDA){
		hldatgt = NULL;
		if(header->frameNum > 0){
			int htgtDim4 = (srcDim * 3 + 3)&(~3);
			hldatgt = (float *)Malloc32(header->frameNum* htgtDim4, int(sizeof(float) ), true);   // hldatgt = new float [3*header->frameNum*srcDim]; syq 070706
			int i;
			for (i=0; i<header->frameNum; i++) {
				int k;
				for (k=0;k<matrix_n;k++){
					hldatgt[htgtDim4*i+k] = 0;
					int j;
					for(j = 0;j < matrix_h;j++){
						hldatgt[htgtDim4*i+k] += tgt[i*tgtDim4+j]*Matrix[k*matrix_h+j]; //mBuffer->get(i,j)*Matrix[k*matrix_h+j];
					}
				}
			}
		}
		if (tgt) Free32(tgt), tgt = NULL;      // [6/29/2007 added by syq] 说明：提供给外部使用，不知何时结束（例如解码）所以对外的内存不在此处释放！
		return hldatgt;
	}else{
		float *temp = Buffer;
		return tgt;
	}
}

// 070620 syq move from main
void CreateFeature::WriteFile(char *outFileName, float *tgtfeature, FeatureHeader header, bool bAlign4, bool bAppend)
{
	if (tgtfeature==NULL) return;
	int nAlign4 = 0;
	if (bAlign4) nAlign4 = 4 - header.frameSize/sizeof(float) % 4;
	FILE *fout; int i,k; float *ft;
	char outFileFullName[256] = "";
	strcpy(outFileFullName, outFileName);
	switch(m_targetKind) {
	case FYT_MFCC:
		strcat(outFileFullName, ".mfc");		break;
	case FYT_MFCCPLP:
		strcat(outFileFullName, ".plp");		break;
	case FYT_RASTAPLP:
		strcat(outFileFullName, ".rplp");		break;
	}
	strcpy(outFileName, outFileFullName);
	if (!bAppend || access(outFileFullName,0)!=0)
	{
		fout=fopen(outFileFullName, "wb");
		if (fout) fwrite(&header.frameNum,sizeof(int),1,fout);
	}
	else
	{
		fout=fopen(outFileFullName, "r+b");
		FeatureHeader oriheader;
		fread(&oriheader,sizeof(FeatureHeader),1,fout);
		if (oriheader.frameNum<0 || oriheader.frameSize!=header.frameSize)
		{
			printf("error in WriteFile %s\n", outFileFullName);
			fclose(fout);
			exit(3);
		}
		if (fout) fseek(fout, 0, SEEK_SET);
		oriheader.frameNum+=header.frameNum;
		fwrite(&oriheader.frameNum,sizeof(int),1,fout);
	}

	//fwrite(&header.frameNum,sizeof(int),1,fout);
	fwrite(&header.framePeriod,sizeof(int),1,fout);
	fwrite(&header.frameSize,sizeof(short),1,fout);
	fwrite(&header.mfccKind,sizeof(short),1,fout);
	fflush(fout);
	fseek(fout, 0, SEEK_END);
	for (i=0,ft=tgtfeature;i<header.frameNum;i++,ft+=header.frameSize/sizeof(float) + nAlign4) 
	{
		for (k=0;k<header.frameSize/sizeof(float);k++) {
			if (ft[k]>50) ft[k]=50;
			if (ft[k]<-50) ft[k]= -50;
		}
		fwrite(ft,sizeof(float),header.frameSize/sizeof(float),fout);
	}
   fclose(fout);
}

///////////////////////////////////////////////////////////////////
//函数名       FindSegType
//
//输入参数说明 :none
//
//返回值说明   :bool 分析是否成功
//
//功能说明     :  GMM分类，语段类型
// 
///////////////////////////////////////////////////////////////////
bool CreateFeature::FindSegType(short *waveData,int smpNum,SOUND_TYPE *myType)//fpan change 2005-06-11
{
	if (!gmm)
	{
		*myType=MALE;
		return true;
	}
	float fRst,fMax;
	int nMaxGmmNo;
	int nFmNum;
	if (waveData && smpNum > 0 && myType)
	{
		if (smpNum<SampleRate*0.1) //长度小于0.1秒为噪音
		{
			*myType = NOISE;
			return true;
		}
		mfcc->AddWaveData(waveData, smpNum);
		
		///////////////////fpan add 2005-06-11
		int mfccFrameNum=mfcc->GetFrameNumber();
		if (mfccFrameNum<=3)
		{
			*myType = MALE;
			return false;
		}
		///////////////////////////////////////////////
		
		feat->CalNewFeature(mfcc->GetFrameNumber(),mfcc->GetBaseDim(),mfcc->GetMfccData());
		// free(mfcc->GetMfccData());   // 070907 syq leakage bug part 1 !
		nFmNum=feat->GetFrameNumber();
		gmm->AdjustVecSize(nFmNum);
		
		// 计算每一帧的GMM score,默认四类，但是对会议效果剪掉反而不好，可以只分男女！061115
		// 考虑对各类加权，这样可能效果更好，通过大的加权相当于排除噪声、音乐。
		for (int nGmm=0;nGmm<m_nGmmNum;nGmm++)
		{ // 都是负数，所以要除以
			fRst=gmm->CalculateProb(nGmm,feat->GetObv(0),nFmNum);
			if(nGmm>=2)
				fRst*=VoiceCoef;
				/*			if(nGmm>=2 && fRst>fMax){
				FILE * fp =fopen("noise.txt","a");
				fprintf(fp, "%f,%f:%f\n", fMax, fRst, fMax/fRst);
				fclose(fp);
		}*/
		/*			if (nGmm==0)
		{
		fMax=fRst;	nMaxGmmNo=0;
		}
			else*/
			if (nGmm==0 || fMax<fRst && nGmm<2)
			{
				fMax=fRst;	nMaxGmmNo=nGmm;
			}
		}
		
		*myType = (SOUND_TYPE)nMaxGmmNo;
		return true;
	}
	else return false;
}

int CreateFeature::CalResidue(int smpNum, int *frameNum)
{
	int FrameNum = (smpNum-winSize)/frameRate+1; // 只少不多
	if (frameNum) *frameNum = FrameNum;
	//int residue = smpNum-(FrameNum-1)*frameRate-winSize;
	int residue = smpNum-max(0,FrameNum)*frameRate;
	return residue;
}

int CreateFeature::CalRemainder(int &smpNum)
{
	int residue = winSize-frameRate;
	if (smpNum>=residue) smpNum-=residue;
	else residue=smpNum, smpNum=0;
	return residue;
}