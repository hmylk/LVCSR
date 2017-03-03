/*++
                         ThinkIT	LVCSR API  Demo
---------------------------------------------------------------------------------
Name:       TBNR_test_stream.cpp

Function:   A simple demo of LVCSR working in the OFFLINE mode
            for multi-instances

Author:     HM

Time  :     2016/08/03

Revision :
Copyright:  ThinkIT Speech Lab

---------------------------------------------------------------------------------

--*/

#include "ComDefinition.h"

#ifndef WIN32
#include "porting.h"
#endif

FILE *fp = fopen ("tbnr_Result.txt","wt");   
int   opt_SecondPerSend = 100;      //millisecond data per send 
int SampleRate = 8000;                    //sample rate
FILE **sigle_fp=NULL;
char **filename = NULL;
char **DataBuffer =NULL;
FILE **TestFile  = NULL;
long *FileSize   = NULL;
long *ReadBytes  = NULL;
int  *SessionId  = NULL;
bool *Sessionread =NULL;
short *FrameSize=NULL;
char **filename_sent= NULL;
bool *bDecodingFinished=NULL;
bool bIsFullWavTest = false;     //send whole wav each time
bool bIsDnnVadTest = false;      //send 600s wav each time
char opt_ListFile[256] = "";             //file handle of wav list file
FILE *ListFile = NULL;                   //wav list file
SortedResult** pSortedResList=NULL;      //used for keeping recognize result
CRITICAL_SECTION g_CSReadList;           //protect reading listFile
CRITICAL_SECTION g_CSWriteSentenceRes;   //protect writing tbnr_Result.txt
SPEECHTYPE *type =NULL;                  
SPEECHTYPE opt_WavFormat = TSR_RAW_16;     //wav format

//thread funcion
#ifdef WIN32
VOID InstanceThreadProc(PVOID pvoid);
#else
static void *InstanceThreadProc(void *pvoid);
#endif

void printResult(int sessId);               //write recognize result to tbnr_Result.txt
void myTBNR_ProcessEventCallback(int eventID, int sessionId);                              //event callback function
void myTBNR_ResultCallback( FeSegInfo *pResultArray, int sessionId);    //recognize result callback function
long OpenFile(FILE*(&aFile), char *fileName, SPEECHTYPE *type = NULL, char *mode = "rb");  //open wav file, return true voice data length


