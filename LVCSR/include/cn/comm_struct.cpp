#include "comm_struct.h"
#include "LineSplitter.h"
#include "comm.h"
//#include "ClusterFile.h"
//ClusterFile WordMap("../lexicon/cluster_result.txt");


//////////////////////////////////////////////////////////////////////////
//	HypoElement
//////////////////////////////////////////////////////////////////////////
HypoElement::HypoElement()
{
	triphoneNum  = 0;
	triphoneInfo = NULL;
	alscore		 = UNDEF_LN;
#ifdef USE_ENG //  [2/22/2011 administrator]
	wordHead = 0;
	wordTail = 0;
	maxScore = 0;
#endif
	/*for (int i=0;i<4;i++)
	{
		tone[i]=false;
	}*/
}
HypoElement::HypoElement(const string& line, Vocab2& vocab,bool bRemoveTone)
{
	LineSplitter fields;
	fields.Split(line);
	triphoneNum  = 0;
	triphoneInfo = NULL;
#ifndef REMOVE_IDX_UNUSED
	alscore		 = UNDEF_LN;
#endif

#ifdef USE_ENG //  [2/22/2011 administrator]
	wordHead = 0;
	wordTail = 0;
	maxScore = 0;
#endif
	unsigned int no_fields = fields.NoWords();

	/*for (int m=0;m<4;m++)
	{
		tone[m]=false;
	}*/

	
	int i=0;
	for(i=0; i < no_fields; i++)
	{
		int pos = fields[i].find("=");
		if (pos == string::npos)
		{
			printf("pos %d endpos %d %s   !!!\n",pos,string::npos,line.c_str());
		}
		assert(pos != string::npos);
		const string& idf = fields[i].substr(0,pos); 
		const string& entry = fields[i].substr(pos+1,string::npos); 

		FillHypoFields(idf, entry, vocab,bRemoveTone);
	}	
	
}


HypoElement::HypoElement(const HypoElement&rhs)
{
	
	if (&rhs == this)
	{
		return ;
	}
	wordID	= rhs.wordID;		//word id
	score	= rhs.score;		//score of the hypothesis
	tmStart	= rhs.tmStart;		//start time
	tmEnd	= rhs.tmEnd;			//end time
// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
	wordHead = rhs.wordHead;
	wordTail = rhs.wordTail;
	maxScore = rhs.maxScore;
#endif

	/*for (int i=0;i<4;i++)
	{
		tone[i]=rhs.tone[i];
	}*/
	
	//Extended information
	alscore		= rhs.alscore;			//acoustic score
	triphoneNum	= rhs.triphoneNum;
	triphoneInfo= NULL;
	if(triphoneNum > 0)
	{
		if(triphoneInfo != NULL)
		{
			free(triphoneInfo);
		}
		triphoneInfo = (TriphoneInWord *)malloc(sizeof(TriphoneInWord)*triphoneNum);
		assert(triphoneInfo != NULL);
		memcpy(triphoneInfo,rhs.triphoneInfo,sizeof(TriphoneInWord)*triphoneNum);
	}
}

HypoElement& HypoElement::operator =(const HypoElement &rhs)
{
	wordID	= rhs.wordID;		//word id
	score	= rhs.score;		//score of the hypothesis
	tmStart	= rhs.tmStart;		//start time
	tmEnd	= rhs.tmEnd;			//end time

	// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
	wordHead = rhs.wordHead;
	wordTail = rhs.wordTail;
	maxScore = rhs.maxScore;
#endif

	/*for (int i=0;i<4;i++)
	{
		tone[i]=rhs.tone[i];
	}*/
	
	//Extended information
	alscore		= rhs.alscore;			//acoustic score
	triphoneNum	= rhs.triphoneNum;
	if(triphoneNum > 0)
	{
		if(triphoneInfo != NULL)
		{
			free(triphoneInfo);
			triphoneInfo=NULL;
		}
		triphoneInfo = (TriphoneInWord *)malloc(sizeof(TriphoneInWord)*triphoneNum);
		assert(triphoneInfo != NULL);
		memcpy(triphoneInfo,rhs.triphoneInfo,sizeof(TriphoneInWord)*triphoneNum);
	}
	return *this;
}

