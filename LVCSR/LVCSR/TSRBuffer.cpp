

#include "TSRBuffer.h"
#include <assert.h>

extern CycLine  *DecoderPool;
extern LateralLink *FeaturePools;
extern bool bOnlineMode;
extern LVCSR_ResultCallback *pResultCallBack;

CTSRBuffer::CTSRBuffer(char* sysDir, char* configFile, LogFile* ctLogFile, int sessionId)
{
	SampleRate = LVCSR_SampleRate();
	SessionId = sessionId;
	logFile = ctLogFile;
	strcpy(systemDir, sysDir);

	aCreateFeature = new CreateFeature;
	aCreateFeature2 = new CreateFeature;
	aCreateFeature->InitFeatureCfg(sysDir, configFile, bOnlineMode);
	aCreateFeature2->InitFeatureCfg(sysDir, configFile, bOnlineMode);

	ResultPool = new LateralLink;

	if (!bOnlineMode) myFeaturePools = FeaturePools;
	else myFeaturePools = FeaturePools+SessionId;

	bTaskSendToDecoder=false;
	ResetEnv();
}

CTSRBuffer::~CTSRBuffer()
{
	aCreateFeature->CleanFeature();
	aCreateFeature2->CleanFeature();
	if (aCreateFeature)
	{
		delete aCreateFeature;
		aCreateFeature = NULL;
	}
	if (aCreateFeature2)
	{
		delete aCreateFeature2;
		aCreateFeature2 = NULL;
	}
	ResultPool->DelAllTask();
	delete ResultPool;
}

int CTSRBuffer::ResetEnv()
{
	IsPermitReceiveData = false;
	bSaveData = false;
	bSaveFeature = false;
	SentFileIdx = 0;
	SavedFileIdx = 0;
	SegIdxInFile=0;
	ResultPool->DelAllTask(); 
	memset(sessWavId,0,256);
	if (myFeaturePools) myFeaturePools->DelAllTask(SessionId);
	bTaskSendToDecoder = false;
	bEndRecord = false;
    pResultLink = NULL;
	vecSortedResult.clear();
	return 0;
}

int CTSRBuffer::SetParam(char * paramName, char * paramValue)
{
	if (!stricmp(paramName, "isSaveData"))
		bSaveData = ConvertStrToInt(paramName, paramValue)?true:false;
	else if (!stricmp(paramName, "isSaveFeature"))
		bSaveFeature = ConvertStrToInt(paramName, paramValue)?true:false;
	else if (!stricmp(paramName, "timestamp") && atof(paramValue)>=0)
		;//DataTimestamp = int(atof(paramValue) * SampleRate);
	else return -1;
	return 0;
}

int CTSRBuffer::ConvertStrToInt(const char *optName, char *valStr)
{
	int i;
	if (!(stricmp(valStr,"0") && stricmp(valStr,"false") && stricmp(valStr,"no")))
		return 0;
	else if (!(stricmp(valStr,"1") && stricmp(valStr,"true") && stricmp(valStr,"yes")))
		return 1;
	else 
	{
		printf("Option: Invalid value '%s' specified for option -%s\n", valStr, optName);   // tprintf7
		exit(-1);
	}
}


