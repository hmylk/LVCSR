
#include <string.h>
#include "CreateFeature.h"
// #include "mfcc.h"
// #include "plp.h"
// #include "rastaplp.h"
// #include "config.h"
// aConfigEnv *envs;
// static class MFCC2   *m_mfcc;
// static class PLP   *m_plp;
// static class RASTA_PLP *m_rasta;
// FEATURE_BASEINFO fea_info;
// int         m_targetKind;

extern int ElementsInSrcFeature;

//----------cfg-----------
void CreateFeature::SetEnv(aConfigEnv *&envs, char *config) {
  char *pt;
  while ((pt = strchr(config, '"'))) /*strcpy(pt,pt+1);*/memmove(pt, pt + 1, STRINGLENGTH - 1);
  if ((pt = strchr(config, '='))) 
  {
    aConfigEnv *tmp = new aConfigEnv;
    tmp->next = envs;
    envs = tmp;
    *pt = '\0';
    strcpy(tmp->env, config);
    strcpy(tmp->def, pt + 1);
    *pt = '=';
  }
}

char *CreateFeature::GetEnv(char *env) {
  for (aConfigEnv *ev = envs; ev; ev = ev->next)
    if (!strcmp(ev->env, env)) return ev->def;
  ApiLog->logoutf2("find no %s in cfg, so try to getenv!\n", env);
  char *tmp = getenv(env);
  if (!tmp)  return NULL;
  return tmp;
}

void CreateFeature::ReadConfig(char *line, int &num) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  sscanf(param, "%d", &num);
}

void CreateFeature::ReadConfig(char *line, int &num1, int &num2) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  sscanf(param, "%d %d", &num1, &num2);
}

void CreateFeature::ReadConfig(char *line, bool &bln) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  bln = (!strncmp(param, "true", 4));
}

void CreateFeature::ReadConfig(char *line, float &num) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  sscanf(param, "%g", &num);
}

void CreateFeature::ReadConfig(char *line, float &num1, float &num2) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  sscanf(param, "%g,%g", &num1, &num2);
}

void CreateFeature::ReadConfig(char *line, float &num1, float &num2, float &num3) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  sscanf(param, "%g,%g,%g", &num1, &num2, &num3);
}

void CreateFeature::ReadConfig(char *line, float &n1, float &n2, float &n3, float &n4) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;
  if (sscanf(param, "%g ,%g %g ,%g", &n1, &n2, &n3, &n4) < 3) {
    n3 = n1;
    n4 = n2;
  }
}

void CreateFeature::ReadConfig(char *line, char *str) {
  str[0] = '\0';
  char *param;
  if ( !(param = GetEnv(line)) )  return;

  sscanf(param, "%[^\n\r]s", str);
}

void CreateFeature::ReadConfig(char *line, char *str1, char *str2) {
  char *param;
  if ( !(param = GetEnv(line)) )  return;

  if (sscanf(param, "%s %s", str1, str2) < 2) strcpy(str2, str1);
  if (!strcmp(str1, "null") || !strcmp(str1, "NULL")) str1[0] = '\0';
  if (!strcmp(str2, "null") || !strcmp(str2, "NULL")) str2[0] = '\0';
}

