#ifndef _TSRBUFFER__H_
#define _TSRBUFFER__H_

#include "comm.h"
#include "LVCSR_API.h"
#include "LateralLink.h"
#include "CycLink.h"
#include "createfeature.h"
#include <vector>
using namespace std;

extern LogFile *ApiLog;
extern bool bCallBackSorted;

class CTSRBuffer
{
public:
	CTSRBuffer(char* sysDir, char* configFile, LogFile* logFile = ApiLog, int sessionId = 0);
	virtual ~CTSRBuffer();

	int   SetParam(char *paramName, char *paramValue);
	int   GetParam(char *paramName, char *paramValue);
	int   SendData(FeSegInfo* segInfo,FeSpeechAttri* dataInfo);
	int   CleanData()                // �������, ��ResetEnv��Ч
	{
		return ResetEnv();
	}
	int   PermitData()               // �����������
	{
		IsPermitReceiveData = true;
		return 0;
	}
	int   RefuseData()               // �ܾ���������
	{
		IsPermitReceiveData = false;
		StopRecording();
		return 0;
	}
	int   ResetEnv();                // ��ʼ������
	int  StopRecording();            //��·����������

	CreateFeature* aCreateFeature;   
	CreateFeature* aCreateFeature2;

	LateralLink *ResultPool;       // ������塢����ѭ������
	FeSegInfo*  pResultLink;       //���洫�������ͷָ�룬�����ʶ����Ϣ�󣬷��ؽ��
	vector<Task*>  vecSortedResult;//�����Ѿ��ź���Ľ���ṹ

	char  sessWavId[256];          //��ǰsession��wav id��
	DWORD SentFileIdx;
	DWORD SavedFileIdx;            //��¼task����
	DWORD SegIdxInFile;            //����ģʽ�£���¼��ǰ������һ�仰��ʲôλ�ã�0��ʼ����>0�м����-1��������-2��ʾ����
	bool  bEndRecord;			
	bool bTaskSendToDecoder;       //����ģʽ�¼�¼��ʶһ�仰�ĵ�һ���Ƿ��������
protected:
	LateralLink *myFeaturePools;   //��·���������
	bool  bSaveData;               // �Ƿ񱣴�����
	bool  bSaveFeature;            // �Ƿ񱣴�����
	bool  IsPermitReceiveData;     // �Ƿ��������
	int   SampleRate;
	DWORD SessionId;               // ·��ʶ��
	char  systemDir[256];          // ϵͳ·��			
	
	int ConvertStrToInt(const char *optName, char *valStr);
private:
};

#endif