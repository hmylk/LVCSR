#ifndef _LATERALLINK_H_
#define _LATERALLINK_H_

#include "LVCSR_API.h"
#include <Windows.h>

extern LogFile *ApiLog;
extern bool bCallBackSorted;

class LateralLink   //for real taskline and resultline
{
	struct Node
	{
		Task *info;  
		Node *next;
		Node()
		{
			info = NULL;
			next = NULL;
		};
	};
	Node *start, *last;   // �����յ�
	int  sentFileIdx, number;   // ��һ�����, ��Ծ����
	CRITICAL_SECTION CS_Link;
	int nMaxNum, nTotalNum, nKeptNum;
	float fKeptRate;
public:
	LateralLink(int initNum = 0)
	{
		start = NULL, last = NULL;
		sentFileIdx = initNum, number = 0;
#ifndef WIN32
		int ret=pthread_mutex_init(&CS_Link,NULL);
		if(ret==EBUSY)
			ApiLog->logoutf2("InitializeCriticalSection for CS_Link,EBUSY,return %d \n",ret );
		else if(ret==EINVAL)
			ApiLog->logoutf2("InitializeCriticalSection for CS_Link,EINVAL,return %d \n",ret );
		else if(ret==EAGAIN)
			ApiLog->logoutf2("InitializeCriticalSection for CS_Link,EAGAIN,return %d \n",ret );
		else if(ret==ENOMEM)
			ApiLog->logoutf2("InitializeCriticalSection for CS_Link,ENOMEM,return %d \n",ret );
		else if(ret==EPERM)
			ApiLog->logoutf2("InitializeCriticalSection for CS_Link,EPERM,return %d \n",ret );
		else
			ApiLog->logoutf2("InitializeCriticalSection for CS_Link,return %d \n",ret );
#else
		InitializeCriticalSection(&CS_Link);
#endif

#ifndef WIN32
		//		pthread_mutexattr_destroy(&mutexattr1);  // destroy the attribute
#endif
		nMaxNum = -1, fKeptRate = 1.0, nTotalNum = 0, nKeptNum = 0;
	}
	~LateralLink()
	{
		if (nTotalNum > 0) 
			ApiLog->logoutf2("Total Income Tasks %d, Accept %d, Drop Rate is %f\n", nTotalNum, nKeptNum, 100.0 * (1.0 - float(nKeptNum)/float(nTotalNum)));
		DeleteCriticalSection(&CS_Link);
	}
	// ����������ҪΪ�������񻺴�
	int GetATask(Task *(& info))   // ����Ƿ���������, ȡ�����ÿ�
	{
		int ret = -1;
		EnterCriticalSection(&CS_Link);   // �����ٽ���
		if (start)
		{
			ret = 0;
			info = start->info;   // ��ȡ����ÿ�ζ�������ĵ�һ�����
			Node *temp = start;  
			if (temp == last) last = NULL;   // bug!  �������һ����
			start = start->next;
			delete temp;         //ɾ���Ѿ�ȡ�ߵĽ��
			--number;
		}
		LeaveCriticalSection(&CS_Link);   // �뿪�ٽ���
		return ret;
	}
	int AddATask(Task *info)   // ����/��������
	{
		int ret = -1;
		EnterCriticalSection(&CS_Link);   // �����ٽ���
		if (last && last->next == NULL || last == NULL)
		{
			nTotalNum++;
			if ((nMaxNum < 0 || number + 1 <= nMaxNum) &&
				(fKeptRate < 0 || fKeptRate >= 1.0 || !nTotalNum && fKeptRate || nTotalNum && float(nKeptNum+1)/float(nTotalNum) <= fKeptRate))
			{
				ret = 0;
				Node *temp = new Node;
				temp->info = info;
				temp->next = NULL;
				if (last != NULL) last->next = temp;
				else start = temp;   // bug!
				last = temp;
				++number;
				nKeptNum++;

			}
			if (nTotalNum % 1000)
				ApiLog->logoutf2("Income Tasks %d, Accept %d, Drop Rate is %f\n", nTotalNum, nKeptNum, 100.0 * (1.0 - float(nKeptNum)/float(nTotalNum)));
		}
		LeaveCriticalSection(&CS_Link);   // �뿪�ٽ���
		return ret;
	}
	bool DelAllTask(int SessionId = -1)
	{
		bool ret = true;
		EnterCriticalSection(&CS_Link);   // �����ٽ���
		Node *temp = start, *templast = start;
		while (temp)
		{
			if (SessionId<0 || SessionId == temp->info->sessionId)
			{
				if (temp->info) free(temp->info);
				Node *tempnext = temp->next;
				if (temp == start) start = tempnext;
				else templast->next = tempnext;
				if (temp == last) last = tempnext;
				delete temp;
				temp = tempnext;
				--number;
			}
			else templast = temp, temp = temp->next;
		}
		if (SessionId < 0)
		{
			number = 0;
			sentFileIdx = 0;   // bug! thanks to gaojie
		}
		LeaveCriticalSection(&CS_Link);   // �뿪�ٽ���
		return ret;
	}

