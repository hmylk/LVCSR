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
int SampleRate = 8000;                    //sample rate
FILE **sigle_fp=NULL;
char **filename = NULL;
FILE **TestFile  = NULL;
long *FileSize   = NULL;
int  *SessionId  = NULL;
char **filename_sent= NULL;
bool *bDecodingFinished=NULL;
bool bIsFullWavTest = false;     //send whole wav each time
char opt_ListFile[256] = "";             //file handle of wav list file
FILE *ListFile = NULL;                   //wav list file
CRITICAL_SECTION g_CSReadList;           //protect reading listFile
SPEECHTYPE *type =NULL;                  
SPEECHTYPE opt_WavFormat = TSR_RAW_16;     //wav format

//thread funcion
#ifdef WIN32
VOID InstanceThreadProc(PVOID pvoid);
#else
static void *InstanceThreadProc(void *pvoid);
#endif

void myTBNR_ResultCallback(FeSegInfo * pResultArray,bool bIsOver, int sessionId);
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

#ifdef WIN32
	HANDLE			Threadhandle[NUM_OF_THREAD];
	DWORD			InstThreadID[NUM_OF_THREAD];
#else
	pthread_t Threadhandle [NUM_OF_THREAD];
#endif

	ThreadParam sessionParam[NUM_OF_THREAD];

	SampleRate=LVCSR_SampleRate();
	TestFile  = new FILE*[opt_SessionNum];
	FileSize   = new long[opt_SessionNum];
	SessionId  = new int[opt_SessionNum];
	bDecodingFinished = new bool[opt_SessionNum];
	type = new SPEECHTYPE[opt_SessionNum];
	filename = new char*[opt_SessionNum];
	sigle_fp=new FILE*[opt_SessionNum];
	filename_sent = new char*[opt_SessionNum];

	/*Start all the threads*/
	for (i=0; i<opt_SessionNum; ++i)
		sigle_fp[i]=NULL,TestFile[i] = NULL, FileSize[i] = 0, SessionId[i] = -1, bDecodingFinished[i] = true, filename[i]=NULL, filename_sent[i]=new char[256];

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
	delete []TestFile;
	delete []FileSize;
	delete []SessionId;
	delete []bDecodingFinished;
	delete []type;

	for (int k=0;k<opt_SessionNum;k++)
	{
		delete[] filename[k];
		delete[] filename_sent[k];
	}
	delete []filename;
	delete []filename_sent;
	delete []sigle_fp;

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
	filename[SessNum] =new char[256];

	string filepath;
	while (1)
	{
		char line[256]="";
		EnterCriticalSection(&g_CSReadList);
		char *pRead= fgets(line,256,ListFile);     
		LeaveCriticalSection(&g_CSReadList);

		if (NULL==pRead)
		{
			break;
		}
		chomp(line);
		if (!line[0]) continue;

		if (strchr(line,'\\'))
		{
			filepath = line;
		}

		printf("processing %s\n",filepath.c_str());

		FILE* segList = fopen(filepath.c_str(),"rt");
		if (segList==NULL)
		{
			return;
		}
		FeSegInfo* segInfo =NULL;
		FeSegInfo* pCur = NULL;
		while (1)
		{
			char line[256]="";
			char *pRead= fgets(line,256,segList);     
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


			if (segInfo==NULL)
			{
				segInfo = new FeSegInfo;
				segInfo->start_time =atoi(start.c_str()) ;
				segInfo->end_time = atoi(end.c_str());
				segInfo->speech_type = RT_VOICE;
				segInfo->detect_status = FE_SEG_ALL;
				segInfo->next =NULL;
				segInfo->pText = NULL;
				pCur = segInfo;
			}
			else
			{
				FeSegInfo* newItem = new FeSegInfo;
				newItem->start_time = atoi(start.c_str()) ;
				newItem->end_time = atoi(end.c_str());
				newItem->speech_type = RT_VOICE;
				newItem->detect_status = FE_SEG_ALL;
				newItem->next = NULL;
				newItem->pText = NULL;
				pCur->next = newItem;
				pCur = newItem;
			}
		}
		fclose(segList),segList= NULL;

		int pos = filepath.find_last_of("_");
		string wavFileName= filepath.substr(0,pos);
		FileSize[SessNum] = OpenFile(TestFile[SessNum], (char*)(wavFileName.c_str()), &type[SessNum]);
		if(FileSize[SessNum]<=0)    
		{
			printf("session %d:%s is a bad file,drop it\n",SessNum,filepath.c_str());
			if (TestFile[SessNum]) fclose(TestFile[SessNum]), TestFile[SessNum] = NULL;
			continue;
		}	

		memset(filename[SessNum],0,256);
		strcpy(filename[SessNum],wavFileName.c_str());

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
				printf("session %d: OPEN _SENT File %s ERROR\n", SessNum, filename_sent[SessNum]);
				if (TestFile[SessNum]) fclose(TestFile[SessNum]), TestFile[SessNum] = NULL;
				continue;
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
			if (LVCSR_SetResultCallbackFunc(myTBNR_ResultCallback, SessionId[SessNum]) < 0)
			{ printf("Set TBNR ResultCallback fail\n"); 	}
		}

		if (bIsFullWavTest)//针对人工分段好的语音，整包送入语音文件，引擎不做vad
		{
			short *tmpData = new short[FileSize[SessNum]/2];
			memset(tmpData,0,FileSize[SessNum]);
			int tempRead = fread(tmpData, 2, FileSize[SessNum]/2, TestFile[SessNum]);
			if (tempRead<=0)
			{
				delete[] tmpData,tmpData=NULL;
			}
			FeSpeechAttri  dataAttr;
			dataAttr.data_buf = tmpData;
			dataAttr.dataLen = FileSize[SessNum]/2;
			dataAttr.send_status = FE_SEND_ALL;

			LVCSR_SendData(segInfo,&dataAttr,SessNum);	
			bDecodingFinished[SessNum] = false;
			delete[] tmpData,tmpData=NULL; 
			fclose(TestFile[SessNum]),TestFile[SessNum]=NULL;
		}

		LVCSR_StopRecording(SessNum);
	
		while(1)
		{
			//if decode finished, then print recognize result to tbnr_Result.txt
			if (TestFile[SessNum] == NULL&& bDecodingFinished[SessNum]&&SessionId[SessNum] >= 0)
			{
				LVCSR_Stop(SessionId[SessNum]);

				SessionId[SessNum] = -1;
				if(sigle_fp[SessNum])
				{
					fclose(sigle_fp[SessNum]);
					sigle_fp[SessNum]=NULL;
				}
				FeSegInfo* pCurSegH = segInfo;
				while(pCurSegH!=NULL)
				{
					FeSegInfo* pTemp = pCurSegH->next;
					delete pCurSegH;
					pCurSegH = pTemp;
				}
				break;
			}
			else
			{
				Sleep(8);
				continue;
			}
		}

	}
	printf("no file to handle\n");
#ifdef  WIN32
	return ;
#else
	return NULL;
#endif
}

//recognize result callback function
void myTBNR_ResultCallback(FeSegInfo * pResultArray,bool bIsOver, int sessionId)
{
	FeSegInfo* ptemp = pResultArray;
	while(ptemp!=NULL)
	{
		if (ptemp->pText==NULL)
		{
			ptemp=ptemp->next;
			continue;
		}
		string text = ptemp->pText;
		int pos= text.find_first_of("|");
		string recstr = text.substr(0,pos);
		string segtime = text.substr(pos+1);
		fprintf(sigle_fp[sessionId], "%s\n",recstr.c_str());
		fprintf(sigle_fp[sessionId],"%s\n",segtime.c_str());
		fprintf(fp,"%s",recstr.c_str());
		printf("%s\n\n",ptemp->pText);
		ptemp=ptemp->next;
	}
	fprintf(fp,"( %s )\n",filename[sessionId]);
	
	if (bIsOver)
	{
		bDecodingFinished[sessionId] = true;
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

