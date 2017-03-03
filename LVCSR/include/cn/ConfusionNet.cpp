
/*++
---------------------------------------------------------------------------------
Name:       ConfusionNet.cpp

Function:   Implementation for ConfusionNet class

Author:     JGao

Time  :     2007/06/17

Revision :  0.1

Copyright:  ThinkIT Speech Lab

---------------------------------------------------------------------------------
--*/

#include "ConfusionNet.h"

ConfusionNet::ConfusionNet()
{
	//Initialization
	m_nHypo = 0;
	m_nSlice = 0;
	m_Hypos.resize(100);
	m_Slices.resize(100);
}
ConfusionNet::~ConfusionNet()
{
}

uint ConfusionNet::GetSliceNo()
{
		return m_nSlice;
}

uint ConfusionNet::GetHypoNo()
{
	     return m_nHypo;
}

SegElement  ConfusionNet::GetSeg()
{
	SegElement segEle;
	segEle.tmStart = m_infor.fTimeStart;
	segEle.tmEnd = m_infor.fTimeEnd;
	segEle.nSlice = m_nSlice;
	return segEle;
	
}
SliceElement& ConfusionNet::GetSlice(SliceID id)
{
		assert(id < m_nSlice);
		return m_Slices[id];
}

HypoElement& ConfusionNet::GetHypo(SliceID sliceID, HypoID hypoID)
{
		assert(sliceID < m_nSlice);
		SliceElement& this_slice = m_Slices[sliceID];
		OffPtr ptrThisHypo = this_slice.ptrHypo + hypoID;
		assert(ptrThisHypo < m_nHypo);
		return m_Hypos[ptrThisHypo];
}

bool ConfusionNet::SetInfor(const ConfInfor infor)
{
  m_infor = infor;
  return true;
}

bool ConfusionNet::GetInfor(ConfInfor& infor)
{
  infor = m_infor;
  return true;
}
ConfusionNet& ConfusionNet::operator = (const ConfusionNet& cnnet)
{
	int nSize = cnnet.m_Slices.size();
	m_Slices.resize(nSize);
	m_Slices.assign(cnnet.m_Slices.begin(),cnnet.m_Slices.begin()+nSize);
   
	nSize = cnnet.m_Hypos.size();
	m_Hypos.resize(nSize);
	m_Hypos.assign(cnnet.m_Hypos.begin(),cnnet.m_Hypos.begin()+nSize);
	
	m_nSlice = cnnet.m_nSlice;                  //number of slices
	m_nHypo = cnnet.m_nHypo;                   //Total number of hypothesis
	m_infor = cnnet.m_infor ;             //Information set by outside
	return *this;
}


/*************************************************************************
 *
 * AddSlice()
 *
 * Add a slice in the confusion network
 *
 * Parameters:
 *
 *
 * Return value: 1 successful,0 fail
 *
 ***********************************************************************/

bool ConfusionNet::AddSlice()
{
	SliceElement slice;
	slice.ptrHypo = m_nHypo;//Add to the end
	slice.nHypo = 0;  //No hypothesis default
	slice.refID = -1; //No default reference here
	if(m_Slices.size() <= m_nSlice)
			m_Slices.push_back(slice);
	else
			m_Slices[m_nSlice] = slice;
	m_nSlice++;

	return true;
}
bool ConfusionNet::AddHypo(HypoElement& hypo,bool bRemoveTone)
{
	bool bExist = false;
//#ifdef NOTUNE
	if(bRemoveTone)
	{
		SliceElement& this_slice = m_Slices[m_nSlice-1];
		int i=0;
		for(i = 0; i < this_slice.nHypo; i++)
		{
			HypoElement &old_hypo = m_Hypos[this_slice.ptrHypo+i];
			if(old_hypo.wordID == hypo.wordID)
			{
				//logadd
				old_hypo.score = old_hypo.score+ hypo.score;
				bExist = true;
			}
			else
			{
				continue;
			}
		}
	}
//#endif
		if(!bExist)
		{
			if(m_Hypos.size() <= m_nHypo)
			{
				m_Hypos.push_back(hypo);
			}
			else
			{
				m_Hypos[m_nHypo] = hypo;
			}

			m_nHypo++;
			m_Slices[m_nSlice-1].nHypo++;
		}//end if
		return true;
}



//2-24-2010, JGao
HypoID ConfusionNet::GetHypoID(SliceID sliceID,WordID wordID)
{
	
	if (sliceID > m_nSlice)
	{
		return INVALID_ID;
	}
	SliceElement& this_slice = m_Slices[sliceID];
	for(HypoID hypoID = 0; hypoID < this_slice.nHypo; hypoID++)
	{
		
		OffPtr ptrThisHypo = this_slice.ptrHypo + hypoID;
		assert(ptrThisHypo < m_nHypo);
		HypoElement& this_hypo = m_Hypos[ptrThisHypo];
		if (this_hypo.wordID == wordID)
		{
			return hypoID;
		}
	}
	return INVALID_ID;
}