	bool DelCurSessAllTask(int SessionId )
	{
		bool ret = true;
		EnterCriticalSection(&CS_Link);   // �����ٽ���
		Node *temp = start, *templast = start;
		while (temp)
		{
			if (SessionId == temp->info->sessionId)
			{
				if (temp->info) free(temp->info);
				Node *tempnext = temp->next;
				if (temp == start) start = tempnext;
				else templast->next = tempnext;
				if (temp == last) last = tempnext;
				delete temp;
				temp = tempnext;
				--number;
			}
			else templast = temp, temp = temp->next;
		}
		LeaveCriticalSection(&CS_Link);   // �뿪�ٽ���
		return ret;
	}

	bool IsEmpty()
	{
		if (start == NULL && number == 0) return true;
		else return false;
	}


	// ������ҪΪʶ������������
	int InsertATask(Task*(&newTask), bool bGet = true, int ShouldSentFileIdx = -1, int TotalSavedFileIdx = -1)   // ����/������, 071221�����Ƿ�ȡ���ݵ�ѡ��
	{
		int ret = -1;
		if (ShouldSentFileIdx < 0) ShouldSentFileIdx = sentFileIdx;
		if (!bCallBackSorted)
			if (newTask) return 1;
			else return 0;
		EnterCriticalSection(&CS_Link);   // �����ٽ���
		// old˼�룺������ǡ��ȡ����ͳ��ȡ������������˳����룻
		//    ������������ֻ�治ȡ���ƣ�Ǳ��bug��һ�����飬��������ȡ��
		// ��Ϊ˳����룬�ж��Ƿ����ȡ������ҪΪ�˴���������⣨�൱������˳��ߵ���
		// 1��˳�����
		if (newTask)   // ˳�����
		{
			Node *former = NULL;   // bug!
			Node *temp = start;
			while (temp)
			{
				if(temp->info && temp->info->saveFileIdx < newTask->saveFileIdx)
				{
					former = temp, temp = temp->next;
				}
				else if (temp->info == NULL)
					temp = temp->next;
				else 
					break;
			}
			Node *newNode = new Node;
			newNode->info = newTask, newNode->next = temp;
			if (former) 
				former->next = newNode;
			else 
				start = newNode;
			++number;

		}
		// 2���ж��ܷ�ȡ��
		Node *temp = start;
		temp = start;
		if (start && start->info->saveFileIdx == sentFileIdx && start->info->saveFileIdx <= ShouldSentFileIdx)   // ֱ��ȡ���
		{
			int i = 0;
			while (temp)   //Ԥ����������Ҫ�ĸ���
			{
				if (temp->info && temp->info->saveFileIdx == sentFileIdx + i)   // �Ƽ���������
					++i, temp = temp->next;
				else if (temp->info == NULL)   // ������Ч���
					temp = temp->next;
				else break;
			}
			if (!bGet && TotalSavedFileIdx > 0 && TotalSavedFileIdx == sentFileIdx + i) bGet = true;   // �յ㴦��
			if (!bGet) { LeaveCriticalSection(&CS_Link); return ret; } 
			Task *retArray = i?(Task*)malloc(sizeof(Task)*i):NULL;
			for (temp = start, i = 0; temp;)
				if (temp->info && temp->info->saveFileIdx == sentFileIdx + i)
				{
					retArray[i] = *(temp->info), start = temp->next;
					free(temp->info);   // bug! 071008
					delete temp;   // bug!
					temp = start;
					i++;
				}
				else if (temp->info == NULL)
					temp = temp->next;
				else break;
				number -= i, sentFileIdx += i;
				ret = i, newTask = retArray;
		}
		LeaveCriticalSection(&CS_Link);   // �뿪�ٽ���
		return ret;
	}
	// ���ñ������ƣ�080410 syq add for 242's 80 task
	int SetMaxNum(int maxNum)
	{
		nMaxNum = maxNum;
		return 0;
	}
	int SetKeptRate(float keptRate)
	{
		if (fKeptRate <= 1)
		{
			fKeptRate = keptRate;
			return 0;
		}
		else
		{
			fKeptRate = 1.0;
			return -1;
		}
	}
	int GetCacheNum()
	{
		return number;
	}
};

#endif