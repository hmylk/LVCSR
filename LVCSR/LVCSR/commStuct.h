#ifndef _COMMSTRUCT_H_
#define _COMMSTRUCT_H_

#ifdef WIN32
#include <Windows.h>
#endif
typedef enum
{
	FE_SEND_BEG=0,        //���͵�һ��
	FE_SEND_CUR,          //�����м�
	FE_SEND_END,          //���һ��
	FE_SEND_ALL           //һ����ȫ�����ͣ�����������
}FeSendStatus;

typedef enum
{
	FE_MONO=0,           //������
	FE_STEREO             //˫����
}FeChannelMode;

//��⵽��������
typedef enum               //speech type
{             
	RT_SILENCE = 0,        //����
	RT_RING,		         //���壨���ﲻ���ֲ��塢���壩
	RT_DTMF,		     //������
	RT_FAX,			     //����������һ�鲻̫��Ϥ���������ӿڣ�
	RT_VOICE=10,  	     //ֵΪ10���ϣ�����Ϊ����
	RT_OVERLAP		     //����
}FeSpeechType;

//��Ҫ��������ģʽ�£���ǰ�������ݰ����������ε�λ��
typedef enum
{        
	FE_SEG_START,        //����ð��������⵽���
	FE_SEG_CONT,        //�ð��������ڵ�ǰ�ε��м�
	FE_SEG_END,         //����ð��������⵽�յ�
	FE_SEG_ALL,         //��ʾ�ð�������������Ƭ�Σ�������ģʽ��
}FeDetectStatus;

//��������
typedef enum
{
	FE_NEU=0,            //����
	FE_HAPPY,            //��������
	FE_ANGRY            //��������
}FeEmotionStatus;



//�������ԣ����÷����룩
typedef  struct  
{ 
	short taskId;                   //¼��id
	short  *data_buf;               //¼�����ݵ�ַ
	int  dataLen;                   //���ݳ��� 
	FeChannelMode  nChannel;        //����
	FeSendStatus  send_status;         //����״̬
}FeSpeechAttri;

struct  FeSegInfo 
{
	int start_time, end_time;          //�����㣬�����¼����ʼλ�õľ���ʱ���
	int nGenderType;                //�Ա���� 1-male,2-female, 3-���, 4-�޷��ж�,
	int languageType;                //����
	int SpkId;                       //˵����ID
	FeDetectStatus detect_status;      //��ǰ�������ݰ����������ε�λ��
	FeEmotionStatus  emotion_status;  //��������ʶ��    	
	FeSpeechType speech_type;        //��������
	char *pText;                    //������ı����ݣ���vadʱ�洢DTMF���ִ���ʶ��//ʱ�ɴ洢ʶ�����ȣ�
	float energy;                   //����
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