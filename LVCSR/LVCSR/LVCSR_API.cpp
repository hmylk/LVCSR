
#include "LVCSR_API.h"
#include "comm.h"
#include "onepass.h"
#include "lvcsrcfg.h"
#include "arg.h"
#include "ConfusionNet.h"
#include "comm.h"
#include "comm_struct.h"
#include "MultiAlign.h"
#include "TSRBuffer.h"
#include "logfile.h"
#include <map>
#include <Windows.h>
#include <stdio.h>
using namespace std;

/*
*license
*/
int maxSesNum = 1;   //the max number of license permitted
char LICENSEFILE[256];

/*
*
*/
#define RSTLIMIT 8192
char256 systemDir;
char256 SaveData;    // 是否保存检测到的语音段
char256 SaveFeature; // 是否保存相应特征
char256 szFeatDir;   // 特征路径
int ElementsInSrcFeature;
int SampleRate = 16000;
int sessionNum = 0;

/*
*resource 
*/
bool bOnlineMode = false;
CTSRBuffer **Buffer_;
CycLine *SessionPool = NULL;
bool bUseFixedDecodeNum = true; // 事先启动固定数目的解码线程，还是根据需要动态启动、释放
LateralLink *FeaturePools = NULL;
//decoder resource
int decoderNum = 0;
int  g_DecoderInitAcousticModelOnly = false;
OnePass **onepass;
CycLine *DecoderPool = NULL;
static int OnepassThreadOK[128]={0};
LvcsrConfig *g_LVCSRcfg=NULL;


/*
*log and  cfg file
*/
LogFile *ApiLog = NULL;
LogFile *pflog;
LvcsrConfig *Lvcsrcfg = NULL; 
bool bLogToConsole = false;      //print the log to screen at the same time
bool bShowTimeStamp = false;     //ahead every log, print such time info as "[04/25/10 14:14:45:940]:"
int logMaxAccess;                //the max num of log file ,if bigger than this num ,will auto write another file
int logLevel;                    //the log level[0-2], level is more bigger,the log file will more sepcific

/*
*result 
*/
bool g_IndexWithRec = false;
bool  printSegTimeOfWord=false;
bool  printPunctuation=false;
bool bCallBackSorted = true;   // 由于分段解码可能后来的先解完，是等待前续完成后顺序返回，还是即完即回
bool bCallBackCreateThread = false; // 回调起线程，防止回调时间太长阻塞解码；一般都很快，弃用；
char256 CNFormat;
LVCSR_ResultCallback *pResultCallBack = NULL;      // 结果回掉
LVCSR_ResultCallback *pRecResultCallBack = NULL;
int DecodeATask(float *newSet, int frameNum, DWORD SessionId, DWORD SavedFileIdx, DWORD DataTimestamp, DWORD DataTimestampEnd, int decoderId, SOUND_TYPE soundType, int SegIdxInFile);
void CountWordTimeInSentence(Task* pResult,int numberofsentence,int sessId);
int CallBackResult(void* lpParam);
LVCSR_ERROR CallbackSessionResult(int sessionId,bool bIsOver); //返回一路的分段识别结果
void AddSortedResToVector(Task* resArray,int numberOfSentences,int SessionId);
void CopyResFromDecoder(void* pSrcRes,WordResult*& des);
void FillResultLink(int sessionId); //填充结果链表FeSegInfo
void clrSessResMem(int sessionId);
void clrWord(WordResult *startw);
/*
*
*/
CRITICAL_SECTION CS_READCONFIG;
extern "C" CRITICAL_SECTION CS_LTqsort;  //RecEngine.lib 使用
CRITICAL_SECTION CS_LTqsort;             //RecEngine.lib 使用


/*
*work thread
*/
#ifdef WIN32
DWORD WINAPI DecodeProcThread(LPVOID parameter);
DWORD WINAPI DecodeProcStartThread(LPVOID parameter);
#else
static void *DecodeProcThread(void *parameter);
static void *DecodeProcStartThread(void * parameter);
extern int CreateThread(pthread_t *thread, void *(*start_routine)(void*), void *arg, const int stacksize=120*1024, int bdetached = 0);
#endif