HypoElement::~HypoElement()
{
	if (triphoneInfo != NULL)
	{
		free(triphoneInfo);
		triphoneInfo = NULL;
	}
}



void HypoElement::FillHypoFields(const string& idf, const string& entry, Vocab2& vocab,bool bRemoveTone)//  [3/8/2011 zhzhang]
{
	TriphoneInWord tri_Buf[MAX_TRIPHONE_NUM];
	if (idf == "H")
	{	
	}
	else if(idf == "Word")
	{
//1-24-2010, JGao
//#ifndef NOTUNE 
	/*	if (!bRemoveTone)
		{
			wordID = vocab.GetIdx(entry);
		}*/
//#else
//		else
		if (!bRemoveTone)
		{
			char tonename='\0';
			string str = SkipTune(entry,tonename);//  [3/8/2011 zhzhang]
			if (tonename>='1' && tonename<='5')
			{
				int tmp=tonename-48;
				//tone[tmp-1]=true;//变为bool变量  [3/8/2011 zhzhang]
			}
			
			wordID = vocab.GetIdx(str);
		}
//#endif
	}
	
	else if(idf == "Score")
	{
		//use logSocre
		double prob = (double)atof(entry.c_str());
		score = (float)prob;
	}
	else if(idf == "ALScore")
	{
		//use logSocre
		double prob = (double)atof(entry.c_str());
		alscore = (float)prob;
	}
	else if(idf == "ST")
	{
		tmStart = (Time)atof(entry.c_str());
	}
	else if(idf == "ET")
	{
		tmEnd = (Time)atof(entry.c_str());
	}
	//3-2-2010, JGao,读取triphone 信息
	else if(idf == "DIV")
	{
		//7-27-2009, JGao for Uyghur, temporliry disabled
		//return;
		
#ifdef USE_PHONE_ENCODING
		//如果没有monophone，则无法进行编码
		if (mono==NULL)
		{
			return;
		}
#endif
		char seps[]   = ":";
		char *token;
		int phoneCount=0;
		char strTmp[2048]="";
		Time curStart;
		curStart = tmStart;
		token = strtok( (char*)entry.c_str(), seps );
		if (token==NULL)
		{
			return;
		}
		
		while( token != NULL )
		{
			//printf("%s\t",token);
			// char leftPn[256],middlePn[256],rightPn[256];
			
			char *sp;
			Time phoneDur;
			sp = strchr(token, ',');
			if (sp == NULL)
			{
				printf("[Warning]:Invalid triphone information %s\n",token);
				//break;
				return;
			}
			strcpy(strTmp,sp+1);
			*sp=0;
			phoneDur= (Time)atof(strTmp);
			//sprintf(token,"%s-%s+%s",leftPn,middlePn,rightPn);
			//从triphone转换为phone idx
			//去掉sil和Sp和garbage ,解析下一个triphone
			if(!strcmp(token,"sil")||!strcmp(token,"sp")||strstr(token,"-.garbage"))
			{
				token = strtok( NULL, seps );
				continue;
			}
#ifdef USE_PHONE_ENCODING
			
			phnenc triphn=0;
			tri_Buf[phoneCount].triphnIdx;
			uchar lp=0;
			uchar cp=0;
			uchar rp=0;
			PhoneEncoding::Index(mono,token,&lp,&cp,&rp);
			if (lp==0 ||rp==0||cp==0)
			{
				fprintf(stderr,"wrong triphone for %s\n",token);
			}
			ASSERT2(lp !=0,"lp wrong");
			ASSERT2(cp !=0,"cp wrong");
			ASSERT2(rp !=0,"rp wrong");
			triphn=PhoneEncoding::Index(lp,cp,rp);
			
			//测试用
			char triphonName[32]="";
			PhoneEncoding::Name(triphonName,mono,triphn);
			ASSERT2(!strcmp(triphonName,token),"triphone encoding != decoding");
			tri_Buf[phoneCount].triphnIdx=triphn;
#else
			strcpy(tri_Buf[phoneCount].triphoneName,token);
			
#endif
			tri_Buf[phoneCount].phoneStart = curStart;
			tri_Buf[phoneCount].phoneEnd = curStart + phoneDur;
			
			curStart += phoneDur;
			phoneCount++;
			token = strtok( NULL, seps );
		}
		triphoneNum  = phoneCount;
		assert(triphoneNum<MAX_TRIPHONE_NUM);
		if (triphoneNum > 0)
		{
			triphoneInfo = (TriphoneInWord* )malloc(sizeof(TriphoneInWord)*triphoneNum);
			assert(triphoneInfo != NULL);
			memcpy(triphoneInfo,tri_Buf,sizeof(TriphoneInWord)*triphoneNum);
		}	
		
		//For sp time //  [3/20/2008]
#ifdef USE_PHONE_ENCODING
		
#else
		int i = triphoneNum-1;
		while (i>=0 &&!strcmp("sp",triphoneInfo[i].triphoneName))
		{
			
			tmEnd -=(triphoneInfo[i].phoneEnd-triphoneInfo[i].phoneStart);
			i--;
		}
#endif
	}
	else
	{
		
	}
}
void HypoElement::ReadHypo(FILE*fp,bool bReadExtend)
{
	assert(fp != NULL);
    //Read the basic information
	fread(&wordID,1,sizeof(WordID),fp);
	fread(&score,1,sizeof(float),fp);
	fread(&tmStart,1,sizeof(Time),fp);
	fread(&tmEnd,1,sizeof(Time),fp);
	
	//Read the extended information
	if (bReadExtend)
	{
		//Extended information
		fread(&triphoneNum,1,sizeof(int),fp);
		if (triphoneNum != UNDEF_NUM && triphoneNum>0)
		{
			triphoneInfo = (TriphoneInWord* )malloc(sizeof(TriphoneInWord)*triphoneNum);
			assert(triphoneInfo != NULL);
			fread(triphoneInfo,1,sizeof(TriphoneInWord)*triphoneNum,fp);
		}
		fread(&alscore,1,sizeof(Time),fp);
	}
}