//----------------
void CreateFeature::CleanInfo() {
  if (m_mfcc) {
    delete m_mfcc;
  }
  if (m_plp) {
    delete m_plp;
  }
  if (m_plp_ncc) delete m_plp_ncc;
  if (m_rasta) {
    delete m_rasta;
  }
  if (mfcc) delete mfcc;
  if (gmm) delete gmm;
  if (feat) delete feat;
}
void CreateFeature::SetBaseInfo(FEATURE_BASEINFO p_info) {
  ApiLog->logoutf3("//*****************************************//\n");

  if (strstr(p_info.targetKind, "MFCCPLP_NCC")) {
    m_targetKind = FYT_MFCCPLP_NCC;
    ApiLog->logoutf3(" Feautre type is : MFCCPLP_NCC\n");
  } else if (strstr(p_info.targetKind, "MFCCPLP")) {
    m_targetKind = FYT_MFCCPLP;
    ApiLog->logoutf3(" Feature type is : MFCCPLP\n");
  } else if (strstr(p_info.targetKind, "MFCC")) {
    m_targetKind = FYT_MFCC;
    ApiLog->logoutf3(" Feature type is : MFCC\n");
  } else if (strstr(p_info.targetKind, "RASTAPLP")) {
    m_targetKind = FYT_RASTAPLP;
    ApiLog->logoutf3(" Feautre type is : RASTAPLP\n");
  } else {
    ApiLog->logoutf2("Error input targetKind!!!\n");
    exit(-1);
  }
  if (p_info.bPitch) {
    ApiLog->logoutf3("Pitch added to the last vec\n");
  }
  ApiLog->logoutf3("targetKind=%s\n", p_info.targetKind);
  ApiLog->logoutf3("smpPeriod=%d\n", p_info.smpPeriod);
  ApiLog->logoutf3("framePeriod=%d\n", p_info.framePeriod);

  switch (m_targetKind) {
  case FYT_MFCC:
    m_mfcc = new MFCC2;
    if (!m_mfcc) {
      ApiLog->logoutf("Error new MFCC!\n");
      exit(-1);
    }
    m_mfcc->SetBaseInfo(p_info);
    m_mfcc->Initialize(BaseDim);

    if (p_info.zeroGlobalMean)
      ApiLog->logoutf3("zeroGlobalMean=true\n");
    else
      ApiLog->logoutf3("zeroGlobalMean=false\n");
    ApiLog->logoutf3("chlNum=%d\n", p_info.chlNum);
    ApiLog->logoutf3("cepNum=%d\n", p_info.cepNum);
    ApiLog->logoutf3("cepLifter=%d\n", p_info.cepLifter);
    ApiLog->logoutf3("winSize=%d\n", p_info.winSize);

    ApiLog->logoutf3("lowPass=%.0f\n", p_info.lowPass);
    ApiLog->logoutf3("highPass=%.0f\n", p_info.highPass);
    if (p_info.normEnergy) {
      ApiLog->logoutf3("normEnergy=true\n");
      ApiLog->logoutf3("energyScale=%.0f\n", p_info.energyScale);
      ApiLog->logoutf3("silFloor=%.0f\n", p_info.silFloor);
    } else
      ApiLog->logoutf3("normEnergy=false\n");

    break;
  case FYT_MFCCPLP:
    m_plp = new PLP;
    if (!m_plp) {
      ApiLog->logoutf2("Error new MFCCPLP\n");
      exit(-1);
    }
    m_plp->SetBaseInfo(p_info);
    m_plp->Initialize(BaseDim);

    if (p_info.zeroGlobalMean)
      ApiLog->logoutf3("zeroGlobalMean=true\n");
    else
      ApiLog->logoutf3("zeroGlobalMean=false\n");
    ApiLog->logoutf3("chlNum=%d\n", p_info.chlNum);
    ApiLog->logoutf3("cepNum=%d\n", p_info.cepNum);
    ApiLog->logoutf3("cepLifter=%d\n", p_info.cepLifter);
    ApiLog->logoutf3("winSize=%d\n", p_info.winSize);

    ApiLog->logoutf3("lowPass=%.0f\n", p_info.lowPass);
    ApiLog->logoutf3("highPass=%.0f\n", p_info.highPass);

    ApiLog->logoutf3("lpcOrder=%d\n", p_info.lpcOrder);
    ApiLog->logoutf3("compressFact=%.3f\n", p_info.compressFact);

    break;
  case FYT_MFCCPLP_NCC:
    m_plp_ncc = new PLP_NCC;
    if (!m_plp_ncc) {
      printf("Error new MFCCPLP_NCC\n");
      exit(-1);
    }
    m_plp_ncc->SetBaseInfo(p_info);
    if (HLDA_MatrixDir != NULL) {
      m_plp_ncc->SetHLDAMatrixDir(HLDA_MatrixDir);
    }
    m_plp_ncc->Initialize(BaseDim);
    break;
  case FYT_RASTAPLP:
    m_rasta = new RASTA_PLP;
    if (!m_rasta) {
      ApiLog->logoutf2("Error new RASTA_PLP\n");
      exit(-1);
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

    ApiLog->logoutf3("lpcOrder=%d\n", p_info.lpcOrder);

    ApiLog->logoutf3("stepsize=%d\n", p_info.stepsize);

    ApiLog->logoutf3("fpole=%.3f\n", p_info.fpole);
    ApiLog->logoutf3("fcepLifter=%.3f\n", p_info.fcepLifter);

    break;
  }
//   if (cms_strategy == 0) {
//     FrameLag = bHLDA ? 6 : 4;
//   } else {
//     FrameLag = bHLDA ? (cms_delayNum + 6) : (cms_delayNum + 4);
//     cms_frameNum = 0;
//   }
  
  //changed by songmeixu and hongmi for pitch_ncc
  if (cms_strategy == 0)
  {
	  if (!strcmp(fea_info.targetKind,"MFCCPLP_NCC_C0")&&fea_info.bPitch)
	  {
		  FrameLag = (iDeltaNum-1)*4;
	  }
	  else
		  FrameLag = (iDeltaNum-1)*2;
  } 
  else 
  {
	  if (!strcmp(fea_info.targetKind,"MFCCPLP_NCC_C0")&&fea_info.bPitch)
	  {
		  FrameLag = (iDeltaNum-1)*4+cms_delayNum;
	  }
	  else
		  FrameLag = (iDeltaNum-1)*2+cms_delayNum;
	  cms_frameNum = 0;
  }//

  //int tgtDim = (bHLDA?4:3)*BaseDim;
  int tgtDim = iDeltaNum * BaseDim;
  int tgtDim4 = (tgtDim + 3) & (~3);

  Buffer = new float[(FrameLag + 2)*tgtDim4];
  mBuffer = new MultiBuffer<float>;
  FrameCnt = 0;
  BufferWav = new short[winSize*5]; //[2014-07-01]为了防止后面不够10帧的数据能够缓存下来供下次使用，因此此处的内存大小需要不少于10帧的大小
  SampleCnt = 0;
  mBufferWav = new MultiBuffer<short>;
  ASSERT3(m_targetKind & KINDMASK, "Unsupported target kind: %s", p_info.targetKind);

  ApiLog->logoutf3("//*****************************************//\n");
}

void CreateFeature::AddWaveData(short *waveData, int smpNum) {
  switch (m_targetKind) {
  case FYT_MFCC:
    m_mfcc->AddWaveData(waveData, smpNum);
    break;
  case FYT_MFCCPLP:
    m_plp->AddWaveData(waveData, smpNum);
    break;
  case FYT_MFCCPLP_NCC:
    m_plp_ncc->AddWaveData(waveData, smpNum);
    break;
  case FYT_RASTAPLP:
    m_rasta->AddWaveData(waveData, smpNum);
    break;
  }
}

void CreateFeature::WriteFile(char *outFileName) {
  switch (m_targetKind) {
  case FYT_MFCC:
    m_mfcc->WriteFile(outFileName);
    break;
  case FYT_MFCCPLP:
    m_plp->WriteFile(outFileName);
    break;
  case FYT_MFCCPLP_NCC:
    m_plp_ncc->WriteFile(outFileName);
    break;
  case FYT_RASTAPLP:
    m_rasta->WriteFile(outFileName);
    break;
  }
}

void CreateFeature::WriteBuffer(char *buffer, int *length) {
  switch (m_targetKind) {

  case FYT_MFCCPLP:
    //m_plp->WriteBuffer(buffer,length);
    break;
  case FYT_MFCCPLP_NCC:
    m_plp_ncc->WriteBuffer(buffer, length);
    break;

  }

}


void CreateFeature::WriteBaseFeatFile(char *outFileName) 
{
  m_plp_ncc->WriteBaseFeatFile(outFileName);
  return;
}
void CreateFeature::InitFeatureCfg(char *systemDir, char *cfgFileName, bool bOnlineMode) 
{
  SampleRate = LVCSR_SampleRate();
  /////////////////////////////////////
  int nPathLen = strlen(systemDir);
  FILE *fCFG;
  fCFG = fopen(cfgFileName, "rt");
  if (!fCFG) {
    ApiLog->logoutf("Error open %s for read!\n", cfgFileName);
    exit(0);
  }
  /*static*/
  stringbuf1 config = "";
  envs = NULL;   // a bug when F5 run by jgao on 64bit PC, 080314 syq
  while (!feof(fCFG)) {
    if (fscanf(fCFG, "\n %[^\n]s", config) < 1) break;
    SetEnv(envs, config);
  }
  fclose(fCFG);

  ReadConfig("FYT_TargetKind", fea_info.targetKind);
  //ReadConfig("FYT_SourceRate",fea_info.smpPeriod);
  fea_info.smpPeriod = 10000000 / SampleRate;   // syq 070807 add for different SampleRate
  ReadConfig("FYT_TargetRate", fea_info.framePeriod);
  ReadConfig("FYT_ZeroGlobalMean", fea_info.zeroGlobalMean); // remove time-domain mean
  ReadConfig("FYT_WindowSize", fea_info.winSize);

  frameRate = fea_info.framePeriod / fea_info.smpPeriod; //sample number of each frame

  winSize = fea_info.winSize / fea_info.smpPeriod;
  ReadConfig("FYT_ChannelNum", fea_info.chlNum);
  ReadConfig("FYT_CepstrumLifter", fea_info.cepLifter);
  ReadConfig("FYT_CepstrumNum", fea_info.cepNum);
  ReadConfig("FYT_LowFreq", fea_info.lowPass); // used by fft
  ReadConfig("FYT_HighFreq", fea_info.highPass);
  ReadConfig("FYT_NormEnergy", fea_info.normEnergy);
  ReadConfig("FYT_EnergyScale", fea_info.energyScale);
  ReadConfig("FYT_SilFloor", fea_info.silFloor);

  ReadConfig("FYT_LPCOrder", fea_info.lpcOrder);
  ReadConfig("FYT_CompressFact", fea_info.compressFact);
  ReadConfig("FYT_DoRASTA", fea_info.bDoRasta);
  ReadConfig("FYT_DoCep", fea_info.bDoCep);
  ReadConfig("FYT_DoGain", fea_info.bgainflag);
  ReadConfig("FYT_StepSize", fea_info.stepsize);
  ReadConfig("FYT_FPole", fea_info.fpole);
  ReadConfig("FYT_fCepLifter", fea_info.fcepLifter);

  ReadConfig("FYT_Pitch", fea_info.bPitch);
  ReadConfig("FYT_bHLDA", bHLDA);


  //add by hongmi
  ReadConfig("FYT_DeltaNum", iDeltaNum);
  fea_info.bHLDA = bHLDA;
  HLDA_MatrixDir = NULL;
  //
  Matrix = NULL;
  m_mfcc = NULL;
  m_plp = NULL;
  m_rasta = NULL;
  m_plp_ncc = NULL;
  if (bHLDA) {
    HLDA_MatrixDir = new  char[256 * sizeof(char)];
    strcpy(HLDA_MatrixDir, systemDir);
    ReadConfig("FYT_HLDA_Matrix", HLDA_MatrixDir + nPathLen);
    FILE *matrixfile = fopen(HLDA_MatrixDir, "rb");
    if (!matrixfile) {
      ApiLog->logoutf("error opening %s file! nPathLen=%d, systemDir=%s\n", HLDA_MatrixDir, nPathLen, systemDir);
      exit(1);
    }
    fread(&matrix_h, sizeof(int), 1, matrixfile);
    fread(&matrix_n, sizeof(int), 1, matrixfile);
    Matrix = new double[matrix_h * matrix_n];
    fread(Matrix, sizeof(double), matrix_h * matrix_n, matrixfile);
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

  


  cms_strategy = 0;
  ReadConfig("FYT_CMS_Strategy", cms_strategy);
  cms_delayNum = 51;
  ReadConfig("FYT_CMS_Delay_Num", cms_delayNum);

  /*changed by songmeixu for pitch_ncc*/
  if (!strcmp(fea_info.targetKind,"MFCCPLP_NCC_C0")&&fea_info.bPitch)
  {
	  cms_cacheLen = (cms_delayNum + 16) * frameRate * sizeof(short);
  }
  else
	  cms_cacheLen = (cms_delayNum + 10) * frameRate * sizeof(short);
  /**/


  rmZeroValue_strategy = 0;
  ReadConfig("FYT_RmZeroValue_Strategy", rmZeroValue_strategy);


  if (bOnlineMode && bonlinecms)
  {
    ApiLog->logoutf3("use online cms!\n");
    char CMS_bin[256];
    //strcpy(CMS_bin, systemDir);
    //ReadConfig("FYT_CMS_bin", CMS_bin+ nPathLen);
    if (cms_strategy != 1) 
	{
      strcpy(CMS_bin, systemDir);
      ReadConfig("FYT_CMS_bin", CMS_bin + nPathLen);
    }
	else
      CMS_bin[0] = '\0';
    cmsobj = new OnLineCms(-1, CMS_bin);

    cmsobj->ResetInitCmsValue(cms_delayNum);
    bonlinecms = true;// lita [4/9/2010]
  } 
  else 
	  ApiLog->logoutf3("use offline cms!\n"), cmsobj = NULL, bonlinecms = false;

  SetBaseInfo(fea_info);


  src = NULL;
  tgt = NULL;
  if (bHLDA) {
    hldatgt = NULL;
  }
  ApiLog->logoutf3("Begin to init feat and Gmm for Gender Classification:\n");
  //  m_mfcc=NULL; m_plp=NULL; m_rasta=NULL;
  mfcc = NULL;
  mfcc = new MFCC2;   // [6/26/2007 copyed from Segment by syq]
  if (!mfcc)  {
    ApiLog->logoutf("Error new MFCC!\n");
    exit(-3);
  }
  FEATURE_BASEINFO gmm_featInfo;
  memcpy(&gmm_featInfo, &fea_info, sizeof(FEATURE_BASEINFO));
  gmm_featInfo.cepNum = 16;
  sprintf(gmm_featInfo.targetKind, "MFCC");
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
  if (!strcmp(GmmModel, systemDir)) GmmModel[0] = '\0'; // 080218 jgao found bug using pitch am
  //  strcpy(GmmModel, cfgFileName);
  //  char *pt = NULL;
  //  char *pt1=strrchr(GmmModel,'/');
  //  if (pt1) pt = pt1;
  //  char *pt2=strrchr(GmmModel,'\\');
  //  if (pt2 && pt2 > pt1) pt = pt2;
  //  if (!pt) pt = GmmModel;
  //  ReadConfig("GmmModel", pt);
  ReadConfig("VoiceCoef", VoiceCoef);
  if (GmmModel[0]) {
    //ApiLog->logoutf3("\t2,Begin to load GmmModel %s;\n", GmmModel);
    gmm = new iGmmParam(GmmModel);
    if (!gmm) {
      ApiLog->logoutf("Error new iGmmParam!\n");
      exit(-1);
    } else ApiLog->logoutf3("GmmModel \"%s\": has %d classes, %d mixture, vector size = %d\n", GmmModel, gmm->header->gmmNum, gmm->header->mixNum, gmm->header->vecSize);
    m_nGmmNum = gmm->header->gmmNum;
    switch (gmm->header->vecSize / mfcc->GetBaseDim()) {
    case 1:
      ApiLog->logoutf3("GmmModel vector is : MFCC \n");
      break;
    case 2:
      ApiLog->logoutf3("GmmModel vector is : MFCC + delt\n");
      break;
    case 3:
      ApiLog->logoutf3("GmmModel vector is : MFCC + delt + deltdelt\n");
      break;
    default:
      ApiLog->logoutf2("Error : GMMModel_vecSize=%d  cepNum=%d\n", gmm->header->vecSize, gmm_featInfo.cepNum);
      gmm = NULL; // exit(-2);   080303 syq add
    }
    //ApiLog->logoutf3("\t3,Begin to new Feature2;\n");
    feat = new Feature2(gmm->header->vecSize, false, false);
    if (!feat)  {
      ApiLog->logoutf2("Error new Feature!\n");
      exit(-1);
    }
  } else gmm = NULL, feat = NULL;
  ApiLog->logoutf3("Finish init feat and Gmm for Gender Classification!\n");

  pitchesOri = new float[10];
  nccsOri = new float[10];

}
void CreateFeature::CleanFeature() {

  CleanInfo();
  if (Matrix) {
    delete [] Matrix;
  }
  if (tgt) {
    //    delete [] tgt;
  }
  if (hldatgt && bHLDA) {
    //    delete [] hldatgt;   // [6/27/2007 added by syq] make mem to be controled by user (so could be used when decoding so long)
  }
  //if (envs) delete envs;
  for (aConfigEnv *ev = envs; ev;) {
    aConfigEnv *temp = ev;
    ev = ev->next;
    delete temp;
  }

  // lita [4/9/2010]
  //if (bonlinecms) delete [] cmslen, cmslen = NULL;// llu [3/30/2010]
  if (cmsobj) delete(cmsobj), cmsobj = NULL;
  if (Buffer) delete []Buffer, Buffer = NULL;
  if (pitchesOri) delete []pitchesOri, pitchesOri = NULL;
  if (nccsOri) delete []nccsOri, nccsOri = NULL;
  if (mBuffer) delete(mBuffer), mBuffer = NULL;
  if (BufferWav) delete []BufferWav, BufferWav = NULL;
  if (mBufferWav) delete(mBufferWav), mBufferWav = NULL;
  if (HLDA_MatrixDir) delete HLDA_MatrixDir, HLDA_MatrixDir = NULL;
}

int creNum = 0;
int cmsNum=0;
int smptotal=0;
float *CreateFeature::GetFeature(short *waveData, int smpNum, FeatureHeader *header, bool bBaseOnly, TSR_STREAM_FLAG flag, int *residue) 
{

  // 070620 syq add
  header->framePeriod = fea_info.framePeriod;
  header->mfccKind = m_targetKind;
  int srcDim;
  if ((waveData && smpNum > 0) || (SampleCnt >winSize&&smpNum==0)) //如果flag=end且缓存区中只剩下不到winSize个点，那就不够后面的特征提去，因此直接扔掉
  {
    // deal with negative input
    if (!bonlinecms) 
	{
		FrameBack = 0;
	    SampleCnt=0;
	}
    else if (bonlinecms && (flag == TSR_STREAM_START || flag == TSR_STREAM_ALL)) 
	{
      cmsobj->ResetInitCmsValue(cms_delayNum), FrameCnt = 0, SampleCnt = 0, FrameBack = 0;//may be a bug?FrameBack shou no be 0?
      cms_frameNum = 0;
    }
	//mBufferWav用于缓存送入需要提取特征的数据，并且能够保持一句话的多包之间的连续性。每句话的起始包不应该保存上一句话的数据，所以每句话的起始包SampleCnt应该为0
    mBufferWav->Initialize(1, BufferWav, SampleCnt, waveData, smpNum);
    waveData = NULL;
    bool bforce = true;
    smpNum = mBufferWav->framenum();//获取拼接以后的数据点数
	
 	if (smpNum < winSize )//AddWaveData处FrameNum=(smpNum-winSize)/frameRate+1，因此凡是小于200个点的非end包，都会缓存下来。
 	{
 		if(flag == TSR_STREAM_ALL) //如果整包都小于200个点，那直接扔掉
 		{
 			header->frameNum= 0;
 			return NULL;
 		}
 		   header->frameNum = 0;
 		   mBufferWav->getBuffer(0, smpNum, bforce, BufferWav);//缓存这包数据以便等下一包来一起提特征。
 		SampleCnt = smpNum;
 		return NULL;
 	}
    mBufferWav->getBuffer(0, smpNum, bforce, waveData);//根据点数获取数据
    switch (m_targetKind) {
    case FYT_MFCC:
      m_mfcc->AddWaveData(waveData, smpNum, residue);
      src = m_mfcc->GetFeatureDate(&header->frameNum, &header->frameSize);
      break;
    case FYT_MFCCPLP:
      m_plp->AddWaveData(waveData, smpNum, residue);
      src = m_plp->GetFeatureDate(&header->frameNum, &header->frameSize);
      break;
    case FYT_RASTAPLP:
      m_rasta->AddWaveData(waveData, smpNum, residue);
      src = m_rasta->GetFeatureDate(&header->frameNum, &header->frameSize);
      break;
    case FYT_MFCCPLP_NCC:
      //m_plp_ncc->AddWaveData(waveData, smpNum);
	  m_plp_ncc->AddWaveData(waveData, smpNum, (flag == TSR_STREAM_START || flag == TSR_STREAM_ALL));
	  //m_plp_ncc->AddWaveData(waveData, smpNum, true);
      src = m_plp_ncc->GetFeatureDate(&header->frameNum, &header->frameSize);
      break;
    }

    /*output the basedim feature*/
    //add by hongmi
//      float *temp = src;
//      char orgfeatureName[256];
//      short tframeSize = header->frameSize;
//      sprintf(orgfeatureName, "%04d.base.plp", creNum++);
//      FILE *ffeat = fopen(orgfeatureName, "wb");
//      fwrite(&header->frameNum, sizeof(int), 1, ffeat);
//      fwrite(&header->framePeriod, sizeof(int), 1, ffeat);
//      fwrite(&tframeSize, sizeof(short), 1, ffeat);
//      fwrite(&header->mfccKind, sizeof(short), 1, ffeat);
//      fwrite(temp, sizeof(float), (header->frameNum)*BaseDim, ffeat);
//      fclose(ffeat);

    SampleCnt = CalResidue(smpNum);//计算此次数据包中最后不够一帧的数据会留下来给下一包用。
    mBufferWav->getBuffer(smpNum - SampleCnt, SampleCnt, bforce, BufferWav);
    delete []waveData;
    srcDim = header->frameSize / sizeof(float);
	int tmpSrcDim = srcDim;
	if(m_targetKind==FYT_MFCCPLP&& srcDim == 14)//plp +pitch
		tmpSrcDim = srcDim-1;
	//printf("tmpSrcDim is %d\n",tmpSrcDim);
	ApiLog->logoutf("tmpSrcDim is %d \n", tmpSrcDim);
    switch (m_targetKind)
	{
    // 091117 add
    case FYT_MFCCPLP:
    case FYT_MFCCPLP_NCC:
    case FYT_RASTAPLP:
      int ii, kk;
      float *ft;
      for (ii = 0, ft = src; ii < header->frameNum; ii++, ft += srcDim)
	  {
        for (kk = 0; kk < tmpSrcDim; kk++) 
		{
          //if (k == m_BaseDim-1) {
          // }else{
          ft[kk] *= 10.f; // 2004.11.04 plu : plp鍙傛暟鐨勯噺绾уお灏忥紝鍔犲�嶄箣
          if (ft[kk] > DYN_RANGE) ft[kk] = DYN_RANGE;
          if (ft[kk] < -DYN_RANGE) ft[kk] = -DYN_RANGE;
          //}
        }
      }
      break;
    }

    if (bBaseOnly) return src;
    if (flag == TSR_STREAM_ALL && (!src || header->frameNum <3) || flag == TSR_STREAM_START && (!src || header->frameNum <= 0)) 
	{
      header->frameNum = 0;
      return NULL;
    }
 	if (m_targetKind == FYT_MFCCPLP_NCC&&flag!=TSR_STREAM_END&& (!src || header->frameNum<10) )
 	{
 		if (flag == TSR_STREAM_ALL&& header->frameNum<10)
 		{
 			header->frameNum = 0;
 			return NULL;
 		}
 		else 
 		{
 		     mBufferWav->getBuffer(0, smpNum, bforce, BufferWav);//缓存这包数据以便等下一包来一起提特征。
 		     SampleCnt = smpNum;//记录当前缓存的不够做平滑的数据的点数
 			 header->frameNum=0;
 		     return NULL;
 		}
 	}
  } 
   else if (smpNum==0&&!bonlinecms)
   {
 	  header->frameNum = 0;
 	  srcDim = BaseDim;
 	  return NULL;
 
   }
  else
  {
	  header->frameNum = 0;
	  srcDim = BaseDim;
	  //return NULL;  
  }


  /*output the basedim feature*/
  //add by hongmi
//    float *tempbase = src;
//    char orgfeatureNamebase[256];
//    short tframeSizebase = header->frameSize;
//    sprintf(orgfeatureNamebase, "%04d.base.plp", creNum++);
//    FILE *ffeatbase = fopen(orgfeatureNamebase, "wb");
//    fwrite(&header->frameNum, sizeof(int), 1, ffeatbase);
//    fwrite(&header->framePeriod, sizeof(int), 1, ffeatbase);
//    fwrite(&tframeSizebase, sizeof(short), 1, ffeatbase);
//    fwrite(&header->mfccKind, sizeof(short), 1, ffeatbase);
//    fwrite(tempbase, sizeof(float), (header->frameNum)*BaseDim, ffeatbase);
//    fclose(ffeatbase);


  int tgtDim;
  ApiLog->logoutf("srcDim is %d \n", srcDim);
//  printf("srcDim is %d\n", srcDim);
  tgtDim = iDeltaNum * srcDim; //changed by hongmi
//  printf("tgtDim is %d\n", tgtDim);
  ApiLog->logoutf("tgtDim is %d \n", tgtDim);
  header->frameSize = iDeltaNum * srcDim * sizeof(float);//changed by hongmi
  int tgtDim4 = (tgtDim + 3) & (~3);

  // deal with last buffer
  int FrameLast = FrameBack; //min(FrameCnt,FrameLag+2);//The frames stored by last process for current process.
  //  if(cms_strategy == 0)
  //    int FrameToPad=FrameBack>2?min(6,FrameLast-2):0;//The frames needed moving from preious buffer to current buffer for computing delta part.
  //  else

  /*changed by songmeixu and hongmi for pitch_ncc*/
  int FrameToPad;
  if (m_targetKind == FYT_MFCCPLP_NCC&& srcDim == 15 )
  {//pitch_ncc
	  FrameToPad = FrameBack > 2 ? min(cms_delayNum + (tgtDim / srcDim - 1)*4, FrameLast - 2) : 0;
  }
  else
  {//plp
	  FrameToPad = FrameBack > 2 ? min(cms_delayNum + 6, FrameLast - 2) : 0;
  }
  
  //

  tgt = (float *)Malloc32((header->frameNum + FrameToPad) * tgtDim4 * sizeof(float), false); // new float [header->frameNum*tmpDim]; syq 070706
  if (FrameToPad > 0) memcpy(tgt, Buffer + (FrameLast - FrameToPad)*tgtDim4, tgtDim4 * FrameToPad * sizeof(float));

  for (int i = 0; i < header->frameNum; i++) 
  {
    memcpy(tgt + tgtDim4 * (i + FrameToPad), src + i * srcDim, srcDim * sizeof(float));
  }
  header->frameNum += FrameToPad;
  FrameLast -= FrameToPad;

  const int DELWIN = 2, ACCWIN = 2;
  const float SIGMAD2 = 10.0, SIGMAA2 = 10.0;

  int delta;
  //MultiBuffer<float> *mBuffer = new MultiBuffer<float>(tgtDim4,Buffer,FrameLast,tgt,header->frameNum);
  mBuffer->Initialize(tgtDim4, Buffer, FrameLast, tgt, header->frameNum);

  /* smoooth pitch and ncc here, hypothesise each last two dim of srcDim in mBuffer is pitch and ncc that we need to process,
  given the mean of cms_delayNum and FrameLast */
  /*[2013/12/13]add by songmeixu 用于pitch和ncc的平滑*/
  if (m_targetKind == FYT_MFCCPLP_NCC && srcDim == 15 && flag != TSR_STREAM_END) 
  {
    int smoothNum = 0;
    int st = 0;
    float *pitches = NULL;
    float *nccs = NULL;

    if (flag == TSR_STREAM_START || flag == TSR_STREAM_ALL) 
	{
      assert(FrameLast == 0);
      smoothNum = header->frameNum;
      st = 0;

      pitches = (float *)Malloc32(sizeof(float) * (smoothNum));
      nccs = (float *)Malloc32(sizeof(float) * (smoothNum));
      for (int i = 0; i < mBuffer->framenum(); ++i) 
	  {
        float *cfeat = mBuffer->get(i);
        pitches[i] = cfeat[13];
        nccs[i] = cfeat[14];
      }
      assert(mBuffer->framenum() - 10 >= 0);
      for (int i = mBuffer->framenum() - 10, j = 0; i < mBuffer->framenum() && j < 10; ++i, ++j) {
        float *cfeat = mBuffer->get(i);
        pitchesOri[j] = cfeat[13];
        nccsOri[j] = cfeat[14];
      }

      for (int i = 2; i < mBuffer->framenum() - 2; i++) {
        pitches[i] = pitches[i + 2];
        nccs[i] = nccs[i + 2];
      }
      for (int i = 0; i < 3 && ((i+3) < header->frameNum + FrameLast); i++) {
        pitches[i] = pitches[i + 3];
        nccs[i] = nccs[i + 3];
      }
    } 
	else
	{
      int cur = mBuffer->framenum() - header->frameNum + FrameToPad;
      smoothNum = 10 + header->frameNum - FrameToPad;
      st = cur - 6;

      pitches = (float *)Malloc32(sizeof(float) * (smoothNum));
      nccs = (float *)Malloc32(sizeof(float) * (smoothNum));
      for (int i = 0; i < 10; ++i) 
	  {
        pitches[i] = pitchesOri[i];
        nccs[i] = nccsOri[i];
      }
      for (int i = cur, j = 10; i < mBuffer->framenum() && j < smoothNum; ++i, ++j)
	  {
        float *cfeat = mBuffer->get(i);
        pitches[j] = cfeat[13];
        nccs[j] = cfeat[14];
      }
      assert(mBuffer->framenum() - 10 >= 0);
      for (int i = mBuffer->framenum() - 10, j = 0; i < mBuffer->framenum() && j < 10; ++i, ++j) {
        float *cfeat = mBuffer->get(i);
        pitchesOri[j] = cfeat[13];
        nccsOri[j] = cfeat[14];
      }

      for (int i = 0; i < smoothNum - 2; ++i) {
        pitches[i] = pitches[i + 2];
        nccs[i] = nccs[i + 2];
      }
    }

    LinearSmoothPitchArray(pitches, smoothNum);
    MedianSmoothArray(pitches, smoothNum);
    LinearSmoothNCCArray(nccs, smoothNum);
    MedianSmoothArray(nccs, smoothNum);

    // assign the adjusted pitch & ncc
    for (int i = st, j = smoothNum - (mBuffer->framenum() - st); i < mBuffer->framenum() && j < smoothNum; ++i, ++j) {
      float *cfeat = mBuffer->get(i);
      cfeat[13] = pitches[j];
      cfeat[14] = nccs[j];
    }
    Free32(pitches);
    Free32(nccs);
  }//end songmeixu



  /**************************做delta***************************/
  for (delta = 0; delta < tgtDim / srcDim - 1; delta++)
  {
    for (int i = FrameLast; i < mBuffer->framenum(); i++) 
	{
      for (int j = 0; j < srcDim; j++) 
	  {
        double sum = 0;
        for (int k = 1; k <= DELWIN; k++)
		{
          //float *ffeat=tgt+min(i+k,header->frameNum-1)*tgtDim4+delta*srcDim;
          //float *ffeat=(float*)(mBuffer+(i+k))+delta*srcDim;
          //float *bfeat=(float*)(mBuffer+(i-k))+delta*srcDim;
          float *ffeat = mBuffer->get(i + k) + delta * srcDim;
          float *bfeat = mBuffer->get(i - k) + delta * srcDim;
          //if (flag==TSR_STREAM_CONT && i-k<0)
          //  bfeat=tgt+min(i-k,0)*tgtDim4+delta*srcDim;
          //else bfeat=Buffer+max(FrameLast+i-k,0)*tgtDim4+delta*srcDim;
          sum += k * (ffeat[j] - bfeat[j]);
        }
        //tgt[i*tgtDim4+delta*srcDim+srcDim+j]=float(sum*(1.0/SIGMAD2));
        //mBuffer[i,(delta+1)*srcDim+j]=float(sum*(1.0/SIGMAD2));
        //*((float*)(mBuffer+i)+(delta+1)*srcDim+j)=float(sum*(1.0/SIGMAD2));
        mBuffer->get(i, (delta + 1)*srcDim + j) = float(sum * (1.0 / SIGMAD2));
      }
    }
  }
  /*output the tgtDim feature*/
  //add by hongmi
//   float *temp = tgt;
//   char orgfeatureName[256];
//   short tframeSize = 60*4;
//   assert(tgtDim4 == 60);
//   sprintf(orgfeatureName, "%04d_nocms.plp", creNum);
//   FILE *ffeat = fopen(orgfeatureName, "wb");
//   if (header->frameNum - FrameLag  >= 0) {
// 	  if (!(flag == TSR_STREAM_END || flag == TSR_STREAM_ALL)) {
// 		  int num = header->frameNum - FrameLag;
// 		  fwrite(&num, sizeof(int), 1, ffeat);
// 		  fwrite(&header->framePeriod, sizeof(int), 1, ffeat);
// 		  fwrite(&tframeSize, sizeof(short), 1, ffeat);
// 		  fwrite(&header->mfccKind, sizeof(short), 1, ffeat);
// 		  fwrite(temp, sizeof(float), (header->frameNum - FrameLag)*tgtDim4, ffeat);
// 		  AddFrameNum(header->frameNum - FrameLag);
// 	  } else {
// 		  fwrite(&header->frameNum, sizeof(int), 1, ffeat);
// 		  fwrite(&header->framePeriod, sizeof(int), 1, ffeat);
// 		  fwrite(&tframeSize, sizeof(short), 1, ffeat);
// 		  fwrite(&header->mfccKind, sizeof(short), 1, ffeat);
// 		  fwrite(temp, sizeof(float), (header->frameNum)*tgtDim4, ffeat);
// 		  printf("output all frame num = %d\n", AddFrameNum(header->frameNum));
// 	  }
//   }
//   fclose(ffeat);
  /*float* temp =tgt;
  char orgfeatureName[256];
  short tframeSize = 60*4;
  sprintf(orgfeatureName,"%04d.tgt.plp",creNum++);
  FILE* ffeat = fopen(orgfeatureName, "wb");
  fwrite(&header->frameNum,sizeof(int),1,ffeat);
  fwrite(&header->framePeriod,sizeof(int),1,ffeat);
  fwrite(&tframeSize,sizeof(short),1,ffeat);
  fwrite(&header->mfccKind,sizeof(short),1,ffeat);
  fwrite(temp,sizeof(float),(header->frameNum)*tgtDim4,ffeat);
  fclose(ffeat);*/

  if (!bonlinecms) 
  {
    for (int j = 0; j < tgtDim; j++) {
      double res;
      double Meanacc = 0.0;
      double Varacc  = 0.0;

      int frm = 0;
      for (frm = 0; frm < header->frameNum; frm++ ) {
        res = tgt[frm * tgtDim4 + j];
        Meanacc += res;
        Varacc += res * res;
      }

      double mean, var;
      mean = Meanacc / (double)header->frameNum;
      if (header->frameNum > 1)
        var = sqrt((Varacc - mean * Meanacc) / (double)(header->frameNum - 1));
      else
        var = double(1.0);
      if (var == 0.0f) var = double(1.0);
      for ( frm = 0; frm < header->frameNum; frm++ ) {
        res = tgt[frm * tgtDim4 + j];
        tgt[frm * tgtDim4 + j] = float((res - mean) / var);
      }
    }
    //
  } 
  else 
  {
    // 
    int FrameTotal = FrameLast + header->frameNum;//total frames that will be calculated cms
    FrameBack  = min(FrameTotal, FrameLag + 2); //save some frames for next data, 2 frames overlap with current cms computing.
    bool bforce = true;
    mBuffer->getBuffer(mBuffer->framenum() - FrameBack, FrameBack, bforce, Buffer);

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
    if (cms_strategy == 0) 
	{
      ApiLog->logoutf("creature  %d \t %d\t\n", header->frameNum, FrameLag);
      if (!(flag == TSR_STREAM_END || flag == TSR_STREAM_ALL)) {
        header->frameNum -= FrameLag; //if not end, left Framelag without cms.
        ApiLog->logoutf("flag %d \n", header->frameNum);
      }
      FrameCnt += header->frameNum;
      if (header->frameNum < 0)
        return NULL;
      cmsobj->OnlineCmsCal(tgt, tgtDim, tgtDim4, header->frameNum);
    } 
	else 
	{
	  //changed by hongmi 
	  if (m_targetKind == FYT_MFCCPLP_NCC)//
	  {
		  if (!(flag == TSR_STREAM_END || flag == TSR_STREAM_ALL))
		  {
			  if (header->frameNum - FrameLag<0)//[2014/3/28 add by hm]
			  {
				  if (tgt) Free32(tgt), tgt = NULL; 
				  header->frameNum=0;
				  
				  return NULL;
			  }
			  cmsobj->OnlineCmsDelay(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum - FrameLag, &cms_frameNum);
			  header->frameNum -= FrameLag;
		  } 
		  else
		  {
			  cmsobj->OnlineCmsDelay(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum, &cms_frameNum);
		  }
	  }
	  else//云端情况,防止最后最后一包多送:header->frameNum - FrameLag>=0
	  {
		  if (!(flag == TSR_STREAM_END || flag == TSR_STREAM_ALL))
		  {
			  if (header->frameNum - FrameLag<0)//	云端开vad会出现的情况，需要屏蔽否则数据多送 [4/28/2015 hongmi]
			  {
				  if (tgt) Free32(tgt), tgt = NULL; 
				  header->frameNum=0;
				  return NULL;
			  }
			  cmsobj->OnlineCmsDelay(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum - FrameLag, &cms_frameNum);
			  header->frameNum -= FrameLag;
		  } 
		  else 
		  {
			  cmsobj->OnlineCmsDelay(tgt, tgtDim, tgtDim4, header->frameNum, header->frameNum, &cms_frameNum);
		  }
	  }

    }
  }
  if (bHLDA) 
  {
    hldatgt = NULL;
    int htgtDim4 = (srcDim * 3 + 3) & (~3);
	ApiLog->logoutf("bHLDA , frame size = %d\n",htgtDim4);
    if (header->frameNum > 0) 
	{
		header->frameSize =htgtDim4*sizeof(float);
		ApiLog->logoutf("bHLDA , header frameSize = %d\n",header->frameSize);
      hldatgt = (float *)Malloc32(header->frameNum * htgtDim4, int(sizeof(float) ), true);  // hldatgt = new float [3*header->frameNum*srcDim]; syq 070706
      int i;
      for (i = 0; i < header->frameNum; i++) {
        int k;
        for (k = 0; k < matrix_n; k++) {
          hldatgt[htgtDim4 * i + k] = 0;
          int j;
          for (j = 0; j < matrix_h; j++) {
            hldatgt[htgtDim4 * i + k] += tgt[i * tgtDim4 + j] * Matrix[k * matrix_h + j]; //mBuffer->get(i,j)*Matrix[k*matrix_h+j];
          }
        }
      }
    }
    if (tgt) Free32(tgt), tgt = NULL;      // [6/29/2007 added by syq] 
	if (hldatgt == NULL) {
		ApiLog->logoutf("creature error \n");
	}
    /*output the htgtDim4 feature*/
    //add by hongmi
// 	float* temp = hldatgt;
// 	char orgfeatureName[256];
// 	short tframeSize = 44*4;
// 	sprintf(orgfeatureName,"%04d",creNum++);
// 	FILE* ffeat = fopen(orgfeatureName, "wb");
// 	fwrite(&header->frameNum,sizeof(int),1,ffeat);
// 	fwrite(&header->framePeriod,sizeof(int),1,ffeat);
// 	fwrite(&tframeSize,sizeof(short),1,ffeat);
// 	fwrite(&header->mfccKind,sizeof(short),1,ffeat);
// 	fwrite(temp,sizeof(float),header->frameNum* htgtDim4,ffeat);
// 	fclose(ffeat);
    return hldatgt;
  } else {
//  	float *tmp = tgt;
//  	char orgfeatureName[256];
//  	short tframeSize = 60*4;
//  	assert(tgtDim4 == 60);
//  	sprintf(orgfeatureName, "%04d_cms.plp", creNum);
//  	FILE *ffeat = fopen(orgfeatureName, "wb");
//  	if (header->frameNum >= 0) {
//  		if (!(flag == TSR_STREAM_END || flag == TSR_STREAM_ALL)) {
//  			int num = header->frameNum;
//  			fwrite(&num, sizeof(int), 1, ffeat);
//  			fwrite(&header->framePeriod, sizeof(int), 1, ffeat);
//  			fwrite(&tframeSize, sizeof(short), 1, ffeat);
//  			fwrite(&header->mfccKind, sizeof(short), 1, ffeat);
//  			fwrite(tmp, sizeof(float), (header->frameNum)*tgtDim4, ffeat);
//  			AddFrameNum(header->frameNum);
//  		} else {
//  			fwrite(&header->frameNum, sizeof(int), 1, ffeat);
//  			fwrite(&header->framePeriod, sizeof(int), 1, ffeat);
//  			fwrite(&tframeSize, sizeof(short), 1, ffeat);
//  			fwrite(&header->mfccKind, sizeof(short), 1, ffeat);
//  			fwrite(tmp, sizeof(float), (header->frameNum)*tgtDim4, ffeat);
//  			printf("output all frame num = %d\n", AddFrameNum(header->frameNum));
//  		}
//  	}
//  	fclose(ffeat);
// 	++creNum;
    float *temp = Buffer;
    return tgt;
  }
}

// 070620 syq move from main
void CreateFeature::WriteFile(char *outFileName, float *tgtfeature, FeatureHeader header, bool bAlign4, bool bAppend) {
  if (tgtfeature == NULL) return;
  int nAlign4 = 0;
  //if (bAlign4)
  //  nAlign4 = 4 - header.frameSize / sizeof(float) % 4;
  int ElementsInSrcFeatureTemp = (ElementsInSrcFeature + 3) & (~3);
  FILE *fout;
  int i, k;
  float *ft;
  char outFileFullName[256] = "";
  strcpy(outFileFullName, outFileName);
  switch (m_targetKind) {
  case FYT_MFCC:
    strcat(outFileFullName, ".mfc");
    break;
  case FYT_MFCCPLP:
    strcat(outFileFullName, ".plp");
    break;
  case FYT_RASTAPLP:
    strcat(outFileFullName, ".rplp");
    break;
  case  FYT_MFCCPLP_NCC:
    strcat(outFileFullName, ".plpncc");
    break;
  }
  strcpy(outFileName, outFileFullName);
  if (!bAppend || access(outFileFullName, 0) != 0)
  {
    fout = fopen(outFileFullName, "wb");
    if (fout) fwrite(&header.frameNum, sizeof(int), 1, fout);
	else printf("error open plpfile %s for write\n", outFileFullName);
  } else {
    fout = fopen(outFileFullName, "r+b");
    FeatureHeader oriheader;
    fread(&oriheader, sizeof(FeatureHeader), 1, fout);
    if (oriheader.frameNum < 0 || oriheader.frameSize != header.frameSize) {
      printf("error in WriteFile %s\n", outFileFullName);
      fclose(fout);
      exit(3);
    }
    if (fout) fseek(fout, 0, SEEK_SET);
    oriheader.frameNum += header.frameNum;
    fwrite(&oriheader.frameNum, sizeof(int), 1, fout);
  }

  //fwrite(&header.frameNum,sizeof(int),1,fout);
  fwrite(&header.framePeriod, sizeof(int), 1, fout);
  fwrite(&header.frameSize, sizeof(short), 1, fout);
  fwrite(&header.mfccKind, sizeof(short), 1, fout);
  fflush(fout);
  fseek(fout, 0, SEEK_END);
  //for (i = 0, ft = tgtfeature; i < header.frameNum; i++, ft += header.frameSize / sizeof(float) + nAlign4)
  for (i = 0, ft = tgtfeature; i < header.frameNum; i++, ft += ElementsInSrcFeatureTemp ) {
    for (k = 0; k < ElementsInSrcFeatureTemp; k++) {
      if (ft[k] > 50) ft[k] = 50;
      if (ft[k] < -50) ft[k] = -50;
    }
    fwrite(ft, sizeof(float), ElementsInSrcFeatureTemp, fout);
  }
  fclose(fout);
}


bool CreateFeature::FindSegType(short *waveData, int smpNum, SOUND_TYPE *myType) { //fpan change 2005-06-11
  if (!gmm) {
    *myType = MALE;
    return true;
  }
  float fRst, fMax;
  int nMaxGmmNo;
  int nFmNum;
  if (waveData && smpNum > 0 && myType) {
    if (smpNum < SampleRate * 0.1) { 
      *myType = NOISE;
      return true;
    }
    mfcc->AddWaveData(waveData, smpNum);

    ///////////////////fpan add 2005-06-11
    int mfccFrameNum = mfcc->GetFrameNumber();
    if (mfccFrameNum <= 3) {
      *myType = MALE;
      return false;
    }
    ///////////////////////////////////////////////

    feat->CalNewFeature(mfcc->GetFrameNumber(), mfcc->GetBaseDim(), mfcc->GetMfccData());
    // free(mfcc->GetMfccData());   // 070907 syq leakage bug part 1 !
    nFmNum = feat->GetFrameNumber();
    gmm->AdjustVecSize(nFmNum);


	for (int nGmm = 0; nGmm < m_nGmmNum; nGmm++) {
      fRst = gmm->CalculateProb(nGmm, feat->GetObv(0), nFmNum);
      if (nGmm >= 2)
        fRst *= VoiceCoef;
      /*      if(nGmm>=2 && fRst>fMax){
      FILE * fp =fopen("noise.txt","a");
      fprintf(fp, "%f,%f:%f\n", fMax, fRst, fMax/fRst);
      fclose(fp);
      }*/
      /*      if (nGmm==0)
      {
      fMax=fRst;  nMaxGmmNo=0;
      }
        else*/
      if (nGmm == 0 || fMax < fRst && nGmm < 2) {
        fMax = fRst;
        nMaxGmmNo = nGmm;
      }
    }

    *myType = (SOUND_TYPE)nMaxGmmNo;
    return true;
  } else return false;
}

int CreateFeature::CalResidue(int smpNum, int *frameNum) {
  int FrameNum = (smpNum - winSize) / frameRate + 1; //
  if (frameNum) *frameNum = FrameNum;
  //int residue = smpNum-(FrameNum-1)*frameRate-winSize;
  int residue = smpNum - max(0, FrameNum) * frameRate;
  return residue;
}

int CreateFeature::CalRemainder(int &smpNum) {
  int residue = winSize - frameRate;
  if (smpNum >= residue) smpNum -= residue;
  else residue = smpNum, smpNum = 0;
  return residue;
}

void CreateFeature::LinearSmoothPitchArray(float *src, int num) {
  int    *bank;
  int   i;
  bank = (int *)calloc(num, sizeof(int));
  for (i = 0; i < num; i++)
    bank[i] = 100*src[i] + 0.5F;
  for (i = 2; i < num - 2; i++) {
    src[i] = (float)((int)((bank[i - 2] + bank[i - 1] * 2 + bank[i + 1] * 2 + bank[i + 2]) / 12.0F + bank[i] / 2.0F + 0.5F)) / 100;
  }
  free(bank);
}

void CreateFeature::LinearSmoothNCCArray(float *src, int num) {
  float    *bank;
  int   i;
  bank = (float *)calloc(num, sizeof(float));
  for (i = 0; i < num; i++)
    bank[i] = src[i];
  for (i = 2; i < num - 2; i++) {
    src[i] = (float)((bank[i - 2] + bank[i - 1] * 2 + bank[i + 1] * 2 + bank[i + 2]) / 12.0F + bank[i] / 2.0F + 5.0F);
  }
  free(bank);
}

void CreateFeature::MedianSmoothArray(float *src, int num) {
  float *bank, *tmp;
  int   i, j, t;
  int   index, curIndex;
  float smoothed;
  //int Median_Smooth_Range = 5;

  bank = (float *)calloc(num, sizeof(float));
  assert(bank);
  tmp = (float *)calloc(num, sizeof(float));
  assert(tmp);

  for (index = 0; index < num; index++)
    bank[index] = src[index];

  //  use the median src value as the final src value
  assert(Median_Smooth_Range % 2 != 0);
  int   radius = (Median_Smooth_Range - 1) / 2;

  for (t = num - 1; t >= 0; t--) {
    curIndex = t;
    //  if curIndex equals to the stopBackIndex, this means the rest of them are the same
    //  as before, so it's not necessary to do it again for them
    if (t - radius < 0 || t + radius >= num) {
      smoothed = src[curIndex];
    } else {
      int    outerIndex, innerIndex;
      float temp;
      for (i = -radius; i <= radius; i++) {
        index = curIndex + i;
        tmp[index] = src[index];
      }

      //  sort them and get the median value
      for (i = -radius; i <= 0; i++) {
        outerIndex = curIndex + i;
        for (j = i + 1; j <= radius; j++) {
          innerIndex = curIndex + j;
          if (tmp[outerIndex] < tmp[innerIndex]) {
            temp = tmp[outerIndex];
            tmp[outerIndex] = tmp[innerIndex];
            tmp[innerIndex] = temp;
          }
        }
      }
      smoothed = tmp[curIndex];
    }
    bank[curIndex] = smoothed;
  }

  for (index = 0; index < num; index++) {
    src[index] = bank[index];
  }

  free(tmp);
  free(bank);
}

int CreateFeature::AddFrameNum(int num) {
  static int outputFrameNum = 0;
  return outputFrameNum += num;
}