LVCSR_API_API int LVCSR_Init(const char* sysdir,const char*  cfgFile,int NumberOfSession)
{
	LVCSR_ERROR retval = LVCSR_SUCCESS;
	if (cfgFile[0]=='\0')
		return LVCSR_ERROR_COMMON;

	InitializeCriticalSection(&CS_READCONFIG);

	char *sysDir = NULL, *configFile = NULL;
	sysDir = (char*)sysdir, configFile = (char*)cfgFile;
	strcpy(systemDir, sysDir);
	if (systemDir[0])
	{	
		if (systemDir[strlen(systemDir)-1] == '/' || systemDir[strlen(systemDir)-1] == '\\')
			strcat(systemDir, "scripts/");
		else
			strcat(systemDir, "/scripts/");
	}
	char256 configFilePath;
	sprintf(configFilePath, "%s%s", systemDir, configFile);
	if (access(configFilePath,0)!=0)
	{
		sprintf(configFilePath, "./%s", configFile);
		if (access(configFilePath,0)!=0)
		{
			printf("configFile %s not exist in %s", configFile, systemDir);
			return LVCSR_ERROR_COMMON;
		}
		else strcpy(systemDir, "./");
		strcpy((char*)sysdir, "../");
	}
	int nPathLen = strlen(systemDir);

	/*1.init the cfg object and read cfg file*/
	int argc=3;
	char *argv[3];
	argv[0]=(char*)malloc(sizeof(char)*256);
	argv[1]=(char*)malloc(sizeof(char)*256);
	argv[2]=(char*)malloc(sizeof(char)*256);
	strcpy(argv[0], "onepass.exe");
	strcpy(argv[1], "-config");
	strcpy(argv[2], configFilePath);   
	Arg *arg = new Arg (argc, argv, "TSR_SERVER", "decoding");   
	LvcsrConfig *cfg = new LvcsrConfig(arg);
	char256 apiLogFileName;
	cfg->GetParameter("ElementsInSrcFeature", ElementsInSrcFeature, 39);
	cfg->GetParameter("LogToConsole", bLogToConsole, false);   
	cfg->GetParameter("ShowTimeStampInLog", bShowTimeStamp, false);
	cfg->GetParameter("LogMaxNumberOfAccess", logMaxAccess, 20000000);
	cfg->GetParameter("LogLevel", logLevel, 1);
	cfg->GetParameter("LogFileName", apiLogFileName, "DictationLog.txt");
	ApiLog = new LogFile(apiLogFileName, bLogToConsole, bShowTimeStamp, logMaxAccess, logLevel);
	ApiLog->logoutf("systemDir is [%s]\n", systemDir);
	cfg->GetParameter("SampleRate", SampleRate, 16000);
	cfg->GetParameter("NumberOfSession", sessionNum, 1);
	if (NumberOfSession > 0) sessionNum = NumberOfSession;
	cfg->GetParameter("NumberOfDecoder", decoderNum, 2);
	float decoderNumPerSession;
	cfg->GetParameter("NumberOfDecoderPerSession", decoderNumPerSession, -1.0);
	cfg->GetParameter("isSaveData", SaveData, "false");
	cfg->GetParameter("isSaveFeature", SaveFeature, "false");
	cfg->GetParameter("isOnlineMode", bOnlineMode, false);
	if (!bOnlineMode) ApiLog->logoutf("warning: Not real on-line mode, maybe just Semi-on-line\n");
	cfg->GetParameter("CNFormat", CNFormat, "DaLian");
	ApiLog->logoutf("Results will be given in '%s' format! Notice:\tIf you want text, 'DaLian' should be your choise!\n", CNFormat);
	cfg->GetParameter("CallBackCreateThread", bCallBackCreateThread, false);   // 080420 add this function
	if (bCallBackCreateThread) ApiLog->logoutf("When result callback, will Create Thread\n");
	cfg->GetParameter("isCallBackSorted", bCallBackSorted, true);
	if (!bCallBackSorted) ApiLog->logoutf("warning: Will call back at once without Sorted, maybe out-of-order\n");
	cfg->GetParameter("UseFixedDecodeNum", bUseFixedDecodeNum, false);
	if (bUseFixedDecodeNum) ApiLog->logoutf("When decoding tasks, will use Fixed Decode thread and Num\n");
	else ApiLog->logoutf("warning: When decoding tasks, will Create Thread and release dynamically\n");
	cfg->GetParameter("IndexWithRec",g_IndexWithRec,false);
	cfg->GetParameter("PrintSegTimeOFWord",printSegTimeOfWord,false); 
	cfg->GetParameter("PrintPunctuation",printPunctuation,false);
	// 设置任务缓存、丢弃限制：防止送的太快
	int nMaxNum = -1;
	float fKeptRate = 1.0;
	cfg->GetParameter("MaxNumOfTaskAcc", nMaxNum, -1);
	if (nMaxNum >= 0) ApiLog->logoutf("The upper bound of Accumulated Tasks' Number will be %d\n", nMaxNum);
	cfg->GetParameter("KeptRateOfTaskAcc", fKeptRate, 1.0);
	if (fKeptRate >= 0) ApiLog->logoutf("Only %.1f %% of all Accumulated Tasks will be kept\n", fKeptRate*100);
	//cfg->GetParameter("Sent2CNWordDic",sent2cnDic,"\0");
	//cfg->GetParameter("Sent2Index",bSent2Index,false);

#ifdef WIN32
	cfg->GetParameter("LICENSEFILE", LICENSEFILE,"license.txt");
#else
	cfg->GetParameter("LICENSEFILE", LICENSEFILE,"license.dat");
#endif

	delete arg;
	delete cfg;
	arg=NULL;
	cfg=NULL;
	if (sessionNum < 1) sessionNum = 1;
	if (bOnlineMode) //在线模式：多少个session就有多少个decoder
		decoderNum = sessionNum;
	else if (decoderNumPerSession > 0)//此处对应离线模式：decoder的个数可以超过外面的session个数，但是不能超过license的最大线数。
	{
		decoderNum = int (0.5 + decoderNumPerSession * sessionNum);   // 比例优先
		//if (decoderNum>maxSesNum)
		//	decoderNum = maxSesNum;
	}
	if (decoderNum < 1) decoderNum = 2;
	if (!FeaturePools)
	{
		if (!bOnlineMode) //离线模式，所有的session公用一个任务链表
			FeaturePools = new LateralLink;
		else 
			FeaturePools = new LateralLink[sessionNum]; //在线模式：创建sessionNum个任务链表
	}


	int ssnum=1;
	if(bOnlineMode) ssnum = sessionNum;
	for (int i=0; i<ssnum; i++)
	{
		if (nMaxNum<0) 
			FeaturePools->SetMaxNum(nMaxNum);
		else if (!bOnlineMode) 
			FeaturePools->SetMaxNum(nMaxNum);
		else 
			FeaturePools->SetMaxNum(ceil((float)nMaxNum/sessionNum));
		FeaturePools->SetKeptRate(fKeptRate);
		if (bUseFixedDecodeNum && nMaxNum >= 0 && nMaxNum < decoderNum)
		{
			FeaturePools->SetMaxNum(decoderNum);
		}
	}
	if (bUseFixedDecodeNum && nMaxNum >= 0 && nMaxNum < decoderNum)
	{
		nMaxNum = decoderNum, FeaturePools->SetMaxNum(nMaxNum);	
	}

	
	DecoderPool = new CycLine(decoderNum);
	SessionPool = new CycLine(sessionNum);
	pResultCallBack = (LVCSR_ResultCallback*)malloc(sizeof(LVCSR_ResultCallback)*sessionNum);
	if(g_IndexWithRec)
	{
		pRecResultCallBack = (LVCSR_ResultCallback*)malloc(sizeof(LVCSR_ResultCallback)*sessionNum);
	}


	/*	2, 读入onepass解码器相关参数*/
	char256 scheduler,outDir,logFileName,TaskListFile;
	char256 cmdHmmParamFile;   // for second pass HMM param model
	cmdHmmParamFile[0] = '\0';
	arg = new Arg (argc, argv, "onepass", "decoding");   // 
	cfg = new LvcsrConfig(arg), Lvcsrcfg = cfg;
	strcpy(logFileName, systemDir), strcpy(scheduler, systemDir), strcpy(outDir, systemDir), strcpy(TaskListFile, systemDir), strcpy(szFeatDir, systemDir);
	cfg->GetParameter("LogFileName",logFileName,"onepass.log");   // + nPathLen
	cfg->GetParameter("Scheduler",scheduler + nPathLen,"\0");
	if (strcmp(scheduler, systemDir) == 0) scheduler[0] = '\0';
	cfg->GetParameter("OutputDir",outDir + nPathLen,"\0");
	if (strcmp(outDir, systemDir) == 0) outDir[0] = '\0', ApiLog->logoutf("\nOutput directory must be defined.\n"); //printf("\nOutput directory must be defined.\n");
	cfg->GetParameter("TaskListFile",TaskListFile + nPathLen,"\0");
	cfg->GetParameter("FeatureRootDir", szFeatDir + nPathLen,"\0");
	if(access(szFeatDir,0)!=0) 
	{
		ApiLog->logoutf("FeatDir '%s' not exists, will make it\n", szFeatDir);
#ifdef WIN32
		if (_mkdir(szFeatDir) == 0) ApiLog->logoutf("\tdirectory '%s' was successfully created\n", szFeatDir);
		else ApiLog->logoutf("! Problem creating directory '%s'\n", szFeatDir);
#else
		mkdir(szFeatDir,0755);
#endif
	}
	pflog = new LogFile(logFileName, bLogToConsole, bShowTimeStamp, logMaxAccess, logLevel);

	/*	3,初始化数据处理对象*/
	ApiLog->logoutf("Initialize %d buffers.\n", sessionNum);
	if (logLevel) printf("Initialize %d buffers.\n", sessionNum);
	Buffer_ = (CTSRBuffer**)malloc(sizeof(CTSRBuffer*)*sessionNum);
	for (int i=0; i<sessionNum; i++)
	{
		Buffer_[i] = new CTSRBuffer(systemDir, configFilePath, ApiLog, i);  
		Buffer_[i]->SetParam("isSaveData", SaveData), Buffer_[i]->SetParam("isSaveFeature", SaveFeature);
		pResultCallBack[i] = NULL;
		if (g_IndexWithRec)
			pRecResultCallBack[i] = NULL;
		if (NULL == Buffer_[i]) 
		{
			ApiLog->logoutf("Error: insufficient memory %s,%d",__FILE__,__LINE__);
			printf("Error: insufficient memory %s,%d",__FILE__,__LINE__);
			return LVCSR_ERROR_COMMON;
		}
	}
	
	
	/*	4,初始化解码器对象*/
	ApiLog->logoutf("Initialize %d decoders.\n", decoderNum);
	if (logLevel) printf("Initialize %d decoders.\n", decoderNum);
	memset(OnepassThreadOK,0,128*sizeof(int));
	g_LVCSRcfg=cfg;
	onepass = new OnePass*[decoderNum];	
	for (int i=0; i<decoderNum; i++)
		onepass[i]=new OnePass(systemDir, outDir,scheduler, cmdHmmParamFile);
	if (g_DecoderInitAcousticModelOnly)
	{
		onepass[0]->SetLog(pflog);
		onepass[0]->SetLoadModelOrNot(g_DecoderInitAcousticModelOnly);
		onepass[0]->InitByCfg(g_LVCSRcfg);
#ifdef HAVECHUNK
		onepass[0]->InitForRescore();
#endif
	}
	else 
	{
		onepass[0]->SetLog(pflog);
		onepass[0]->InitByCfg(g_LVCSRcfg);
		onepass[0]->Init();
		onepass[0]->PrepareForWork(MALE);
	}
	OnepassThreadOK[0]=1;
	if(decoderNum>1)
	{
		for (int i=1; i<decoderNum; i++)
		{

#ifdef WIN32
			HANDLE hStartThread_ = CreateThread(NULL, 0, DecodeProcStartThread, onepass+i, CREATE_SUSPENDED, NULL);
			SetThreadPriority(hStartThread_, THREAD_PRIORITY_HIGHEST);
			ResumeThread(hStartThread_);
#else
			pthread_t hStartThread_;
			pthread_create(&hStartThread_, NULL, DecodeProcStartThread, onepass+i);
#endif
		}
	}
	while(1)
	{
		int Jumpflag=1;
		for(int m=0;m < decoderNum;m++)
		{
			if (OnepassThreadOK[m]==0)
			{
				Jumpflag=0;
				break;
			}
		}
		if (Jumpflag==1)
		{
			break;
		}
		Sleep(10);
	}


	delete arg;
	arg=NULL;
	cfg=NULL;
	free(argv[0]);
	free(argv[1]);
	free(argv[2]);


	/*	4,创建解码线程*/
	if (bUseFixedDecodeNum)
	{
		for (int i=0; i<decoderNum; i++)
		{
#ifdef WIN32
			HANDLE hDecodeThread_ = CreateThread(NULL, 0, DecodeProcThread, onepass+i, CREATE_SUSPENDED, NULL);
			ASSERT2(hDecodeThread_, "CreateThread - DecodeATask for UseFixedDecodeNum error!\n");
			DecoderPool->SetAHandle(i, hDecodeThread_);
			SetThreadPriority(hDecodeThread_, THREAD_PRIORITY_HIGHEST);
			ResumeThread(hDecodeThread_);
#else
			CreateThread(NULL, DecodeProcThread, onepass+i, 2048*1024, 1);
#endif
			int decoderId = DecoderPool->GetAFreeCell();
			DecoderPool->SetATaskInfo(decoderId, -1);   // bug in UseFixedDecodeNum, not initialized
			ASSERT2(decoderId >= 0, "GetAFreeCell not well!\n");
		}
	}

	ApiLog->logoutf("\n\n    ***************Initialization Finished***************    \n\n");
	if (logLevel) printf("\n\n    ***************Initialization Finished***************    \n\n");
	return retval;
}

