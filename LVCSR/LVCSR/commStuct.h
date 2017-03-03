#ifndef _COMMSTRUCT_H_
#define _COMMSTRUCT_H_

#ifdef WIN32
#include <Windows.h>
#endif
typedef enum
{
	FE_SEND_BEG=0,        //发送第一包
	FE_SEND_CUR,          //发送中间
	FE_SEND_END,          //最后一包
	FE_SEND_ALL           //一次性全部发送，适用于离线
}FeSendStatus;

typedef enum
{
	FE_MONO=0,           //单声道
	FE_STEREO             //双声道
}FeChannelMode;

//检测到语音类型
typedef enum               //speech type
{             
	RT_SILENCE = 0,        //静音
	RT_RING,		         //彩铃（这里不区分彩铃、振铃）
	RT_DTMF,		     //按键音
	RT_FAX,			     //传真音（这一块不太熟悉，仅保留接口）
	RT_VOICE=10,  	     //值为10以上（含）为语音
	RT_OVERLAP		     //叠音
}FeSpeechType;

//主要用于在线模式下，当前发送数据包处于语音段的位置
typedef enum
{        
	FE_SEG_START,        //送入该包语音后检测到起点
	FE_SEG_CONT,        //该包语音处于当前段的中间
	FE_SEG_END,         //送入该包语音后检测到终点
	FE_SEG_ALL,         //表示该包语音包含整个片段（半在线模式）
}FeDetectStatus;

//情绪类型
typedef enum
{
	FE_NEU=0,            //中性
	FE_HAPPY,            //正面情绪
	FE_ANGRY            //负面情绪
}FeEmotionStatus;



//数据属性（调用方传入）
typedef  struct  
{ 
	short taskId;                   //录音id
	short  *data_buf;               //录音数据地址
	int  dataLen;                   //数据长度 
	FeChannelMode  nChannel;        //声道
	FeSendStatus  send_status;         //发送状态
}FeSpeechAttri;

struct  FeSegInfo 
{
	int start_time, end_time;          //采样点，相对于录音开始位置的绝对时间点
	int nGenderType;                //性别类别， 1-male,2-female, 3-混合, 4-无法判断,
	int languageType;                //语种
	int SpkId;                       //说话人ID
	FeDetectStatus detect_status;      //当前发送数据包处于语音段的位置
	FeEmotionStatus  emotion_status;  //用于情绪识别    	
	FeSpeechType speech_type;        //语音类型
	char *pText;                    //结果的文本内容，（vad时存储DTMF数字串，识别//时可存储识别结果等）
	float energy;                   //能量
	struct  FeSegInfo *next;
	FeSegInfo()
	{
		start_time=end_time=0;
		nGenderType = 0;
		languageType = 0;
		SpkId = 0;
		detect_status = FE_SEG_ALL;
		emotion_status = FE_NEU;
		pText = NULL;
		energy = 0;
		next = NULL;
	}
};
#endif