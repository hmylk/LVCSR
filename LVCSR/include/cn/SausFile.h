// SausFile.h: interface for the SausFile class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SAUSFILE_H
#define _SAUSFILE_H
#include "comm.h"
#include "comm_struct.h"
#include "dictionary.h"

class SausFile  
{	
public:
	SausFile(const string& strSausFile, Vocab2& vocab,bool bSkipTune=true);
// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
	SausFile(const string& strSausFile, Vocab2& vocab,int addBoundary,bool bSkipTune=true);
#endif
	virtual ~SausFile();
	friend class DocBinFile;
	//Clear
	void  Init()
	{
		nSlice = 0;
		nHypo  = 0;
		bScoreOnLog = false;
	}
	 bool	Score2Log();
	 bool	RelativeTime2AbsoluteTime(Time ST);

	void Process(Dictionary& dict, Vocab2& phoneVocab);

	uint GetSliceNo()
	{
		return nSlice;
	}

/*	const*/ SliceElement& GetSlice(SliceID id)
	{
		assert(id < nSlice);
		return Slices[id];
	}

/*	const */HypoElement& GetHypo(SliceID sliceID, HypoID hypoID)
	{
		assert(sliceID < nSlice);
		SliceElement& this_slice = Slices[sliceID];
		OffPtr ptrThisHypo = this_slice.ptrHypo + hypoID;
		assert(ptrThisHypo < nHypo);
		return Hypos[ptrThisHypo];
	}

	void Print(Vocab2& vocab);

protected:
	void AddSlice() 
	{
		SliceElement slice;
		slice.ptrHypo = nHypo;
		slice.nHypo = 0;
		slice.refID = -1;
		if(Slices.size() <= nSlice)
			Slices.push_back(slice);
		else
			Slices[nSlice] = slice;
		nSlice++;
	}

	void AddHypo(HypoElement& hypo,bool bRemoveTone=true) //  [3/8/2011 zhzhang]
	{
		bool bExist = false;
//#ifdef NOTUNE
		if (bRemoveTone)//1-24-2010, JGao
		{
			
			SliceElement& this_slice = Slices[nSlice-1];
			for(int i = 0; i < this_slice.nHypo; i++)
			{
				HypoElement &old_hypo = Hypos[this_slice.ptrHypo+i];
				if(old_hypo.wordID == hypo.wordID)
				{
					//logadd
					old_hypo.score = old_hypo.score+ hypo.score;
					//old_hypo.score = max(old_hypo.score, hypo.score);
					bExist = true;
					/*for (int m=0;m<4;m++)
					{
						if (old_hypo.tone[m])
						{
							continue;
						}
						else
						{
							old_hypo.tone[m]=hypo.tone[m];// 出现过的调记录下来 [3/13/2011 zhzhang]
						}
					}*/
				}
				else
				{
					continue;
				}
			}
		}
//#endif
		if(!bExist){
			if(Hypos.size() <= nHypo)
				Hypos.push_back(hypo);
			else
				Hypos[nHypo] = hypo;
			nHypo++;
			Slices[nSlice-1].nHypo++;
		}
		//  [3/8/2011 zhzhang]

	

	}

	// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
//#define WFRAG
#ifndef WFRAG
	void AddHypo(HypoElement& hypo, Vocab2& vocab) 
	{
		bool bExist = false;
		SliceElement& this_slice = Slices[nSlice-1];
		
		string oldHypoWord, hypoWord;
		
		hypoWord = vocab.GetWord(hypo.wordID);
		if(hypoWord.substr(0,1) != "<" && hypoWord.substr(0,1) != "!")
		{
			int k=0;
			for(k = hypoWord.size() - 1; k >= 0; k--)
			{
				string aChar = hypoWord.substr(k,1);  
				if(aChar >= "a" && aChar <= "z")
				{
					break;
				}
				else
				{
					if(aChar == "*")//add boundary information
					//if(aChar == "1")//add boundary information
					{
						hypo.wordHead = 1; 
					}
					else
					{
						if(aChar == "#")
						//if(aChar == "2")
						{
							hypo.wordTail = 1;
						}
					}
				}
			}
			assert(k >= 0);
			hypo.wordID = vocab.GetIdx(hypoWord.substr(0,k+1));
			assert(hypo.wordID != -1);
		}
		
		for(int i = 0; i < this_slice.nHypo; i++)
		{
			HypoElement &old_hypo = Hypos[this_slice.ptrHypo+i];
			
			if(old_hypo.wordID == hypo.wordID)
			{
				if(hypo.maxScore > old_hypo.maxScore)
				{
					old_hypo.maxScore = hypo.maxScore;
					old_hypo.wordHead = hypo.wordHead;
					old_hypo.wordTail = hypo.wordTail;
				}
				old_hypo.score = old_hypo.score+ hypo.score;
				bExist = true;
			}
			else
			{
				continue;
			}
		}
		
		if(!bExist){
			if(Hypos.size() <= nHypo)
				Hypos.push_back(hypo);
			else
				Hypos[nHypo] = hypo;
			nHypo++;
			Slices[nSlice-1].nHypo++;
		}
	}
#else
	void AddHypo(HypoElement& hypo, Vocab2& vocab) 
	{
		bool bExist = false;
		SliceElement& this_slice = Slices[nSlice-1];
		
		string oldHypoWord, hypoWord;
		
		hypoWord = vocab.GetWord(hypo.wordID);
		hypo.wordHead = 0; 
		hypo.wordTail = 0;
		if(hypoWord.substr(0,1) != "<" && hypoWord.substr(0,1) != "!")
		{
			//printf("the subword is %s \n",hypoWord.c_str());
			//Sleep(1000);
			for(int k = hypoWord.size() - 1; k >= 0; k--)
			{
				string aChar = hypoWord.substr(k,1);  // [12/10/2010 zhzhang]
				//	printf("the substr is %s \n",aChar.c_str());
			
				if(aChar >= "a" && aChar <= "z")
				{
					continue;//  [12/10/2010 zhzhang]
				}
				else
				{
					if(aChar == "1")//add boundary information
					{
						hypo.wordHead = 1; 
					}
					else
					{
						if(aChar == "2")
						{
							hypo.wordTail = 1;
						}
					}
				}
			}
			
			assert(k >= -1);
			int pos_start=0;
			int pos_end=0;
			if (hypo.wordHead==1 && hypo.wordTail==1)
			{
				pos_start=1;
				pos_end=hypoWord.size()-2;
			}
			else if(hypo.wordHead==1)
			{
				pos_start=1;
				pos_end=hypoWord.size()-1;
			}
			else if(hypo.wordTail==1)
			{
				pos_start=0;
				pos_end=hypoWord.size()-2;
			}
			else
			{
				pos_start=0;
				pos_end=hypoWord.size()-1;
			}
			
			
			hypo.wordID = vocab.GetIdx(hypoWord.substr(pos_start,(pos_end-pos_start+1)));
			assert(hypo.wordID != -1);
		}
		
		for(int i = 0; i < this_slice.nHypo; i++)
		{
			HypoElement &old_hypo = Hypos[this_slice.ptrHypo+i];
			
			if(old_hypo.wordID == hypo.wordID)
			{
				if(hypo.maxScore > old_hypo.maxScore)
				{
					old_hypo.maxScore = hypo.maxScore;
					old_hypo.wordHead = hypo.wordHead;
					old_hypo.wordTail = hypo.wordTail;
				}
				old_hypo.score = old_hypo.score+ hypo.score;
				bExist = true;
			}
			else
			{
				continue;
			}
		}
		
		if(!bExist){
			if(Hypos.size() <= nHypo)
				Hypos.push_back(hypo);
			else
				Hypos[nHypo] = hypo;
			nHypo++;
			Slices[nSlice-1].nHypo++;
		}
	}
#endif
#endif

//private:
public:
	vector<SliceElement> Slices;
	vector<HypoElement> Hypos;
	int nSlice;
	int nHypo;
	bool	bScoreOnLog;
};

#endif // !defined(AFX_SAUSFILE_H__1ED136C7_0AD2_4E5B_8882_7A3890FCE17E__INCLUDED_)
