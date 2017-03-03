// Vocab2.cpp: implementation of the Vocab2 class.
//
//////////////////////////////////////////////////////////////////////
#include "Vocab2.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Vocab2::Vocab2(const string& filename, bool bToLower):bToLower(bToLower)
{
  Construct(filename, bToLower);
}

//do nothing ,then call the construct
Vocab2::Vocab2()
{
}
Vocab2::~Vocab2()
{
}

void Vocab2::Construct(const string& filename, bool bToLower /* = false */)
{
	this->bToLower = bToLower;
	size = 0;
	int2word.resize(INIT_VOC_SIZE);
	Read(filename);	
	int2word.resize(size);
}

// map word string to lowercase, returns a static buffer
static char *
mapToLower(char* name)
{
	const unsigned int	maxWordLength = 256;
    static char lower[maxWordLength + 1];

    unsigned  i;
    for (i = 0; name[i] != 0 && i < maxWordLength; i++) 
	{
		lower[i] = tolower((unsigned char)name[i]);
    }
    lower[i] = '\0';
    assert(i < maxWordLength);
    return lower;
}

void
Vocab2::AddWord(char* name)
{
	if(bToLower)
		name = mapToLower(name);
	//string sname = name;
	char *sname = strdup(name);
	if (word2int.find(sname) != word2int.end())
	{
		//printf("%d != %d\n", word2int.find(sname), word2int.end());
		cerr <<"The word "<< sname <<" has existed!"<< endl;
		return;
	}
	//map
	word2int[sname] = size;

	//container
	if(int2word.size() == size)
		int2word.push_back(sname);
	else
		int2word[size] = sname;

	free(sname);
	sname=NULL;
    size++;
}



//////////////////////////////////////////////////////////////////////////
//---Access
WordID
Vocab2::GetIdx( const string& w )
{
	//if (w == "<exit>" || w == "<enter>") return -1;   // 070606 syq disable this warning message
	if (word2int.find(w.c_str()) == word2int.end()){
		cerr << "The word " << w << " could not be found in your dictionary " << endl;
		return -1;
	}
    return word2int[w.c_str()];	
}

string
Vocab2::GetWord( const WordID idx )
{
	assert(idx < size);
	return int2word[idx];
}


// Read Vocab2ulary from file
unsigned int
Vocab2::Read(const string& filename)
{
	File file(filename.c_str(), "rt");

    char *line;
    while (line = file.getline()) 
	{
		char* word = strtok(line, wordSeparators);
		AddWord( word );
    }
	file.close();

    return size;
}

void Vocab2::Print()
{
	int i;
	for(i = 0; i < Size(); i++)
	{
		cout<<GetWord(i)<<endl;
	}
}



