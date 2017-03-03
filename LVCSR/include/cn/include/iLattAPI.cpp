#include "iLattAPI.h"

LatParam::LatParam()
{
	splitMultiwords = false;
	skipTune = true;
	vocabFile[0] = 0;
	multiwordDictFile[0] = 0;
	strcpy(multiChar, "_");
	
	fPosteriorPrune = 0;   // 0.01;
	fPosteriorScale = 14;
	fHTKScale = 9;
	fHTKWdPenalty = -10;
	fHTKLogBase = 2.7182818284590452354;
	nbestDecode = 0;
	nbestViterbi= 0;
	nbestOutDir[0] = '\0';
}

iLatAPI::iLatAPI(LatParam& param):m_latParam(param)
{
	setlocale(LC_CTYPE, LANGUAGE);

	/*
	* Use multiword vocab in case we need it for -multiwords processing
	*/
	if(m_latParam.splitMultiwords)
		vocab = new MultiwordVocab(m_latParam.multiChar);
	else
		vocab = new Vocab;

// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
	m_latParam.addBoundary = param.addBoundary;
#endif
	/*
	* Read predefined vocabulary
	* (required by -limit-vocab and useful with -unk)
	*/
	if (m_latParam.vocabFile[0]) {
		File file(m_latParam.vocabFile, "r");
		vocab->read(file);
	}
	noiseVocab = new SubVocab(*vocab);
	ignoreVocab = new SubVocab(*vocab);
	if (m_latParam.multiwordDictFile[0]) {
		if( access(m_latParam.multiwordDictFile,0)!=0 )
			printf("Saus file %s is not existent.\n", m_latParam.multiwordDictFile);
		else {
			File file(m_latParam.multiwordDictFile, "r");
			Lattice::readMultiwordDict(file, multiwordDict);
		}
	}
	htkparms = new HTKHeader(HTK_undef_float, HTK_undef_float, HTK_undef_float,
						  HTK_undef_float, HTK_undef_float, HTK_undef_float,
						  HTK_undef_float, HTK_undef_float, HTK_undef_float,
						  HTK_undef_float, HTK_undef_float, HTK_undef_float,
						  HTK_undef_float, HTK_undef_float, HTK_undef_float);
	htkparms->amscale = m_latParam.fPosteriorScale;
	htkparms->acscale = 1.0;
	htkparms->lmscale = m_latParam.fHTKScale;
	htkparms->wdpenalty = m_latParam.fHTKWdPenalty;
	htkparms->logbase = m_latParam.fHTKLogBase;
}


iLatAPI::~iLatAPI(){
	delete vocab;
	delete ignoreVocab;
	delete noiseVocab;
	delete htkparms;
	// delete multiwordDict;
	//  [4/27/2010 JGao]
	//memory leak for multiwordDict
    LHashIter<const char*, Array< Array<char*>* > > iter(multiwordDict);
   	Array< Array<char *> *> *expansion;
    const char* word;
	Array<char *> * thisArray=NULL;

    while (expansion = iter.next(word)) 
	{
		for (int i=0;i<expansion->size();i++)
		{
			thisArray=(*expansion)[i];
			for (int j=0;j<thisArray->size();j++)
			{
				if ((*thisArray)[j]!=NULL)
				{
					free((*thisArray)[j]);
				}
			}
			//thisArray->clear();
		}
		//expansion->clear();
    }	
	multiwordDict.clear();
}
extern char *getbasename(char *filename);
bool iLatAPI::GenCN(char* inLat, char* writeMesh, HTKLattice *hLat, HTKHeader2 *hHeader)
{
	char *tempid = (char*)idFromFilename(inLat);
	//printf("idFromFilename [%s] ok\n", inLat);
	Lattice lat(*vocab, tempid, *ignoreVocab);
	//printf("Lattice [%s] ok\n", tempid);
	if (tempid) free(tempid);
	lat.setSkipTune(m_latParam.skipTune);
	//printf("setSkipTune ok\n");
	
	//htkparms->amscale = m_latParam.fPosteriorScale;
	//htkparms->amscale = m_latParam.fPosteriorScale;
	Boolean status;
	if (hLat)
		status = lat.loadFromHTK(hLat, hHeader, htkparms, 1);
	else
	{
		/*FILE *pFile = fopen(inLat, "rb");
		FILE *wFile = fopen("d:\\test.txt", "wb");
		if(pFile)
		{
			char szBuf[500];
			while(!feof(pFile))
			{
				int nbype = fread(szBuf, 1, 500, pFile);
				fwrite(szBuf, 1, nbype, wFile);
				//fscanf(pFile, "%s\n", szBuf);
				//printf("%s\n",szBuf);
			}
		}
		fclose(pFile);
		fclose(wFile);*/
		File file(inLat, "rt");  // bug 090514 if "r"
		status = lat.readHTK(file, htkparms, 1);
	}
	if (!status) {
		cerr << "error reading " << inLat << endl;
		return false;
	}
	//printf("loadFromHTK/readHTK ok\n");

	if (m_latParam.splitMultiwords) {
		lat.splitHTKMultiwordNodes((MultiwordVocab &) (*vocab), multiwordDict);
	}
	//printf("splitHTKMultiwordNodes ok\n");

	//return true;
	if (m_latParam.fPosteriorPrune > 0 )
	{
		if (!lat.prunePosteriors(m_latParam.fPosteriorPrune, m_latParam.fPosteriorScale,
			0, 0, 0))
		{
			cerr << "WARNING: posterior pruning of lattice " << inLat
				<< " failed\n";
			return false;
		} 
	}
	//printf("prunePosteriors ok\n");

	int nbestDecode = m_latParam.nbestDecode;
	char *multiChar = m_latParam.multiChar;
	int useMultiwordLM = 0, nbestMaxHyps = 0, nbestDuplicates = 0;
	if (nbestDecode > 0) {
		// output top N hyps
		//static NBestOptions nbestOut("nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest","nbest");
		static NBestOptions nbestOut(m_latParam.nbestOutDir,0,0,0,0,0,0,0,0,0,0,0,0,0);
		(writeMesh);
		nbestOut.openFiles(getbasename(writeMesh));
		if (m_latParam.nbestViterbi) {
			lat.computeNBestViterbi(nbestDecode, nbestOut, *noiseVocab,
				useMultiwordLM ? multiChar : 0);
		} else {
			lat.computeNBest(nbestDecode, nbestOut, *noiseVocab,
				useMultiwordLM ? multiChar : 0,
				nbestMaxHyps, nbestDuplicates);
		}
		nbestOut.closeFiles();
		return true;
	}
	if (writeMesh) {
		VocabDistance *wordDistance = 0;
		/*
		* Use word distance constrained by hidden-vocabulary membership
		* if specified
		*/
		WordMesh mesh(*vocab, lat.getName(), wordDistance);
		//printf("WordMesh [%s] ok\n", lat.getName());
		/*
		* Preserve acoustic information in word mesh if requested,
		* or if needed for CTM generation.
		*/
// mod for eng [2/15/2011 zhzhang]
#ifdef USE_ENG
		mesh.addBoundary = m_latParam.addBoundary;
#endif
		lat.alignLattice(mesh, *noiseVocab, m_latParam.fPosteriorScale, 1);
		//printf("alignLattice ok\n");
		bool bWriteOK = false;
		if (writeMesh) {
			File file(writeMesh, "w");
			bWriteOK =  mesh.write(file);
			file.close();   // bug! 070718 syq add
		}
		//printf("WordMesh write [%s] ok\n", writeMesh);
		if(wordDistance)
			delete wordDistance;
		if (!bWriteOK) return false;
	}
	return true;
}
