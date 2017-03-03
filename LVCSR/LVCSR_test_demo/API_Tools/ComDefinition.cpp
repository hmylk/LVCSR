#include "ComDefinition.h"


void chomp(char *buf)
{
	char *pos=buf+strlen(buf)-1;
	while (pos>=buf && (*pos == '\r' || *pos == '\n' || *pos == ' ' || *pos == '\t' || *pos == '\"'))
		*(pos--) = 0;												// 后向
	pos=buf;
	while (*pos == '\r' || *pos == '\n' || *pos == ' ' || *pos == '\t' || *pos == '\"')
		++pos;														// 前向
	if(buf!=pos) memmove(buf, pos, strlen(pos)+1);
}

//change the file delimiters
int NormFileDelimters(char *filename)
{
	int i,n;
	char workName[256];
	n = (int)strlen(filename);
	strcpy(workName,filename);
#ifdef WIN32
	for (i=1; i < n; i++)  
		if (workName[i] == '/') 
			workName[i] = '\\';
#else
	for (i=1; i < n; i++)  
		if (workName[i] == '\\') 
			workName[i] = '/';
#endif
	strcpy(filename,workName);
	return 0;
}

int NormPathDelimters(char *filename)
{
	chomp(filename);
	int n;
	int i;
	char workName[256];
#ifndef WIN32
	char dirDelim='/';
#else
	char dirDelim='\\';
#endif    

	n = (int)strlen(filename);
	strcpy(workName,filename);
#ifdef WIN32
	for (i=1; i < n; i++)  
		if (workName[i] == '/') 
			workName[i] = '\\';
#else
	for (i=1; i < n; i++)  
		if (workName[i] == '\\') 
			workName[i] = '/';
#endif
	//如果没有，加上
	if (workName[n-1] != '/' &&workName[n-1] != '\\')
	{
#ifdef WIN32
		workName[n] = '\\';
		workName[n+1]= '\0';
#else
		workName[n] = '/';
		workName[n+1]= '\0';
#endif
	}
	//process duplicated delimiters here
	n = (int)strlen(workName);
	for(i=n-1;i>0;i--){
		if(workName[i] == dirDelim &&workName[i-1] == dirDelim){
			workName[i] = '\0';
		}
	}
	//process ./ here
	if(workName[0]=='.'&&workName[1]!='.')
	{
		char temp[512]="";
		strcpy(temp,workName+1);
		strcpy(workName,temp);
	}
	strcpy(filename,workName);
	return 0;
}

//
int ParseFormat(char *szFormat, SPEECHTYPE &format)
{
	if(!strcmp(szFormat,"8K_16BIT_PCM"))
	{
		format=TSR_RAW_16;
		return 0;
	}
	else if (!strcmp(szFormat,"8K_8BIT_ALAW"))
	{
		format =TSR_ALAW_PCM;
		return 0;
	}
	else if (!strcmp(szFormat,"16K_16BIT_PCM"))
	{
		format =TSR_16K_16;
		return 0;
	}
	else
	{
		return -1;
	}
}



