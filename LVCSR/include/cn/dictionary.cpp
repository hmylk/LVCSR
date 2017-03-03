#include "dictionary.h"
#include "LineSplitter.h"
Dictionary::Dictionary(const string& filename, Vocab2& vocab, Vocab2& phoneVocab)
{
	File file(filename.c_str(), "rt");

    char *line;
	int vocabNum = vocab.Size();
	words.resize(vocabNum);
	int i;
	for(i = 0; i < vocabNum; i++){
		words[i].Initial = -1;
		words[i].Final = -1;
	}

    while (line = file.getline()) 
	{
		static LineSplitter fields;
		fields.Split(line);
		unsigned int no_fields = fields.NoWords();

		if(no_fields != 5)
			continue;

		WordID word_id = vocab.GetIdx(fields[0]);
		WordID word_initial = phoneVocab.GetIdx(fields[2]);
		WordID word_final = phoneVocab.GetIdx(fields[3]);
		words[word_id].Initial = word_initial;
		words[word_id].Final = word_final;
    }
	file.close();
}