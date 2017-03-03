/*******************************************************************************
 *
 *                       HCCL PROPRIETARY INFORMATION
 *
 * This software is supplied under the terms of the license agreement
 *   or nondisclosure agreement with HCCL and may not be copied
 * or disclosed except in accordance with the terms of that agreement.
 *
 *                Copyright (c) 2007 HCCL. All Rights Reserved.
 *
 *					rewritten by jshao on 2007-1-10
 *
 ******************************************************************************
 *	OnePass is a tool to drive decoder class running. Onepass can recognize a list of feature files.
 *  generate HTK lattice or top-1 transcriptions.
 *	1.	Construct OnePass object and set configuration 
 *  2.  Init onepass: Attach AM, TM, LM
 *  3.  Decoding a list of files
 *  4.	Clean and deconstruct
 *
 ******************************************************************************/

#ifndef TOOLS_ONEPASS_ONEPASS
#define TOOLS_ONEPASS_ONEPASS


#include "comm.h"
#include "../LVCSR/LVCSR_API.h"


class LvcsrConfig;
class BaseEngine;

class OnePass
{
public:
    OnePass();
    OnePass(char *sysdir,char *outdir,char* tasklist, char * paramfile);
    ~OnePass();

    void InitByCfg(LvcsrConfig *cfg);

    void Init();

    void Decoding(char *file);
    
    void Clean();

    int	 LoadSubTree(char* treeFile);

    int	 LoadSubTree(char* buf, int len);

    int	 FreeSubTree(int treeID);

    bool DoFeature(float *newSet, int frameNum, char *featureName, TSR_STREAM_FLAG flag);

    bool DoFile(char *file);

    void DoUtterance(int start = -1, int end = -1);

    void * GetCN(int DataTimestamp, int DataTimestampEnd, char *format, int &wordNum);

    void SetLog(LogFile* PLog);

    int PrepareForWork(SOUND_TYPE WorkType);

    void* GetLatticeResult(); 

    void * LoadLat(char *szFileName, char *szSausFileName, int DataTimestamp, int DataTimestampEnd, char *format, int &wordNum, bool bDelLat, bool bDelSaus);


    char* GenHTKLattice(char* lattice_id, bool iname_and_lname);
    char* GenConfusionNetwork();

    void OutputHTKLattice(char* fname);
    void OutputConfusionNetwork(char* fname);

    char* GetLatticeOutDir();

    void LogOut(const char* msg);
	
	//  [3/11/2014 Administrator]
	//int CalPLPPPhone(aTriPhoneLnk *phlnk, float *featSet, int nFrameNum,int vecSize);
	int CalPLPLWord(aWORDLINK *wordlnk,float *featSet,int nFrameNum,int nFrameSize);
	int AdjustPhnBnd(aWORDLINK *worlnk,float *featSet,int nFrameNum,int nFrameSize);

	void InitForRescore();
	void SetLoadModelOrNot(bool bLoad);

protected:
private:
    int  NextSpeaker(void);
    void DoInitSpeaker();
    bool DoCmd(char *cmd);
    void DoInitUtterance();
    void DoEndSpeaker();

    void InitCriticalSection() ;
    void DelCriticalSection() ;

    BaseEngine* recEngine_;
    char sysdir_[2048];
    char paramfile_[2048];
    char scheduler_[2048];
    char outDir_[2048];
};



#endif