LVCSR_API_API int LVCSR_Exit()
{
	//----clean and destruct
	ApiLog->logoutf("To DelAllTask, just enter LVCSR_Exit\n");
	if (!bOnlineMode && FeaturePools) FeaturePools->DelAllTask();   // 清除剩余任务队列
	if (!bOnlineMode) FeaturePools->SetMaxNum(decoderNum + 1);
	int i=bUseFixedDecodeNum?0:decoderNum, taskId;

	ApiLog->logoutf("Will sent null tasks to end decoder, in LVCSR_Exit\n");
	for (; i<decoderNum; i++)
	{
		Task * aTask = (Task*)malloc(sizeof(Task)*1);
		aTask->newSet = NULL, aTask->frameNum = -1, aTask->decoderId = -1, aTask->sessionId = -1;
		aTask->latSet = NULL;
		if (bOnlineMode && FeaturePools) FeaturePools[i].SetMaxNum(2), FeaturePools[i].DelAllTask();
		taskId = !bOnlineMode?FeaturePools->AddATask(aTask):FeaturePools[i].AddATask(aTask);
		if (taskId < 0)	ApiLog->logoutf("FeaturePools AddATask fail, in LVCSR_Exit\n");
	}

	for (i=0; i<decoderNum; i++)
	{
		ApiLog->logoutf2("Waiting for decoder %d to end \n", i + 1);
		if (logLevel) printf("Waiting for decoder %d to end ", i + 1);
		while(DecoderPool->IsACellWork(i))   // DecoderPool->SetAFreeCell(i, false)!=i
		{
			if (logLevel) printf(".");
			Sleep(500);
		}
		ApiLog->logoutf2(" OK!\n");
		if (logLevel) printf(" OK!\n");
	}
	if (FeaturePools)
	{
		if (!bOnlineMode) 
			delete FeaturePools;
		else 
			delete [] FeaturePools;
		FeaturePools = NULL;
	}

	for (i=0; i<decoderNum; i++)
	{	
		delete (onepass[i]);
		onepass[i]=NULL;
		ApiLog->logoutf("NO. %d decoder exit successfully.\n", i + 1);
		if (logLevel) printf("NO. %d decoder exit successfully.\n", i + 1);
	}
	if(pflog) delete pflog;
	delete []onepass; onepass=NULL;
	for (i=0; i<sessionNum; i++)
	{
		if (Buffer_[i]) delete Buffer_[i];
		ApiLog->logoutf("NO. %d session exit successfully.\n", i + 1);
		if (logLevel) printf("NO. %d session exit successfully.\n", i + 1);
	}
	if (Buffer_) free(Buffer_); Buffer_ = NULL;

	if (SessionPool) delete SessionPool;
	free(pResultCallBack);
	if (g_IndexWithRec)
	{
		free(pRecResultCallBack);
	}
	if (DecoderPool) delete DecoderPool;
	if(Lvcsrcfg) delete Lvcsrcfg;
	Lvcsrcfg = NULL;
	ApiLog->logoutf("\n\n    ***************ThinkIt LVCSR Exit Finished***************    \n\n");
	if (logLevel) printf("\n\n    ***************ThinkIt LVCSR Exit Finished***************    \n\n");
	if(ApiLog) delete ApiLog;
	if(access(szFeatDir,0)!=0) 
	{
#ifdef WIN32
		_rmdir(szFeatDir);
#else
		rmdir(szFeatDir);
#endif
	}
	return LVCSR_SUCCESS;
}


