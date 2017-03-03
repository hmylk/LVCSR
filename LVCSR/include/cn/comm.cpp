#include "comm.h"

int getlineH(istream& istr, string& line) 
{
	static char temp[2048];
	
	if(! istr.getline(temp, 2048))
		return 0;
	line = temp;
	return 1;
}


int myfgets( FILE *fp, int count, char *line)
{
	if(fgets(line, count, fp)==NULL)
		return 0;

	if(line[strlen(line) - 1] == '\n') 
		line[strlen(line) - 1] = '\0';
	return 1;
}

string SkipTune(string strTune,char &tonename)//  [3/8/2011 zhzhang]
{
	int nLen = strTune.length();
	char last = strTune[nLen - 1];
	if(last <='5'&& last >= '1')
	{
		tonename=last;
		return strTune.substr(0, nLen - 1);
	}
	else
	{
		return strTune;
	}
}

LnProb Ln(double prob)
{
	if(prob < 0.0001)
		prob = 0.0001;
	LnProb lnProb = log(prob);
	return lnProb;

}


