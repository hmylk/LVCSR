 #ifndef _DICTIONARY_H
#define _DICTIONARY_H
#include "comm.h"
#include "File.h"
#include <string>
#include "Vocab2.h"

struct WordEntry{
	WordID Initial;
	WordID Final;
};
class Dictionary  
{
public:
	//constructor
	Dictionary(const string& filename, Vocab2& vocab, Vocab2& phoneVocab);
	virtual ~Dictionary()
	{
	};

	WordEntry& GetPron(WordID id)
	{
		ASSERT2(id < words.size(), "exceed max word id\n");
		return words[id];
	};

private:
	vector<WordEntry> words;
};

#endif //#ifndef _VOCAB_H