LVCSR_API_API int LVCSR_Start(int sessionId /*=0*/)
{
	if (SessionPool->SetAFreeCell(sessionId, false) < 0)
		sessionId = SessionPool->GetAFreeCell();
	if (sessionId >= 0)
	{
		Buffer_[sessionId]->PermitData();
		ApiLog->logoutf("session %d: Started !\n", sessionId);
		if (logLevel) printf("session %d: Started !\n", sessionId);
		LVCSR_SetParam("isSaveData",SaveData,sessionId); 
		LVCSR_SetParam("isSaveFeature", SaveFeature,sessionId);
		return sessionId;
	}
	else return LVCSR_ERROR_COMMON;
}

LVCSR_API_API int LVCSR_StopRecording(int sessionId)
{
	if (!(SessionPool->IsACellWork(sessionId))) 
		return LVCSR_ERROR_COMMON;
	return Buffer_[sessionId]->StopRecording();
}

LVCSR_API_API int LVCSR_Stop(int sessionId)
{
	if (!(SessionPool->IsACellWork(sessionId))) 
		return LVCSR_ERROR_COMMON;	

	DecoderPool->WaitForASessionStop(sessionId);
	Buffer_[sessionId]->RefuseData();   
	Buffer_[sessionId]->CleanData(); 

	ApiLog->logoutf("session %d: Stopped !\n", sessionId);
	if (logLevel) printf("session %d: Stopped !\n", sessionId);
	SessionPool->SetAFreeCell(sessionId, true);   // 释放占用的路数资源
	pResultCallBack[sessionId] = NULL;   // 清除设置的回调函数
	if (g_IndexWithRec)
	{
		pRecResultCallBack[sessionId] = NULL;
	}
	return LVCSR_SUCCESS;
}

LVCSR_API_API int LVCSR_SendData(FeSegInfo* segInfo,FeSpeechAttri* dataInfo,int sessionId)
{
	if (!(SessionPool->IsACellWork(sessionId))) return LVCSR_ERROR_COMMON;
	return Buffer_[sessionId]->SendData(segInfo,dataInfo);
}

LVCSR_API_API int LVCSR_SetResultCallbackFunc(LVCSR_ResultCallback pFunc, int sessionId)
{
	if (!(SessionPool->IsACellWork(sessionId))) return LVCSR_ERROR_COMMON;
	pResultCallBack[sessionId] = pFunc;
	ApiLog->logoutf2("session %d: Set ResultCallback Function\n", sessionId);
	return LVCSR_SUCCESS;
}
LVCSR_API_API int LVCSR_SetParam(char *paramname, char *paramvalue, int sessionId)
{
	if (!stricmp(paramname,"DecoderInitAcousticModelOnly"))
	{
		if (!stricmp(paramvalue,"true")) 
		{
			g_DecoderInitAcousticModelOnly = true;
		}
		else
			g_DecoderInitAcousticModelOnly = false;
		return LVCSR_SUCCESS;
	}

	if (!(SessionPool->IsACellWork(sessionId))) return LVCSR_ERROR_COMMON;
	LVCSR_ERROR retvalue  = LVCSR_SUCCESS;

	char *paramName = paramname;
	char *paramValue = paramvalue;
	if(!stricmp(paramName, "isSaveData") || !stricmp(paramName, "isSaveFeature"))
	{
		if(Buffer_[sessionId])
			retvalue = (LVCSR_ERROR)(Buffer_[sessionId]->SetParam(paramName, paramValue));
	}
	if (retvalue == LVCSR_SUCCESS) 
	{
		ApiLog->logoutf2("session %d: SET LVCSR Param %s : %s\n", sessionId, paramName, paramValue);
	}
	return retvalue;
}

LVCSR_API_API int LVCSR_SampleRate()
{
	return SampleRate;
}