int main(int argc, char * argv[])
{
	char opt_ConfigFile[256] = "";   
	char opt_AbsolutePath[256] = "";   
	int  opt_SessionNum = 2;

	if (argc <3)
	{
		printf("Error: Please check parameters!\n");
		exit(-1);
	}

	/*Parse arguments*/
	int i=1;
	for (i=1; i<argc; i++)
	{
		if (!strcmp(argv[i], "-config") )
		{
			strcpy(opt_ConfigFile, argv[++i]);
		}
		else if (!strcmp(argv[i], "-FULLWAV"))
		{
			bIsFullWavTest=true;
		}
		else if (!strcmp(argv[i], "-DNNVAD"))
		{
			opt_SecondPerSend=600;
			bIsDnnVadTest = true;
		}
		else if (!strcmp(argv[i], "-sysDir"))
		{
			strcpy(opt_AbsolutePath, argv[++i]);
		}	
		else if (!strcmp(argv[i], "-audiolist"))
		{
			strcpy(opt_ListFile, argv[++i]);
		}	
		else if (!strcmp(argv[i], "-num"))
		{
			opt_SessionNum = atoi(argv[++i]);
		}
		else if (!strcmp(argv[i], "-format"))
		{
			if (ParseFormat(argv[++i],opt_WavFormat)<0)
			{
				printf("Invalid audio format %s\n", argv[i]);
				exit(-1);
			}
		}
		else printf("Unknown option:[%s]\n", argv[i]);
	}
	fflush(fp);
	 /*Initialize the environment*/
	if (LVCSR_Init(opt_AbsolutePath, opt_ConfigFile, opt_SessionNum) < 0)   
	{
		printf("Set TBNR Init fail, may be wrong license.txt in your directory !\n");
		return -1;
	}

	InitializeCriticalSection(&g_CSReadList);
	InitializeCriticalSection(&g_CSWriteSentenceRes);

#ifdef WIN32
	HANDLE			Threadhandle[NUM_OF_THREAD];
	DWORD			InstThreadID[NUM_OF_THREAD];
#else
	pthread_t Threadhandle [NUM_OF_THREAD];
#endif

	ThreadParam sessionParam[NUM_OF_THREAD];

	SampleRate=LVCSR_SampleRate();
	DataBuffer = new char*[opt_SessionNum];
	TestFile  = new FILE*[opt_SessionNum];
	FileSize   = new long[opt_SessionNum];
	ReadBytes  = new long[opt_SessionNum];
	SessionId  = new int[opt_SessionNum];
	Sessionread =new bool[opt_SessionNum];
	bDecodingFinished = new bool[opt_SessionNum];
	type = new SPEECHTYPE[opt_SessionNum];
	FrameSize = new short[opt_SessionNum];
	filename = new char*[opt_SessionNum];
	pSortedResList = new SortedResult*[opt_SessionNum];
	sigle_fp=new FILE*[opt_SessionNum];
	filename_sent = new char*[opt_SessionNum];

	/*Start all the threads*/
	for (i=0; i<opt_SessionNum; ++i)
		pSortedResList[i]=NULL,sigle_fp[i]=NULL,DataBuffer[i]=NULL,Sessionread[i]=false,TestFile[i] = NULL, FileSize[i] = 0, ReadBytes[i] = 0, SessionId[i] = -1, bDecodingFinished[i] = true, filename[i]=NULL, filename_sent[i]=new char[256];

	if (opt_ListFile[0])
	{
		printf("processing listfile: %s\n", opt_ListFile);
		OpenFile(ListFile, opt_ListFile);  
	}

	/*Start all the threads*/
	for(int i = 0 ;i < opt_SessionNum; i++)
	{
		sessionParam[i].sessId = i;
#ifdef WIN32
		Threadhandle[i]			 = CreateThread(NULL,
			0,
			(LPTHREAD_START_ROUTINE)InstanceThreadProc, 
			sessionParam+i,
			0,
			&(InstThreadID[i]));
#else
		//No need to set the attribute explicitly
		//The default stack size is 2M !
		pthread_create(&(Threadhandle[i]), NULL, InstanceThreadProc, sessionParam+i);
#endif
		Sleep(100);
		printf("TKW start thread %d \n",i);
	}
#ifdef WIN32
	WaitForMultipleObjects(opt_SessionNum,Threadhandle,TRUE,INFINITE);
#else
	for(i=0;i<opt_SessionNum;i++)
	{
		printf("thread %d - ok\n",i);
		pthread_join(Threadhandle[i], NULL);
	}
#endif
	
	if (ListFile)
	{
		fclose(ListFile);
	}

	DeleteCriticalSection(&g_CSReadList);
	DeleteCriticalSection(&g_CSWriteSentenceRes);
	delete []DataBuffer;
	delete []TestFile;
	delete []FileSize;
	delete []ReadBytes;
	delete []SessionId;
	delete []bDecodingFinished;
	delete []type;
	delete []FrameSize;

	for (int k=0;k<opt_SessionNum;k++)
	{
		delete[] filename[k];
		delete[] filename_sent[k];
	}
	delete []filename;
	delete []filename_sent;
	delete []sigle_fp;
	delete []Sessionread;
	delete []pSortedResList;

	fflush(fp);
	/*exit all the environments*/
	LVCSR_Exit();
	fclose(fp);
	return 0;
}


/*
 *Thread process function
 */