void HypoElement::WriteHypo(FILE * fp, bool bWriteExtend)
{
	assert(fp != NULL);
    //Read the basic information
	fwrite(&wordID,1,sizeof(WordID),fp);
	fwrite(&score,1,sizeof(float),fp);
	fwrite(&tmStart,1,sizeof(Time),fp);
	fwrite(&tmEnd,1,sizeof(Time),fp);
	
	//Write the extended information
	if (bWriteExtend)
	{
		//Extended information
		fwrite(&triphoneNum,1,sizeof(int),fp);
		
		if (triphoneNum != UNDEF_NUM&& triphoneNum>0)
		{
			assert(triphoneInfo != NULL);
			fwrite(triphoneInfo,1,sizeof(TriphoneInWord)*triphoneNum,fp);
		}
		fwrite(&alscore,1,sizeof(Time),fp);
	}
}



//////////////////////////////////////////////////////////////////////////
//	SegElement
//////////////////////////////////////////////////////////////////////////
SegElement::SegElement(const string& line)	
{
	static LineSplitter fields;
	fields.Split(line);
	
	unsigned int no_fields = fields.NoWords();
	
	assert(no_fields ==3);
	assert(line.find_first_of("S") == 0);
	
	int i;
	for(i=0; i < no_fields; i++)
	{
		int pos = fields[i].find("=");
		assert(pos != string::npos);
		const string& idf = fields[i].substr(0,pos);  // MOD 6/30/2000 - const added
		const string& entry = fields[i].substr(pos+1,string::npos); // MOD 6/30/2000 - const added
		FillSegFields(idf, entry);
	}	
}

void SegElement::FillSegFields(const string& idf, const string& entry )
{
	if (idf == "SegID")
	{
		
	}
	else if(idf == "ST")
	{
		tmStart = (float)atoi(entry.c_str()) * FRAME_TIME;
	}
	else if( idf == "ET" )
	{
		tmEnd = (float)atoi(entry.c_str()) * FRAME_TIME;
	}
	else
	{ 
		cerr << "ERROR: DocElement_FillFields: unknown identifier:";
		cerr << idf << " " << entry << endl;
		exit(1);
	}
}