#ifdef WIN32
DWORD WINAPI DecodeProcStartThread(LPVOID parameter)
#else
static void *DecodeProcStartThread(void * parameter)
#endif
{
#ifndef WIN32
	int priority = -18;
	// Increase the priority
	setpriority(PRIO_PROCESS, 0, priority);
#endif

	int decoderId = (OnePass**)parameter - onepass;
	int ret=0;

	if (g_DecoderInitAcousticModelOnly) 
	{
		EnterCriticalSection(&CS_READCONFIG);
		onepass[decoderId]->InitByCfg(g_LVCSRcfg);
		LeaveCriticalSection(&CS_READCONFIG);
#ifdef HAVECHUNK
		onepass[decoderId]->InitForRescore();
#endif
	}
	else
	{
		EnterCriticalSection(&CS_READCONFIG);
		onepass[decoderId]->InitByCfg(g_LVCSRcfg);
		onepass[decoderId]->Init();
		LeaveCriticalSection(&CS_READCONFIG);
		onepass[decoderId]->PrepareForWork(MALE);   // 提前申请内存，避免启动后内存占用过大分配不到！080401 syq
	}
	OnepassThreadOK[decoderId]=1;
	printf("NO. %d decoder finished initializing.\n", decoderId);


#ifdef WIN32
	return 1;
#else
	// set the priority back to normal
	setpriority(PRIO_PROCESS, 0, 0);
	pthread_exit(NULL);
#endif
}

#ifdef WIN32
DWORD WINAPI DecodeProcThread(LPVOID parameter)
#else
static void *DecodeProcThread(void *parameter)
#endif
{
#ifndef WIN32
	int priority = -18;
	// Increase the priority
	setpriority(PRIO_PROCESS, 0, priority);
#endif
	// 不用动态启动线程, 但不灵活, 且Sleep的时间长则略耽误解码(很牵强太小了), 短则作无谓的浪费
	float *newSet = NULL;
	int frameNum = 0, decoderId = (OnePass**)parameter - onepass, ret;
	long SessionId = decoderId;
	DWORD SavedFileIdx, DataTimestamp, DataTimestampEnd;
	SOUND_TYPE soundType;
	void *pResultArray = NULL;
	int SegIdxInFile;
	LateralLink *myFeaturePools = !bOnlineMode?FeaturePools:FeaturePools+SessionId;
	while (1) 
	{
		Task *aTask;
		int taskId = myFeaturePools->GetATask(aTask);
		if (taskId >= 0) 
		{
			newSet = (float*)(aTask->newSet);
			frameNum = aTask->frameNum;
			SessionId = aTask->sessionId;
			SavedFileIdx = aTask->saveFileIdx;
			DataTimestamp = aTask->dataTimestamp;
			DataTimestampEnd = aTask->dataTimestampEnd;
			soundType = aTask->soundType;
			SegIdxInFile = aTask->decoderId;
			free(aTask);
		}
		else 
		{
			Sleep(10);
			continue;
		}
		if (SessionId<0 || frameNum < 0) break;
		DecoderPool->SetATaskInfo(decoderId, SessionId);
		ret = DecodeATask(newSet, frameNum, SessionId, SavedFileIdx, DataTimestamp, DataTimestampEnd, decoderId, soundType, SegIdxInFile);	
		if (ret != LVCSR_SUCCESS) 
			break;
		DecoderPool->SetATaskInfo(decoderId, -1);
	}
	ApiLog->logoutf2("decoder %d: Get a null task to end\n", decoderId + 1);
	if (logLevel) printf("decoder %d: Get a null task to end\n", decoderId + 1);
	ret = DecoderPool->SetAFreeCell(decoderId);   //-释放decoder,
	if (ret < 0)
	{
		ApiLog->logoutf("decoder %d: SetAFreeCell error!!\n", decoderId + 1);
		printf("decoder %d: SetAFreeCell error!!", decoderId + 1);
	}
#ifdef WIN32
	return ret;
#else
	// set the priority back to normal
	setpriority(PRIO_PROCESS, 0, 0);
	pthread_exit(NULL);
#endif
}

int CleanZero(float* srcFeat,float*& DestFeat,int frameNum)
{
	int ElementsInSrcFeatureTemp = (ElementsInSrcFeature+ 3)&(~3);
	DestFeat = new float[(frameNum*ElementsInSrcFeature+1)*sizeof(float)];
	memset(DestFeat,0,(frameNum*ElementsInSrcFeature+1)*sizeof(float));
	if (NULL == DestFeat)
	{
		ApiLog->logoutf("[DecodeATask]: alloc memory error \n");
		return LVCSR_ERROR_COMMON;
	}
	float* temp= srcFeat;
	float* temp2= DestFeat;
	int i=0;
	while(i<frameNum)
	{
		memcpy(temp2,temp,ElementsInSrcFeature*sizeof(float));
		temp+=ElementsInSrcFeatureTemp;
		temp2+=ElementsInSrcFeature;
		i++;
	}
}