#ifdef WIN32
VOID InstanceThreadProc(PVOID pvoid)
#else
static void *InstanceThreadProc(void *pvoid)
#endif
{
	ThreadParam * param = (ThreadParam*)pvoid;
	int SessNum = param->sessId;
	DataBuffer[SessNum] = new char[opt_SecondPerSend*SampleRate*sizeof(short)];
	filename[SessNum] =new char[256];

	
	
	string finepath;
	FeSegInfo* segInfo =NULL;
	char line[256]="";
	char *pRead= fgets(line,256,ListFile);     
	if (NULL==pRead)
	{
		return;
	}
	chomp(line);
	if (!line[0]) return;

	if (strchr(line,'\\'))
	{
		finepath = line;
	}

	FileSize[SessNum] = OpenFile(TestFile[SessNum], (char*)(finepath.c_str()), &type[SessNum]);
	if(FileSize[SessNum]==0)    //add for straight wav
	{
		printf("session %d:%s is a stream file,drop it\n",SessNum,finepath);
		if (TestFile[SessNum]) fclose(TestFile[SessNum]), TestFile[SessNum] = NULL;
	}

	memset(filename[SessNum],0,256);
	strcpy(filename[SessNum],finepath.c_str());
	/*Open sent file in the same directory of wav*/
	char temp_filename[256];
	if(opt_ListFile[0])
	{
		strcpy(temp_filename,filename[SessNum]);
		strtok(temp_filename,".");
		strcpy(filename_sent[SessNum],temp_filename);
		strcat(filename_sent[SessNum],"_sent.txt");
		sigle_fp[SessNum]=fopen(filename_sent[SessNum],"wt");
		if(sigle_fp[SessNum]==NULL)
		{
			//printf("session %d: OPEN _SENT File %s ERROR\n", SessNum, filename_sent[SessNum]);
		}
	}
	if (SessionId[SessNum] < 0)
	{
		SessionId[SessNum] = LVCSR_Start(SessNum);              //Create a session
		printf("process file %s\n",filename[SessNum]);
		if (SessionId[SessNum] < 0)
		{
			printf("session %d Start TBNR failed!\n", SessNum);
#ifdef  WIN32
			return ;
#else
			return NULL;
#endif
		}

		// Set callback function
		if (LVCSR_SetEventCallbackFunc(myTBNR_ProcessEventCallback, SessionId[SessNum]) < 0)
		{ printf("Set TBNR EventCallback fail\n");  }
		if (LVCSR_SetResultCallbackFunc(myTBNR_ResultCallback, SessionId[SessNum]) < 0)
		{ printf("Set TBNR ResultCallback fail\n"); 	}
	}

	short *tmpData = new short[FileSize[SessNum]/2];
	memset(tmpData,0,FileSize[SessNum]);
	int tempRead = fread(tmpData, 2, FileSize[SessNum]/2, TestFile[SessNum]);
	if (tempRead<=0)
	{
		delete[] tmpData,tmpData=NULL;
	}
	while (1)
	{
		char line[256]="";
		char *pRead= fgets(line,256,ListFile);     
		if (NULL==pRead)
		{
			break;
		}
		chomp(line);
		if (!line[0]) continue;

		string strline = line;
		int pos1 = strline.find_first_of(" ");
		string start = strline.substr(0,pos1);
		string end = strline.substr(pos1+2);

		int istart = atoi(start.c_str());
		int iend = atoi(end.c_str());
		int segLen = iend - istart;
		short* databuf = new short[1024];
		ReadBytes = 0;
		int total=0;
		int step=1024;
		int count =0 ;
		short* pAllDataBuf = tmpData;
		pAllDataBuf=pAllDataBuf+istart;
		while(total<segLen)
		{
			if (total+step>segLen)
			{
				step = segLen - total;
				
			}
			memset(databuf,0,1024);
			memcpy(databuf,pAllDataBuf,step*2);
			FeSpeechAttri  dataAttr;
			dataAttr.data_buf = databuf;
			dataAttr.dataLen = step;

			FeSegInfo* segInfo = new FeSegInfo;
			segInfo->start_time =istart ;
			segInfo->end_time =iend;
			segInfo->speech_type = RT_VOICE;
			segInfo->next =NULL;

			if (count==0)
			{
				segInfo->detect_status = FE_SEG_START;
			}
			else if (total+step>=segLen)
			{
				segInfo->detect_status = FE_SEG_END;
			}
			else
			{
				segInfo->detect_status = FE_SEG_CONT;
			}

			LVCSR_SendData(segInfo,&dataAttr,SessNum);	

			total+=step;
			count++;
			pAllDataBuf = pAllDataBuf+step;
			bDecodingFinished[SessNum] = false;
		}
		delete[] databuf,databuf = NULL;
	}
	delete[] tmpData,tmpData = NULL;
	fclose(TestFile[SessNum]),TestFile[SessNum]=NULL;
	LVCSR_StopRecording(SessNum);
	while(1)
	{
		//if decode finished, then print recognize result to tbnr_Result.txt
		if (TestFile[SessNum] == NULL&& bDecodingFinished[SessNum]&&SessionId[SessNum] >= 0)
		{
			LVCSR_Stop(SessionId[SessNum]);                

			EnterCriticalSection(&g_CSWriteSentenceRes);
			printResult(SessionId[SessNum]);
			LeaveCriticalSection(&g_CSWriteSentenceRes);

			SortedResult *head = pSortedResList[SessNum];
			SortedResult* temp=head;
			while(temp!=NULL)
			{
				SortedResult* p = temp;
				delete[] temp->segResult;
				temp = temp->sortedRstNext;
				delete p;
			}
			head= NULL;

			SessionId[SessNum] = -1;
			Sessionread[SessNum]=false;
			if(sigle_fp[SessNum])
			{
				fclose(sigle_fp[SessNum]);
				sigle_fp[SessNum]=NULL;
			}
			break;
		}
		else
		{
			Sleep(8);
			continue;
		}
	}
	delete[] DataBuffer[SessNum];
#ifdef  WIN32
	return ;
#else
	return NULL;
#endif
}