//////////////////////////////////////////////////////////////////////////
//	DocElement
//////////////////////////////////////////////////////////////////////////
DocElement::DocElement()
{
	id = 0;
	nHypo = 0;
	nSeg = 0;
	nSlice = 0;
	szDocName[0] = 0;

}
DocElement::DocElement(const string& line)	
{
	//init
	id = 0;
	nHypo = 0;
	nSeg = 0;
	nSlice = 0;
	szDocName[0] = 0;

	static LineSplitter fields;
	fields.Split(line);
	
	unsigned int no_fields = fields.NoWords();
	
	assert(no_fields <  MAX_NO_FIELDS_NODE);
	assert(no_fields >= MIN_NO_FIELDS_NODE);
	assert(line.find_first_of("D") == 0);
	
	int i;
	for(i=0; i < no_fields; i++)
	{
		int pos = fields[i].find("=");
		assert(pos != string::npos);
		const string& idf = fields[i].substr(0,pos);  // MOD 6/30/2000 - const added
		const string& entry = fields[i].substr(pos+1,string::npos); // MOD 6/30/2000 - const added
		FillDocFields(idf, entry);
	}	
}
void DocElement::FillDocFields(const string& idf, const string& entry )
{
	if (idf == "DocName")
	{
		strcpy(szDocName, entry.c_str());
	}
	else
	{ 
		cerr << "ERROR: DocElement_FillFields: unknown identifier:";
		cerr << idf << " " << entry << endl;
		exit(1);
	}
}


//////////////////////////////////////////////////////////////////////////
//	query
//////////////////////////////////////////////////////////////////////////
#if 0
Query::Query(const string& line, SyllProns& prons)
{
	static LineSplitter fields;
	fields.Split(line);
	
	unsigned int no_fields = fields.NoWords();
	assert(no_fields >= 2);
	strWord = fields[0];
	nSyll = no_fields-1;
	fPronProb = 1.0;
	int i;
	for(i = 1; i< no_fields; i++)
	{
#ifndef NOTUNE
		Sylls[i-1] = prons.GetIdx(fields[i]);

#else
	//	string str = WordMap.MapFrom(SkipTune(fields[i]));
		string str = SkipTune(fields[i]);
		Sylls[i-1] = prons.GetIdx(str);
#endif
	}	
}
#else
Query::Query(const string& line, Vocab2& vocab,bool bRemoveTone)
{
	static LineSplitter fields("=");
	fields.Split(line);
	unsigned int no_fields = fields.NoWords();
	assert(no_fields >= 2);
	fPronProb = 1.0;
	bUseMCE = false;
	if(no_fields ==3)
	{
		//fPronProb = atof(fields[2].c_str());
		LineSplitter syllProbFields;
		syllProbFields.Split(fields[2]);
		int no_syll = syllProbFields.NoWords();
		int j;
		for(j = 0; j < no_syll; j++)
		{
			fSubwordWgt_a[j] = atof(syllProbFields[2 *j].c_str());
			fSubwordWgt_a[j] = sqrt(fSubwordWgt_a[j]);
			fSubwordWgt_b[j] = atof(syllProbFields[2 *j + 1].c_str());
		}
		bUseMCE = true;
	}

	//char sequnece
	static LineSplitter subfields;
	subfields.Split(fields[0].c_str());
	strWord = "";
	int k;
	for(k =0; k < subfields.NoWords(); k++)
		strWord+= subfields[k];

	//syll sequence
	subfields.Split(fields[1].c_str());
    no_fields = subfields.NoWords();
	numOfSubWord = no_fields;
	int i;
	for(i = 0; i< no_fields; i++)
	{
//#ifndef NOTUNE
		if (!bRemoveTone)
		{
			subwords[i] = vocab.GetIdx(subfields[i]);
		}

//#else
		else
		{
			char tonename;
			string str = SkipTune(subfields[i],tonename);
			subwords[i] = vocab.GetIdx(str);
		}
//#endif
	}	
}

#endif