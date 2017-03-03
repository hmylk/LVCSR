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
	int   CleanData()                // 清除数据, 和ResetEnv等效
	{
		return ResetEnv();
	}
	int   PermitData()               // 允许接受数据
	{
		IsPermitReceiveData = true;
		return 0;
	}
	int   RefuseData()               // 拒绝接受数据
	{
		IsPermitReceiveData = false;
		StopRecording();
		return 0;
	}
	int   ResetEnv();                // 初始化环境
	int  StopRecording();            //本路语音段送完

	CreateFeature* aCreateFeature;   
	CreateFeature* aCreateFeature2;

	LateralLink *ResultPool;       // 结果缓冲、排序循环队列
	FeSegInfo*  pResultLink;       //保存传入的链表头指针，并填充识别信息后，返回结果
	vector<Task*>  vecSortedResult;//缓存已经排好序的结果结构

	char  sessWavId[256];          //当前session的wav id号
	DWORD SentFileIdx;
	DWORD SavedFileIdx;            //记录task个数
	DWORD SegIdxInFile;            //在线模式下，记录当前包处于一句话的什么位置：0起始包，>0中间包，-1结束包，-2表示整包
	bool  bEndRecord;			
	bool bTaskSendToDecoder;       //在线模式下记录标识一句话的第一包是否送入解码
protected:
	LateralLink *myFeaturePools;   //本路的任务队列
	bool  bSaveData;               // 是否保存数据
	bool  bSaveFeature;            // 是否保存特征
	bool  IsPermitReceiveData;     // 是否接受数据
	int   SampleRate;
	DWORD SessionId;               // 路标识符
	char  systemDir[256];          // 系统路径			
	
	int ConvertStrToInt(const char *optName, char *valStr);
private:
};

#endif