int DecodeATask(float *newSet, int frameNum, DWORD SessionId, DWORD SavedFileIdx, DWORD DataTimestamp, DWORD DataTimestampEnd, int decoderId, SOUND_TYPE soundType, int SegIdxInFile)
{
	int wordNum = 0;
	void *pResultArray = NULL; 
	void* pWordResultArray = NULL;

	if (decoderId < 0 || frameNum < 0) 
		return LVCSR_ERROR_COMMON; 

	OnePass* aOnepass = *(onepass + decoderId);
	// 解码识别
#ifdef _encrypt
	int returnValue= CheckCopyRight();
	if (returnValue < 0) exit(0);
#endif

	TSR_STREAM_FLAG flag;
	if (!bOnlineMode) flag = TSR_STREAM_ALL;
	else if (SegIdxInFile==0) flag = TSR_STREAM_START;
	else if (SegIdxInFile>0&&!((Buffer_[SessionId])->bTaskSendToDecoder)) flag = TSR_STREAM_START; 
	else if(SegIdxInFile>0) flag = TSR_STREAM_CONT;
	else if (SegIdxInFile==-1&&!((Buffer_[SessionId])->bTaskSendToDecoder)) flag = TSR_STREAM_ALL;
	else if(SegIdxInFile==-1) flag = TSR_STREAM_END;
	else if (SegIdxInFile==-2) flag = TSR_STREAM_ALL;
	if (flag==TSR_STREAM_ALL || flag==TSR_STREAM_START) 
		ApiLog->logoutf2("session %d: %dth task assigned to decoder %d\n", SessionId, SavedFileIdx + 1, decoderId + 1);
	int tcount;
	if (newSet!=NULL && frameNum>0) 
	{
		(Buffer_[SessionId])->bTaskSendToDecoder = true;
		if (flag==TSR_STREAM_ALL || flag==TSR_STREAM_START) 
		{
			aOnepass->PrepareForWork(soundType);
		}
		if (flag==TSR_STREAM_END||flag == TSR_STREAM_ALL)
		{
			(Buffer_[SessionId])->bTaskSendToDecoder =false;
		}
		
		/*剔除特征内存中用于对齐的0*/
		float* newSet2=NULL;
		CleanZero(newSet,newSet2,frameNum);

		char featureName[256];
		sprintf(featureName, "%s_seg%03d.plp", Buffer_[SessionId]->sessWavId, SavedFileIdx + 1);
        bool b = aOnepass->DoFeature(newSet2, frameNum, featureName, flag);
		DecoderPool->SetATaskInfo(decoderId, SessionId);
		aOnepass->DoUtterance();
		if (newSet2)
		{
			delete[] newSet2,newSet2 = NULL;
		}
#ifndef onepass_exe
		if (flag==TSR_STREAM_ALL || flag==TSR_STREAM_END)
		{
			//LatticeResult * pLatRst = NULL;
			//pLatRst = (LatticeResult *)aOnepass->GetLatticeResult();
			//aOnepass->OutputHTKLattice(orgfeatureName); //
			pResultArray = aOnepass->GetCN(DataTimestamp, DataTimestampEnd, CNFormat, wordNum);   // 必须是新申请的空间, 否则必须拷贝以防冲掉
			if (g_IndexWithRec)
			{
				pWordResultArray = aOnepass->GetCN(DataTimestamp,DataTimestampEnd,"DaLian",wordNum);
			}
			if (pResultArray==NULL)
			{
				ApiLog->logoutf("GetCN  no result!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
				printf("GetCN  no result!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			}
			if (g_IndexWithRec) 
			{
				if (pWordResultArray == NULL)
					ApiLog->logoutf("Index mode: GetNBest no result!!!!!!!!!!!!!!!!!!!\n");
			}
		 }
#endif
	}

	if (newSet) 
	{
		Free32(newSet);
		newSet=NULL;
	}
	
	if (!(flag==TSR_STREAM_ALL || flag==TSR_STREAM_END))
		return LVCSR_SUCCESS;
	ApiLog->logoutf("session %d: %dth task finished by decoder %d in %d count\n", SessionId, SavedFileIdx + 1, decoderId + 1, tcount);
	if (logLevel) printf("session %d: %dth task finished by decoder %d\n", SessionId, SavedFileIdx + 1, decoderId + 1);
	
	/*结果处理*/
	WordResult* pResultCopy = NULL;
	Task *aResult = (Task*)malloc(sizeof(Task)*1);
	if (g_IndexWithRec&&!strcmp(CNFormat,"KWS")) //关键词+识别
	{
		CopyResFromDecoder(pWordResultArray,pResultCopy);
		aResult->newSet = pResultArray;
		aResult->latSet = pResultCopy;
		/*if (bSent2Index)
		{
			delete pResultArray;
			aResult->newSet = convert2cn[SessionId].GetCN(pResultCopy,(float)DataTimestamp/SampleRate,(float)DataTimestampEnd/SampleRate);
		}*/
	}
	else if (!g_IndexWithRec&&!strcmp(CNFormat,"DaLian"))//纯识别模式
	{
		CopyResFromDecoder(pResultArray,pResultCopy);
		aResult->newSet = pResultCopy;
		aResult->latSet = NULL;
	}
	else if (!g_IndexWithRec&&!strcmp(CNFormat,"KWS"))//纯关键词模式
	{
		aResult->newSet = pResultArray;
		aResult->latSet = NULL;
	}
	aResult->frameNum = wordNum;
	aResult->sessionId = SessionId;
	aResult->saveFileIdx = SavedFileIdx;
	aResult->dataTimestamp = DataTimestamp;
	aResult->dataTimestampEnd = DataTimestampEnd;
	
	bool bGet = true;
	int retn = (SessionPool->IsACellWork(SessionId))?Buffer_[SessionId]->ResultPool->InsertATask(aResult,bGet,Buffer_[SessionId]->SentFileIdx,Buffer_[SessionId]->SavedFileIdx):-1;
	ApiLog->logoutf("session %d: %dth task  [%s] insertAtask by decoder %d\n", SessionId, SavedFileIdx + 1,pResultCopy->cands->text, decoderId + 1);
	if (retn > 0)
	{
		aResult->soundType = (SOUND_TYPE)retn;
		ApiLog->logoutf2("session %d: To Directly result CallBack without CreateThread start from %d\n", SessionId, aResult->saveFileIdx + 1);
		aResult->decoderId = NULL;
		CallBackResult(aResult);
		ApiLog->logoutf2("session %d: Directly result CallBack without CreateThread start from %d OK\n", SessionId, aResult->saveFileIdx + 1);
	}
	pResultArray = NULL;
	return LVCSR_SUCCESS;
}

void CopyResFromDecoder(void* pSrcRes,WordResult*& pDes)
{
	/*结果拷贝和转换*/
	WordResult* pResultSrc =(WordResult*)pSrcRes;
	if(pResultSrc!=NULL&&pResultSrc->cands!=NULL&&pResultSrc->cands->text!=NULL)
	{
		pDes = (WordResult*)malloc(sizeof(WordResult)*1);
		pDes->cands = NULL;
		int lenText = strlen(pResultSrc->cands->text);
		int lenPhone = strlen(pResultSrc->cands->phone);
		int lenSegtime = strlen(pResultSrc->cands->segTime);
		if (lenText == 0)
		{
			pDes->cands= (ACand *)malloc(sizeof(ACand)*1);
			pDes->cands->text = new char[1];
			pDes->cands->phone = new char[1];
			pDes->cands->segTime=new char[1];
			*(pDes->cands->text)='\0';
			*(pDes->cands->phone)='\0';
			*(pDes->cands->segTime)='\0';
			pDes->next = NULL;
			pDes->candNum = 1;
			pDes->cands->next= NULL;
		}
		else
		{
			pDes->candNum = pResultSrc->candNum;
			int candsNum = pResultSrc->candNum;
			int cn=0;
			ACand* curCpyCand = pDes->cands;
			ACand* curSrcCand = pResultSrc->cands;
			while(curSrcCand!=NULL)//构建多候选链表
			{
				ACand* newACand = (ACand *)malloc(sizeof(ACand)*1);
				lenText = strlen(curSrcCand->text);
				lenPhone = strlen(curSrcCand->phone);
				lenSegtime = strlen(curSrcCand->segTime);

				newACand->text = new char[lenText+1];
				memset(newACand->text,0,lenText+1);
				memcpy(newACand->text,curSrcCand->text,(lenText)*sizeof(char));

				newACand->phone = new char[lenPhone+1];
				memset(newACand->phone,0,lenPhone+1);
				memcpy(newACand->phone,curSrcCand->phone,lenPhone*sizeof(char));

				newACand->segTime=new char[lenSegtime+1];
				memset(newACand->segTime,0,lenSegtime+1);	
				memcpy(newACand->segTime,curSrcCand->segTime,lenSegtime*sizeof(char));

				newACand->startTime = curSrcCand->startTime;
				newACand->endTime = curSrcCand->endTime;
				newACand->score = curSrcCand->score;
				newACand->next = NULL;

				if(pDes->cands==NULL)
				{
					pDes->cands = newACand;
					curCpyCand=newACand;
				}
				else
				{
					curCpyCand->next = newACand;
					curCpyCand = newACand;
				}
				curSrcCand=curSrcCand->next;
			}
			pDes->next = NULL;
		}
	}
	else
	{
		pDes = NULL;
	}


	/*重复结果处理*/
	if (!stricmp(CNFormat, "DaLian"))
	{
		map<string, int> Resultmap; 
		int n=0;
		WordResult *tempw = (WordResult *)pDes;
		if(pDes != NULL&&tempw->cands!=NULL&&tempw->cands->text!=NULL)
		{
			Resultmap.insert(pair<string,int>(tempw->cands->text, n)); 
			n++;
			ACand *tempc = tempw->cands->next, *lastc;
			ACand *cur = tempw->cands;
			while (tempc) 
			{ 
				//删除重复的结点
				if(Resultmap.find(tempc->text) != Resultmap.end())
				{
					free(tempc->text);
					free(tempc->phone);
					free(tempc->segTime);
					lastc = tempc; tempc = tempc->next; delete (lastc);
					cur->next = NULL;
				}
				else
				{
					Resultmap[tempc->text] = n;
					n++;
					cur->next = tempc;
					cur = tempc;
					tempc=tempc->next;			
				}
			}
			tempw->candNum = n;
		}
		else
		{
			ApiLog->logoutf("pResultArray is null or cands is null or cands->text is null\n"); 
		}
	}
}

int CallBackResult(void* lpParam)
{
	LVCSR_ERROR ret;
	Task *parameter = (Task*)lpParam;
	int SessionId = parameter->sessionId;
	int SavedFileIdx = parameter->saveFileIdx;
	int numberOfSentences = parameter->soundType;
	int tcount=0;
	if (bCallBackSorted) 
	{	
		while (Buffer_[SessionId]->SentFileIdx < SavedFileIdx) 
			Sleep(10);
	}
	ApiLog->logoutf("session %d: %dth tasks' Result past %d\n", SessionId, SavedFileIdx + 1, Buffer_[SessionId]->SentFileIdx + 1);
	while(numberOfSentences > 0) 
	{
		ApiLog->logoutf("session %d: %d tasks' Result Call Back, start from %d\n", SessionId, numberOfSentences, SavedFileIdx + 1);
		if (logLevel) printf("session %d: %d tasks' Result Call Back, start from %d\n", SessionId, numberOfSentences, SavedFileIdx + 1);
		if(parameter)
		{	
			if (printSegTimeOfWord)
			{
				CountWordTimeInSentence(parameter,numberOfSentences,SessionId);
			}

			/*缓存取出的结果*/
			AddSortedResToVector(parameter,numberOfSentences,SessionId);
			
			/*在线情况下，分句返回结果*/
			if (bOnlineMode)
			{
				if (Buffer_[SessionId]->bEndRecord && (Buffer_[SessionId]->SavedFileIdx == SavedFileIdx + numberOfSentences))
				{
					ret = CallbackSessionResult(SessionId,true);
				}
				else
				    ret = CallbackSessionResult(SessionId,false);
				free(parameter),parameter=NULL;
				Buffer_[SessionId]->vecSortedResult.clear();
			}
		}
		else 
			ret = LVCSR_ERROR_COMMON;

		//
		Buffer_[SessionId]->SentFileIdx = SavedFileIdx + numberOfSentences;
		
		if (Buffer_[SessionId]->bEndRecord && (Buffer_[SessionId]->SavedFileIdx == SavedFileIdx + numberOfSentences))
		{
			/*离线情况下，整个wav一次返回*/
			if (!bOnlineMode) 
			{
				ret = CallbackSessionResult(SessionId,true);
				free(parameter),parameter=NULL;
			}
			ApiLog->logoutf("session %d: all %d tasks LVCSR_RECOGNITION_COMPLETE\n", SessionId, SavedFileIdx + numberOfSentences);
			if (logLevel) printf("session %d: all %d tasks LVCSR_RECOGNITION_COMPLETE\n", SessionId, SavedFileIdx + numberOfSentences);
		}
		

		Task* para = NULL;   // 仿照解码器增加此处查库存机制
		numberOfSentences = (SessionPool->IsACellWork(SessionId))?Buffer_[SessionId]->ResultPool->InsertATask(para,true,Buffer_[SessionId]->SentFileIdx,Buffer_[SessionId]->SavedFileIdx):-1;
		if (numberOfSentences > 0)
		{
			parameter = para;
			SavedFileIdx = parameter->saveFileIdx;
			ApiLog->logoutf2("session %d: in thread CallBack, get another %d result, start from %d\n", SessionId, numberOfSentences, SavedFileIdx + 1);
		}
	}
#ifdef WIN32
	return ret;
#else
	if (bCallBackCreateThread) 
	{
		ncallbackcnt--;
		setpriority(PRIO_PROCESS, 0, 0);  // set the priority back to normal
		pthread_exit(NULL);
	}
#endif
}

//缓存顺序结果到vector中
void AddSortedResToVector(Task* resArray,int numberOfSentences,int SessionId)
{
	if (resArray==NULL||numberOfSentences==0)
	{
		return;
	}
	for (int i=0;i<numberOfSentences;i++)
	{
		Buffer_[SessionId]->vecSortedResult.push_back(&(resArray[i]));
		ApiLog->logoutf("session %d: insert %s to vector\n", SessionId,((WordResult*)(resArray[i].newSet))->cands->text );
		if (logLevel) printf("session %d: insert %s to vector\n", SessionId,((WordResult*)(resArray[i].newSet))->cands->text);
	}
	
}

LVCSR_ERROR CallbackSessionResult(int sessionId,bool bIsOver)
{
	/*填充结果链表*/
	FillResultLink(sessionId);

	/*回调返回结果*/
	if (pResultCallBack[sessionId])
	{
		(*pResultCallBack[sessionId])(Buffer_[sessionId]->pResultLink,bIsOver,sessionId);
	}

	/*释放相关内存*/
	clrSessResMem(sessionId);

	return LVCSR_SUCCESS;
}

void FillResultLink(int sessionId)
{
	int count=0;
	FeSegInfo* temp = Buffer_[sessionId]->pResultLink;
	vector<Task*>::iterator iterSent = Buffer_[sessionId]->vecSortedResult.begin();
	while(iterSent!=Buffer_[sessionId]->vecSortedResult.end()&&temp)
	{
		if (temp->speech_type==RT_VOICE)
		{
			WordResult *tempw = (WordResult *)((*iterSent)->newSet); 
			if (tempw==NULL)//表示该分段解码结果为空
			{
				iterSent++;
				temp = temp->next;
				count++;
				continue;
			}
			int lenText = strlen(tempw->cands->phone);
			int lenSegtime = strlen(tempw->cands->segTime);
			temp->pText = new char[lenText+lenSegtime+2];
			memset(temp->pText,0,lenText+lenSegtime+2);
			strcpy(temp->pText,tempw->cands->phone);
			strcat(temp->pText,"|");
			strcat(temp->pText,tempw->cands->segTime);
			ApiLog->logoutf("session %d[%d]: %s\n", sessionId,count,temp->pText);
			if (logLevel) printf("session %d[%d]: %s\n", sessionId,count,temp->pText);
			iterSent++;
		}
		temp = temp->next;
		count++;
	}
}


void CountWordTimeInSentence(Task* pResult,int numberofsentence,int sessId)
{
	if (pResult==NULL)
	{
		ApiLog->logoutf2("[CountWordTimeInSentence]:recognition result is null\n");
		return;
	}
	for (int  i=0;i<numberofsentence;i++)
	{
		WordResult *tempw = NULL;
		if (g_IndexWithRec&&!strcmp(CNFormat,"KWS")) //关键词+识别
		{
			tempw = (WordResult *)(pResult[i].latSet);
		}
		else if (!g_IndexWithRec&&!strcmp(CNFormat,"DaLian"))//纯识别模式
		{
			tempw = (WordResult *)(pResult[i].newSet);
		}
		else if (!g_IndexWithRec&&!strcmp(CNFormat,"KWS"))//纯关键词模式
		{
			tempw = NULL;
		}	
		if (!tempw) continue;

		if (tempw->cands->phone==NULL||tempw->cands->segTime==NULL)
		{
			return;
		}
		if (tempw->cands->phone[0]=='\0')
		{
			continue;
		}

		float statSeconds=(float)pResult[i].dataTimestamp/(SampleRate);
		float* timeArray=new float[RSTLIMIT];
		if (timeArray==NULL)
		{
			printf("[CountWordTimeInSentence]:allocate memery error\n");
		}
		char newChrSegTimeArray[RSTLIMIT];
		memset(newChrSegTimeArray,0,RSTLIMIT);
		char* pNewChrTime=newChrSegTimeArray;//保存数组的首地址，方便后面的strcpy
		float *pTime = timeArray;

		char* tempb=tempw->cands->segTime;
		if (*tempb==' ')
		{
			tempb++;
		}
		char* tempd=tempb;
		while((*tempd)!='\0')
		{
			while (((*tempd)!=' ')&&((*tempd)!='\0'))
			{
				tempd++;
			}
			if ((*tempd)==' ')
			{
				char atime[256]="";
				strncpy(atime,tempb,tempd-tempb);
				float timeSecond =atof(atime);
				*pTime=timeSecond+statSeconds;
				atime[0]='\0';
				sprintf(atime,"%.2f",*pTime);
				strcat(newChrSegTimeArray,atime);
				strcat(newChrSegTimeArray," ");
				tempd+=1;
				tempb=tempd;
				pTime++;
			}
		}
		if (*tempd=='\0')
		{
			char atime[256]="";
			strncpy(atime,tempb,tempd-tempb);
			float timeSecond =atof(atime);
			*pTime=timeSecond+statSeconds;
			atime[0]='\0';
			sprintf(atime,"%.2f",*pTime);
			strcat(newChrSegTimeArray,atime);
			strcat(newChrSegTimeArray," ");
		}
		*(++pTime) ='\0';
		if (strlen(tempw->cands->segTime)<strlen(pNewChrTime))
		{
			delete[] tempw->cands->segTime;
			tempw->cands->segTime = new char[strlen(pNewChrTime)+1];
			memset(tempw->cands->segTime,0,strlen(pNewChrTime)+1);
			memcpy(tempw->cands->segTime,pNewChrTime,sizeof(char)*strlen(pNewChrTime));//将新计算的词的时间信息存回到task结构的segtime中
		}
		else
			memcpy(tempw->cands->segTime,pNewChrTime,sizeof(char)*strlen(tempw->cands->segTime));//将新计算的词的时间信息存回到task结构的segtime中
		//end
		if (printPunctuation)
		{
			//JustResultForPunctuation(tempw->cands->phone,timeArray);
		}	
		delete[] timeArray;
		timeArray = NULL;
	}
}

void clrSessResMem(int sessionId)
{
	/*释放开辟的识别结果内存*/
	FeSegInfo* temp = Buffer_[sessionId]->pResultLink;
	while(temp!=NULL)
	{
		if (temp->speech_type==RT_VOICE)
		{
			if (temp->pText)
			{
				delete[] temp->pText;
				temp->pText = NULL;
			}
		}
		temp = temp->next;
	}

	/*释放引擎本身task内存*/
	vector<Task*>::iterator iterSent = Buffer_[sessionId]->vecSortedResult.begin();
	while(iterSent!=Buffer_[sessionId]->vecSortedResult.end())
	{	
		if ((*iterSent)->newSet)
		{
#ifndef onepass_exe
			if (!stricmp(CNFormat, "KWS")) 
			{
				delete (ConfusionNet*)((*iterSent)->newSet);   
				if (g_IndexWithRec) 
				{
					if ((*iterSent)->latSet)
						clrWord((WordResult*)((*iterSent)->latSet));
				}
			}
			else if (!stricmp(CNFormat, "DaLian")) 
				clrWord((WordResult*)((*iterSent)->newSet));
#endif
		}
		iterSent++;
	}
}

void clrWord(WordResult *startw)
{
	WordResult *tempw = startw, *lastw;
	while (tempw) 
	{   // 清空CN信息内存
		lastw = tempw->next;
		ACand *tempc = tempw->cands, *lastc;
		while (tempc) 
		{ 
			delete[] tempc->text; 
#ifdef ENHANCED_RESULT
			if(tempc->phone!=NULL)	
				delete[] (tempc->phone); 
			if(tempc->segTime!=NULL) 
				delete[] (tempc->segTime);
#endif
			lastc = tempc; tempc = tempc->next; free(lastc);
		}
		free(tempw);
		tempw = lastw;
	}
}