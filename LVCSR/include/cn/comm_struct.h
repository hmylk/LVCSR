#ifndef _COMM_STRUCT_H

#define _COMM_STRUCT_H

//////////////////////////////////////////////////////////////////////////

//--class 

//hypothesis element structure

#include "comm.h"
#include "Vocab2.h"


//#define NOTUNE //deleted for Uyghur
//#define MAX_SUBWOR_NUM 16
//2-24-2010, JGao
//For the tri-phone information in the indices
struct TriphoneInWord
{
	char triphoneName[TIPHONENAMELEN];
	Time phoneStart;
	Time phoneEnd;
};

class HypoElement
{
public:

	//2-24-2010, JGao
	//re-written  and overload the 
	HypoElement();
	HypoElement(const string& line, Vocab2& vocab,bool bRemoveTone=true);
	HypoElement(const HypoElement&rhs);
	virtual ~HypoElement();
	
	HypoElement& operator=(const HypoElement &rhs);
	//Fill one field or all fields
	void FillHypoFields(const string& idf, const string& entry, Vocab2& vocab,bool bRemoveTone=true);
	void FillHypoFields(float &score,Time &tmStart,Time &tmEnd,const string& entry,Vocab2& vocab,bool bRemoveTone=true);

	//Input, from the forward indices
	void ReadHypo(FILE*fp,bool bReadExtend);
	//Output, write into the forward indice
	void WriteHypo(FILE * fp, bool bWriteExtend);


public:
	WordID wordID;//word id
	float score;  // score of the hypothesis
	float alscore;// acoustic score
	Time tmStart; //start time
	Time tmEnd;   //end time

	//2-24-2010, JGao for store the extend information 
	//Extended information
	int triphoneNum;
	TriphoneInWord *triphoneInfo;
 // mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
	int wordHead;
	int wordTail;
	//maximum posterior of phone's different types
	float maxScore;
#endif
	bool tone[4];
	
};

//slice element structure

struct SliceElement
{
	OffPtr ptrHypo; //position offset in the hypothesis array
	HypoID nHypo;   //number of hypothesis
	WordID refID;   //reference word ID in the lexicon
};



//segment element structure
//For a segment in a speech 

class SegElement
{
public:
	SegElement(){}
	SegElement(const string& line);	
protected:
	void FillSegFields(const string& idf, const string& entry );
public:
	Time tmStart;      //start time in second 
	Time tmEnd;        //start time in second
	OffPtr ptrSlice;   //segment off from ?
	SliceID nSlice;    //number of slices in the confusion network

};


//Document element structure
//To represent a time-coherent speech  
class DocElement
{
public:
	DocElement();
	DocElement(const string& line);

protected:
	void FillDocFields(const string& idf, const string& entry );
public:
	DocID id;
	char szDocName[256];//the document name   
	SegID ptrSeg;       //to locate start position of segments storage 
	uint nSeg;          //number of segments in the document
	uint nSlice;    
	uint nHypo;
};


//Use the bit field to compress the inverted index
struct HypoPos
{
	operator INT64()
	{
		INT64 pos;
		pos = docID;
		pos = pos << 16;
		pos += segID;
		pos = pos << 16;
		pos += sliceID;
		return pos;
	}
	INT64 docID:32;	 //global position	
	INT64 segID:16;  //Segment inside a document
	INT64 sliceID:16;//Slice off under a particular segment
};
//hit element
class HitElement2
{
public:
	HypoPos	pos;
	short bRef;
	Time tmStart;
	Time tmEnd;
	LnProb score;
	LnProb alscore;
};

//2-24-2010, JGao for acoustic re-scoring
#if BACK_TRACE_ALL
typedef struct TracePos
{
	HypoPos  pos;
	WordID	 wordID;
	HypoID   hypoID;
	TracePos()
	{
		wordID =INVALID_WORDID;
		hypoID = INVALID_HYPOID;
	}
}TracePos;
typedef vector<TracePos>TraceHistory;
#endif

class ConfHitElement: public HitElement2
{
public:
	ConfHitElement()
	{
#if BACK_TRACE_ALL 
		NumOfExactMatch = 0;
		Trace.resize(INIT_TRACE_NUM);
		nTrace=0;
#endif
	}
	ConfHitElement(HitElement2& hit)
	{
		*((HitElement2 *)this) = hit;
#if BACK_TRACE_ALL 
		NumOfExactMatch = 0;
		Trace.resize(INIT_TRACE_NUM);
		nTrace =0;
#endif
	}
	int	  queryID;			//Query
#if BACK_TRACE_ALL 
	ushort NumOfExactMatch;
	TraceHistory Trace;
	int nTrace;
#endif
};

class Query
{
public:
	Query()
	{
		strWord = "";
		numOfSubWord = 0;
		fPronProb = 1.0;
		bUseMCE = false;
		numOfPhone=0;
		queryID=-1;
		memset(phones,0,MAX_SUBWOR_NUM);
	}
	Query(const string& line, Vocab2& vocab,bool bRemoveTone=true);

public:
	string strWord;
	int	  queryID;
	uint  subwords[MAX_SUBWOR_NUM];
	uint  numOfSubWord;
	float fSubwordWgt_a[MAX_SUBWOR_NUM];
	float fSubwordWgt_b[MAX_SUBWOR_NUM];
	float fPronProb;
	int bUseMCE;

	//3-5-2010, JGao
	//The phonetic level transcription of the keyword
	//Used for acoustic re-scoring of the keyword
	uchar phones[MAX_SUBWOR_NUM];
	uint  numOfPhone;
	
};

struct Comp_Hit1 {
    bool operator() (const HitElement2& a, const HitElement2& b) const
	{
		bool bRel = false;
		if(a.pos.docID < b.pos.docID)
		{
			bRel = true;
		}
		else if(a.pos.docID == b.pos.docID)
		{
			if(a.pos.segID < b.pos.segID)
				bRel = true;
			else if(a.pos.segID == b.pos.segID)
			{
				if(a.score >= b.score)
					bRel = true;
			}
		}
		return bRel;

    }
};


struct Comp_Hit2 
{
    bool operator() (HitElement2& a, HitElement2& b) const
	{
		return ((INT64)(a.pos) <= (INT64)(b.pos));

    }
};


struct Comp_Hit_Score {
    bool operator() (const HitElement2& a, const HitElement2& b) const
	{
	
		return a.score > b.score;

    }
};




#endif









