int  CTSRBuffer::SendData(FeSegInfo* segInfo,FeSpeechAttri* dataInfo)
{
	if (IsPermitReceiveData==false) 
	{
		printf("session %d: SendData not permited\n", SessionId);
		return -1; 
	}
	if (segInfo==NULL||dataInfo==NULL||dataInfo->data_buf==NULL)
	{
		logFile->logoutf("segInfo or dataInfo wrong\n");
		return -1;
	}

	TSR_STREAM_FLAG flag;
	//离线
	if (!bOnlineMode)
	{
		logFile->logoutf("[offline model]:income data len is %d\n",dataInfo->dataLen);

		flag = TSR_STREAM_ALL;
		pResultLink = segInfo;
		sprintf(sessWavId,"%d",dataInfo->taskId);

		short* pData = NULL;
		FeSegInfo* seg = segInfo;
		while(seg!=NULL && pData<dataInfo->data_buf+dataInfo->dataLen)
		{
			if (seg->speech_type==RT_VOICE)//有效语音段
			{
				int  decodelen = seg->end_time - seg->start_time+1;
				pData = dataInfo->data_buf + seg->start_time;
				int decoderId = DecoderPool->GetAFreeCell();
				Task* aTask = FillATask(seg,pData,decodelen,decoderId,flag);
				if (aTask == NULL)
				{
					continue;
				}
     			int taskId = myFeaturePools->AddATask(aTask);   // 缓存任务
				//if (taskId < 0)  //待补充
			}	
			seg = seg->next;
		}
		//StopRecording();
	}
	//半在线
	else if(bOnlineMode&&(segInfo->detect_status==FE_SEG_ALL))
	{
		logFile->logoutf("[offline model]:income data len is %d\n",dataInfo->dataLen);

		flag = TSR_STREAM_ALL;
		SegIdxInFile = -2;
		pResultLink = segInfo;
		sprintf(sessWavId,"%d",dataInfo->taskId);
		FeSegInfo* seg = segInfo;
		short* pData = dataInfo->data_buf;
		int decodelen = dataInfo->dataLen;
		Task* aTask = FillATask(seg,pData,decodelen,SegIdxInFile,flag);
		int taskId = myFeaturePools->AddATask(aTask);   // 缓存任务
	}
	//在线
	else if (bOnlineMode)
	{
		FeSegInfo* seg = segInfo;
		switch(seg->detect_status)
		{
		case FE_SEG_START:
			flag = TSR_STREAM_START;
			SegIdxInFile = 0;
			break;
		case FE_SEG_CONT:
			flag = TSR_STREAM_CONT;
			SegIdxInFile++;
			break;
		case FE_SEG_END:
			flag =  TSR_STREAM_END;
			SegIdxInFile = -1;
			pResultLink = seg;
			break;
		}
		
		//缓存语音数据包
		short* pData = dataInfo->data_buf;
		int decodelen = dataInfo->dataLen;
		Task* aTask = FillATask(seg,pData,decodelen,SegIdxInFile,flag);
		int taskId = myFeaturePools->AddATask(aTask);   // 缓存任务
		
	}
	return 0;
}


Task* CTSRBuffer::FillATask(FeSegInfo* segItem,short* pData,int decodelen,int decoderId,TSR_STREAM_FLAG flag)
{
	if (segItem==NULL||pData==NULL||decodelen<0)
	{
		logFile->logoutf("[FillATask]:in param wrong\n");
		return NULL;
	}
	//缓存语音片段数据
	short* temp_decode = new short[decodelen+1];
	memset(temp_decode,0,(decodelen+1)*sizeof(short));
	memcpy(temp_decode,pData,decodelen*sizeof(short));

	//提取特征
	FeatureHeader aheader;
	float *newSet = aCreateFeature->GetFeature(temp_decode, decodelen, &aheader, false, flag);
	SOUND_TYPE myType;
	aCreateFeature->FindSegType(temp_decode, decodelen, &myType);   // myType = MALE;
	delete[] temp_decode,temp_decode= NULL;

	// 组建新task结构
	Task * aTask = (Task*)malloc(sizeof(Task)*1);
	aTask->decoderId = decoderId;
	aTask->newSet = newSet;
	aTask->frameNum = aheader.frameNum;
	aTask->sessionId = SessionId;
	aTask->saveFileIdx = SavedFileIdx;
	aTask->latSet = NULL;
	aTask->soundType = myType;

	if (segItem->detect_status==FE_SEG_ALL||segItem->detect_status==FE_SEG_END)
	{
		SavedFileIdx++;//累计有效语音分段个数
		aTask->dataTimestamp = segItem->start_time;
		aTask->dataTimestampEnd = segItem->end_time;;
	}

	return aTask;
}


int CTSRBuffer::StopRecording()
{
	bEndRecord = true;

	//没有有效语音段送入解码，此处退出引擎防止引擎卡住
	if (!SavedFileIdx )
	{
		if (pResultCallBack[SessionId])   // 当前任务解码完成
		{
			(*pResultCallBack[SessionId])(NULL,true,SessionId);
			ApiLog->logoutf("session %d: all %d tasks TBNR_RECOGNITION_COMPLETE\n", SessionId, SavedFileIdx);
			printf("this file %s is all ring or no seg sent to decoder,so no result\n",sessWavId);
		}
	}
	return LVCSR_SUCCESS;
}