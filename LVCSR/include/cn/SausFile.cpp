// SausFile.cpp: implementation of the SausFile class.
//
//////////////////////////////////////////////////////////////////////

#include "SausFile.h"
#include <math.h>
#include <string.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SausFile::SausFile(const string& strSausFile, Vocab2& vocab,bool bSkipTune):Hypos(100), Slices(1024)
{
	//Initialization
	nHypo = 0;
	nSlice = 0;

	//load file
	ifstream h;
	h.open(strSausFile.c_str());
	
	string line;
	
	while(h.peek() != EOF)
	{
		//search--- slice
		while((h.peek() != EOF) && (h.peek() != 'S'))
		{
			getlineH(h, line); 
			assert(h.good());
			continue;
		}
		
		// read Slice Info
		if(h.peek() == 'S')
		{
			getlineH(h, line); 
			assert(h.good());
			AddSlice();
		}

		//read Hypo info one by one
		while((h.peek() != EOF) && (h.peek() == 'H'))
		{
			getlineH(h, line);
			assert(h.good());
			HypoElement this_hypo(line, vocab,bSkipTune);
			if(this_hypo.wordID == (WordID)(-1))
				continue;
			AddHypo(this_hypo,bSkipTune);
		}

		//read reference information
		if(h.peek() != EOF && h.peek() == 'R')
		{
			getlineH(h, line);
			assert(h.good());

			string fields = line;
			int pos = fields.find("=");
			assert(pos != string::npos);
			const string& entry = fields.substr(pos+1,string::npos); 
			SliceElement& this_slice = Slices[nSlice -1];
			this_slice.refID = vocab.GetIdx(entry);	
		}
	}
	h.close();

}

// mod for eng [10/28/2010 zhzhang]
#ifdef USE_ENG
SausFile::SausFile(const string& strSausFile, Vocab2& vocab,int addBoundary,bool bSkipTune):Hypos(100), Slices(1024)
{
	//Initialization
	nHypo = 0;
	nSlice = 0;
	
	//load file
	ifstream h;
	h.open(strSausFile.c_str());
	
	string line;
	
	while(h.peek() != EOF)
	{
		//search--- slice
		while((h.peek() != EOF) && (h.peek() != 'S'))
		{
			getlineH(h, line); 
			assert(h.good());
			continue;
		}
		
		// read Slice Info
		if(h.peek() == 'S')
		{
			getlineH(h, line); 
			assert(h.good());
			AddSlice();
		}
		
		//read Hypo info one by one
		while((h.peek() != EOF) && (h.peek() == 'H'))
		{
			getlineH(h, line);
			assert(h.good());
			HypoElement this_hypo(line, vocab,bSkipTune);
			if(this_hypo.wordID == (WordID)(-1))
				continue;
			if(addBoundary == 1)
			{
				AddHypo(this_hypo,vocab);
			}
			else
			{
				AddHypo(this_hypo,bSkipTune);
			}
		}
		
		//read reference information
		if(h.peek() != EOF && h.peek() == 'R')
		{
			getlineH(h, line);
			assert(h.good());
			
			string fields = line;
			int pos = fields.find("=");
			assert(pos != string::npos);
			const string& entry = fields.substr(pos+1,string::npos); 
			SliceElement& this_slice = Slices[nSlice -1];
			this_slice.refID = vocab.GetIdx(entry);	
		}
	}
	h.close();
	h.clear();// 1bug [11/2/2010 zhzhang]
	
}
#endif


