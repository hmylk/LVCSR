
#ifndef __COM_DEFINITION__H
#define __COM_DEFINITION__H

#include "handleFile.h"
#include "../../LVCSR/LVCSR_API.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <ctype.h>
#include <vector>
#include <map>

#ifdef WIN32
#include <io.h>
#include <windows.h>
//#include <process.h>
#else
#include "porting.h"
#endif
using namespace std;

#define   MAX_KW_INTEM  1000       //max number of keywords
#define   NUM_OF_THREAD 100        //number of instance
//#define   SEND_INTERVAL_MS   100   //send data period in ms
#define   TKW_SAMPLE_RATE    8000
#define   MIN_SCORE          0.0


//for lvcsr result
struct SortedResult
{
	char*      segResult;
	SortedResult*  sortedRstNext;
	SortedResult()
	{
		segResult=NULL;
		sortedRstNext = NULL;
	}
};

struct ThreadParam
{
	int sessId;
};

int NormPathDelimters(char *filename);
//change the file delimiters
int NormFileDelimters(char *filename);
void chomp(char *buf);
int ParseFormat(char *szFormat, SPEECHTYPE &format);      // parse voice format for onlyrec
//int ParseFormat(char *szFormat, TKW_SPEECHTYPE &format);   //parse voice format for kws


#endif