//write recognize result to tbnr_Result.txt
void printResult(int sessId)
{
	if (sessId<0)
	{
		return;
	}
	SortedResult* temp=pSortedResList[sessId];
	if (temp==NULL)
	{
		return;
	}
	int i=0;
	while(temp!=NULL)
	{
		i++;
		fprintf(fp,"%s\t",temp->segResult);
		fflush(fp);
		temp=temp->sortedRstNext;
	}
	fprintf(fp,"( %s )\n",filename[sessId]);
	fflush(fp);
	printf("\n");

}

//recognize result callback function
void myTBNR_ResultCallback(FeSegInfo *pResultArray, int sessionId)
{
	FeSegInfo* ptemp = pResultArray;
	while(ptemp!=NULL)
	{
		string text = ptemp->pText;
		int pos= text.find_first_of("|");
		string recstr = text.substr(0,pos);
		string segtime = text.substr(pos+1);
		fprintf(sigle_fp[sessionId], "%s\n",recstr.c_str());
		fprintf(sigle_fp[sessionId],"%s\n",segtime.c_str());
		printf("%s\n\n",ptemp->pText);
		ptemp=ptemp->next;
	}
}


//event callback function
void myTBNR_ProcessEventCallback(int eventID, int sessionId)
{
	switch (eventID)
	{
	case LVCSR_EVENT_START_COMPLETE:
		break;
	case LVCSR_EVENT_STOP_COMPLETE:
		bDecodingFinished[sessionId] = true;
		break;
	case LVCSR_EVENT_RECOGNITION_COMPLETE:
		bDecodingFinished[sessionId] = true;
		break;
	case LVCSR_EVENT_RECOGNITION_DOING:
		break;
	default:
		printf("unknown event,myTBNR_ProcessEventCallback()\n");
		break;
	}
}

//open wav file, return true voice data length
long OpenFile(FILE*(&aFile), char *fileName, SPEECHTYPE *type, char *mode )
{
	long FileSize = 0;
	while (fileName[strlen(fileName)-1] == '\n' || fileName[strlen(fileName)-1] == '\r')
		fileName[strlen(fileName)-1] = '\0';
	if (fileName && fileName[0])
		aFile = fopen(fileName, mode);
	else aFile = NULL;
	if (aFile==NULL)
	{
		printf("Can not open file: %s !\n", fileName);
		return -1;   // exit(-1);
	}
	long NSkip = 0;
	char head[5] = {'\0'};
	fread(head, sizeof(char), 4, aFile);
	if (!strcmp(head, "RIFF") && type)
	{
		NSkip = 44;
		fseek(aFile, 20, SEEK_SET);
		fread(head, sizeof(char), 2, aFile);
		/*	wav文件的第14h个字节表示wav的格式:
			11h: imapcm
			06h: a-law
			07h: mu-law
			01h: pcm */
		switch(head[0])
		{
		case '\01': *type=TSR_LINEAR_PCM; break;
		case '\06': *type=TSR_ALAW_PCM; break;
		case '\07': *type=TSR_ULAW_PCM; break;
		case '\11': *type=TSR_ADPCM; break;
		default: printf("Format Category %d! not supported !", head[0]);
		}
		fread(head, sizeof(char), 2, aFile);   // channel
		if (head[0] != '\1') printf("stereo channel! not supported !");
		fread(head, sizeof(char), 4, aFile);   // samplerates
		switch(*(int*)head)
		{
		case 8000: break;
		case 16000: *type = SPEECHTYPE(*type | 8); break; // 070806 thanks jgao for type convert
		default: printf("samplerate of %d! not supported !", *(int*)head);
		}
		// SampleRate = *(int*)head;
		fseek(aFile, 34, SEEK_SET);
		fread(head, sizeof(char), 2, aFile);   // NBitsPersample
		switch(*(short*)head)
		{
		case 8: break;   // 8bit
		case 16: *type = SPEECHTYPE(*type | 4); break;
		default: printf("NBitsPersample of %d! not supported !", *(short*)head);
		}

		fseek(aFile,0,SEEK_SET);

		THINKIT::READ_FILE myFile(aFile);
        unsigned int headsize=myFile.GetHeadSize();
		if (headsize!=NSkip)
		{
			NSkip = headsize;
		}
	}
	else if (type)  //没有头的特征文件或者是语音文件
	{
		*type = TSR_LOST;
	}
	else  //一般的txt文件（通常为wavlist文件），非语音文件或特征文件
	{
	}
	fseek(aFile, 0, SEEK_END);
	FileSize = ftell(aFile) - NSkip;
 	fseek(aFile, NSkip, SEEK_SET);
    printf("file %s headsize is %d\n",fileName,NSkip);
	return FileSize;
}

