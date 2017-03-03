// Vocab2.h: interface for the Vocab2 class.
// Author: J.Shao 
// Time:	2005.12.18
//////////////////////////////////////////////////////////////////////

#ifndef _Vocab2_H
#define _Vocab2_H
#include "comm.h"
#include "File.h"


const unsigned  INIT_VOC_SIZE           = 10000; // max no of words in the Vocab2ulary

//const unsigned int	maxWordLength = 256;


class Vocab2  
{
public:
	//constructor
	Vocab2(const string& filename, bool bToLower = false);
	void Construct(const string& filename, bool bToLower = false);
	Vocab2();////do nothing ,then call the construct
	virtual ~Vocab2();

	//this is the same as the first contructor

	
	//insert, delete, modify
	void AddWord( char* name);

	void SetToLower(bool bValue){ bToLower = bValue; }
	
	//access
    WordID GetIdx( const string& w );	//if return -1, the word is not found.
    string GetWord( WordID idx );
    int Size() const { return size; }

	void Print();
	

protected:
	//io operation
	unsigned int Read( const string& filename );


private:
	WordIntHMap word2int;
	vector<string> int2word;
	int size;
	bool bToLower;			// map word strings to lowercase
};

#endif //#ifndef _Vocab2_H
