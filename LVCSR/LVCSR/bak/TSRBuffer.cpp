

#include "TSRBuffer.h"
#include <assert.h>

extern CycLine  *DecoderPool;
extern LateralLink *FeaturePools;
extern bool bOnlineMode;

CTSRBuffer::CTSRBuffer(char* sysDir, char* configFile, LogFile* ctLogFile, int sessionId)
{
	SampleRate = LVCSR_SampleRate();
	SessionId = sessionId;
	//logFile = ctLogFile;
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
	if (segInfo==NULL||dataInfo==NULL)
	{
		return -1;
	}

	TSR_STREAM_FLAG flag;
	//离线
	if (!bOnlineMode&&dataInfo->send_status==FE_SEND_ALL)
	{
		flag = TSR_STREAM_ALL;
		pResultLink = segInfo;
		sprintf(sessWavId,"%d",dataInfo->taskId);

		short* pData = dataInfo->data_buf;
		FeSegInfo* seg = segInfo;
		while(seg!=NULL)
		{
			if (seg->speech_type==RT_VOICE)
			{
				//缓存语音片段数据
				int  decodelen = seg->end_time - seg->start_time+1;
				short* temp_decode = new short[decodelen+1];
				memset(temp_decode,0,(decodelen+1)*sizeof(short));
				memcpy(temp_decode,pData+seg->start_time,decodelen*sizeof(short));

				//提取特征
				FeatureHeader aheader;
				float *newSet = aCreateFeature->GetFeature(temp_decode, decodelen, &aheader, false, flag);
				SOUND_TYPE myType;
				aCreateFeature->FindSegType(temp_decode, decodelen, &myType);   // myType = MALE;
				delete[] temp_decode,temp_decode= NULL;

				// 组建新task结构
				int decoderId = DecoderPool->GetAFreeCell();
				Task * aTask = (Task*)malloc(sizeof(Task)*1);
				aTask->decoderId = decoderId;
				aTask->newSet = newSet;
				aTask->frameNum = aheader.frameNum;
				aTask->sessionId = SessionId;
				aTask->saveFileIdx = SavedFileIdx;
				aTask->latSet = NULL;
				aTask->soundType = myType;
				aTask->dataTimestamp = seg->start_time;
				aTask->dataTimestampEnd = seg->end_time;
				SavedFileIdx++;

				int taskId = myFeaturePools->AddATask(aTask);   // 缓存任务
				//if (taskId < 0)  //待补充
				
				seg = seg->next;
			}	
		}
		StopRecording();
	}
	//半在线
	else if(bOnlineMode&&segInfo->detect_status==FE_SEG_ALL)
	{
		flag = TSR_STREAM_ALL;
		SegIdxInFile = -2;
		pResultLink = segInfo;
		sprintf(sessWavId,"%d",dataInfo->taskId);

		//缓存片段数据
		short* pData = dataInfo->data_buf;
		int decodelen = dataInfo->dataLen;
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
		int decoderId = DecoderPool->GetAFreeCell();
		Task * aTask = (Task*)malloc(sizeof(Task)*1);
		aTask->decoderId = SegIdxInFile;
		aTask->newSet = newSet;
		aTask->frameNum = aheader.frameNum;
		aTask->sessionId = SessionId;
		aTask->saveFileIdx = SavedFileIdx;
		aTask->latSet = NULL;
		aTask->soundType = myType;
		aTask->dataTimestamp = segInfo->start_time;
		aTask->dataTimestampEnd = segInfo->end_time;

		SavedFileIdx++;

		int taskId = myFeaturePools->AddATask(aTask);   // 缓存任务

	}
	//在线
	else if (bOnlineMode)
	{
		int segStart=0,segEnd=0;
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
			pResultLink = segInfo;
			segStart =segInfo->start_time;
			segEnd = segInfo->end_time;
			break;
		}
		
		//缓存语音数据包
		short* pData = dataInfo->data_buf;
		int decodelen = dataInfo->dataLen;
		short* temp_decode = new short[decodelen+1];
		memset(temp_decode,0,(decodelen+1)*sizeof(short));
		memcpy(temp_decode,pData,decodelen*sizeof(short));

		//提取这一包的特征
		FeatureHeader aheader;
		float *newSet = aCreateFeature->GetFeature(temp_decode, decodelen, &aheader, false, flag);
		SOUND_TYPE myType;
		aCreateFeature->FindSegType(temp_decode, decodelen, &myType);   // myType = MALE;
		delete[] temp_decode,temp_decode= NULL;

		// 组建新task结构
		int decoderId = DecoderPool->GetAFreeCell();  
		Task * aTask = (Task*)malloc(sizeof(Task)*1);
		aTask->decoderId = SegIdxInFile;
		aTask->newSet = newSet;
		aTask->frameNum = aheader.frameNum;
		aTask->sessionId = SessionId;
		aTask->saveFileIdx = SavedFileIdx;
		aTask->latSet = NULL;
		aTask->soundType = myType;
		aTask->dataTimestamp = segStart;
		aTask->dataTimestampEnd = segEnd;
		
		if (segInfo->detect_status==FE_SEG_END)
		{
			SavedFileIdx++;
		}
		int taskId = myFeaturePools->AddATask(aTask);   // 缓存任务
		
	}
	return 0;
}



int CTSRBuffer::StopRecording()
{
	bEndRecord = true;
	return LVCSR_SUCCESS;
}