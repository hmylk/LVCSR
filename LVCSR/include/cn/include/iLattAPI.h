#ifndef _ILAT_API_H
#define _ILAT_API_H

#ifndef lint
static char Copyright[] = "Copyright (c) 1997-2006 SRI International.  All Rights Reserved.";
static char RcsId[] = "@(#)$Id: lattice-tool.cc,v 1.132 2006/09/20 19:55:30 stolcke Exp $";
#endif

#ifdef PRE_ISO_CXX
# include <iostream.h>
#else
# include <iostream>
using namespace std;
#endif
#include <stdio.h>
#include <math.h>
#include <locale.h>
#include <errno.h>
#ifndef _MSC_VER
#include <sys/time.h>
#include <unistd.h>
#endif
#include <signal.h>
#include <setjmp.h>

#ifndef SIGALRM
#define NO_TIMEOUT
#endif

#include "option.h"
#include "version.h"
#include "Vocab.h"
#include "MultiwordVocab.h"
#include "ProductVocab.h"
#include "Lattice.h"
#include "MultiwordLM.h"
#include "Ngram.h"
#include "ClassNgram.h"
#include "SimpleClassNgram.h"
#include "ProductNgram.h"
#include "BayesMix.h"
#include "RefList.h"
#include "LatticeLM.h"
#include "WordMesh.h"
#include "zio.h"
#include "mkdir.h"
#include "comm.h"
//#include "LattAPI.h"

class LatParam
{
public:
	bool splitMultiwords;
	bool skipTune;
// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
	int addBoundary;
#endif
	float fHTKScale;
	float fHTKWdPenalty;
	float fHTKLogBase;
	float fPosteriorScale;
	float fPosteriorPrune;
	char multiChar[256];
	char vocabFile[256];
	char multiwordDictFile[256];
	int  nbestDecode;
	int  nbestViterbi;
	char nbestOutDir[256];
	LatParam();
};


class iLatAPI
{
public:
	iLatAPI(LatParam& param);
	~iLatAPI();
	bool GenCN(char* inLat, char* outMesh, HTKLattice *hLat = NULL, HTKHeader2 *hHeader = NULL);
	LatParam m_latParam;
 private:
	Vocab *vocab;
	SubVocab *noiseVocab;
	SubVocab *ignoreVocab;
	LHash<const char*, Array< Array<char*>* > > multiwordDict;
	HTKHeader *htkparms;
};


#endif