SausFile::~SausFile()
{

}
/*
#if 1
void SausFile::Process(Dictionary& dict, Vocab& phoneVocab)
{
	int phoneNum = phoneVocab.Size();
	
	float * fProbPhones = new float[phoneNum];

	for(SliceID sliceID = 0; sliceID < nSlice; sliceID++)
	{
		//calculate the probability of phones
		for(int i = 0; i < phoneNum; i++)
			fProbPhones[i] = 0.0f;
		

		SliceElement this_slice = GetSlice(sliceID);
		for(HypoID hypoID = 0; hypoID < this_slice.nHypo; hypoID++)
		{
			HypoElement this_hypo = GetHypo(sliceID, hypoID);
			WordEntry& entry = dict.GetEntry(this_hypo.wordID);

			if(entry.Initial == (WordID)(-1) || entry.Final == (WordID)(-1))
				continue;
			fProbPhones[entry.Initial] += this_hypo.score;
			fProbPhones[entry.Final] += this_hypo.score;
		}
			
		//calculate the confidence score based on phones
		float total_alscore = 0.0f;
		float null_alscore = 0.0f;
		for(hypoID = 0; hypoID < this_slice.nHypo; hypoID++)
		{
			HypoElement& this_hypo = GetHypo(sliceID, hypoID);
			WordEntry& entry = dict.GetEntry(this_hypo.wordID);
			if(entry.Initial == (WordID)(-1) || entry.Final == (WordID)(-1))
			{
				this_hypo.alscore = this_hypo.score;
				null_alscore = this_hypo.score;
			}
			else
			{
				float x1 = pow(fProbPhones[entry.Initial], 1.0);
				float x2 = pow(fProbPhones[entry.Final], 1.0);
				this_hypo.alscore = sqrt( x1 *  x2);
				total_alscore+= this_hypo.alscore;
			}
		}

		
		//normalize
		for(hypoID = 0; hypoID < this_slice.nHypo; hypoID++)
		{
			HypoElement& this_hypo = GetHypo(sliceID, hypoID);
			WordEntry& entry = dict.GetEntry(this_hypo.wordID);
			if(entry.Initial != (WordID)(-1) && entry.Final != (WordID)(-1))
			{
				this_hypo.alscore = (1.0f - null_alscore) * this_hypo.alscore/ total_alscore; 
			}
		}
		


	}

	delete[] fProbPhones;
	
}
#else
void SausFile::Process(Dictionary& dict, Vocab& phoneVocab)
{
	for(SliceID sliceID = 0; sliceID < nSlice; sliceID++)
	{
		SliceElement this_slice = GetSlice(sliceID);
		for(HypoID hypoID = 0; hypoID < this_slice.nHypo; hypoID++)
		{
			HypoElement this_hypo = GetHypo(sliceID, hypoID);
			WordEntry& entry = dict.GetEntry(this_hypo.wordID);

			if(entry.Initial == (WordID)(-1) || entry.Final == (WordID)(-1))
				continue;

			float x1 = pow(this_hypo.score, 0.8);
			float x2 = pow(this_hypo.score, 1.3);
			this_hypo.alscore = sqrt( x1 *  x2);
		}
		//normalize
		for(hypoID = 0; hypoID < this_slice.nHypo; hypoID++)
		{
			HypoElement& this_hypo = GetHypo(sliceID, hypoID);
			WordEntry& entry = dict.GetEntry(this_hypo.wordID);
			if(entry.Initial != (WordID)(-1) && entry.Final != (WordID)(-1))
			{
				this_hypo.alscore = (1.0f - null_alscore) * this_hypo.alscore/ total_alscore; 
			}
		}
	}
}
#endif
void SausFile::Print(Vocab& vocab)
{
	for(int i = 0; i < nSlice; i++)
	{
		SliceElement& this_slice = Slices[i];
		printf("\nS=%-6d\n", i);
		for(int j = 0; j < this_slice.nHypo; j++)
		{
			HypoElement& this_hypo = Hypos[this_slice.ptrHypo+j];
			printf("H=%-6d Word=%-10s Score=%-8.3f ST=%-8.2f ET=%-8.2f\n", 
				j, vocab.GetWord(this_hypo.wordID).c_str() , this_hypo.score, this_hypo.tmStart, this_hypo.tmEnd);
		}
	}
}
*/
