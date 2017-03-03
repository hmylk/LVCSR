
/*++
---------------------------------------------------------------------------------
Name:       ConfusionNet.h

Function:   Definition for ConfusionNet class,it is related to a segment 

Author:     JGao

Time  :     2007/06/17

Revision :  0.1

Copyright:  ThinkIT Speech Lab

---------------------------------------------------------------------------------
--*/
#ifndef _CONFUSIONNET_H_
#define _CONFUSIONNET_H_
#include "comm.h"
#include "comm_struct.h"
#include "SausFile.h"
/*
 *	 Information about the confusion network
 *   The most important is time shift
 */
typedef struct ConfInfor
{
  float fTimeStart;//Start time shift to the session start in seconds
  float fTimeEnd;  //End time shift to the session start in seconds
  char  szUttMatName[1024];	//lita added [8-4-2010], ×´Ì¬¾ØÕóµÄÂ·¾¶
}ConfInfor;

class ConfusionNet  
{	
public:
	ConfusionNet();
	ConfusionNet(SausFile&cnfile);
	virtual ~ConfusionNet();
	ConfusionNet& operator = (const ConfusionNet& cnnet);

 /*
  *	Get Slice & Hypothesis number in the confusion network
  */
	uint GetSliceNo();
	uint GetHypoNo();
 /*
  *	Get a Slice or hypothesis given their ID
  */ 
	SegElement    GetSeg();
    SliceElement& GetSlice(SliceID id);
    HypoElement & GetHypo (SliceID sliceID, HypoID hypoID);

	//2-24-2010, JGao
    HypoID	GetHypoID(SliceID sliceID,WordID wordID);

 

 /*
  *	Get&Set information 
  */
	bool SetInfor(const ConfInfor infor);
	bool GetInfor(ConfInfor& infor);

	bool AddSlice(); 
	bool AddHypo(HypoElement& hypo,bool bRemoveTone=true);
//private:
	vector<SliceElement> m_Slices; //the slice information:Word for reference,number of hypothesis,position
	vector<HypoElement>  m_Hypos;  //the hypo  information:Word,Score,time etc.
	int m_nSlice;                  //number of slices
	int m_nHypo;                   //Total number of hypothesis
	ConfInfor m_infor;             //Information set by outside
};

#endif//_CONFUSIONNET_H_
