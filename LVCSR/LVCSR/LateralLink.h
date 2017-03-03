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
	Node *start, *last;   // 起点和终点
	int  sentFileIdx, number;   // 下一个编号, 活跃个数
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
	// 以下两个主要为解码任务缓存
	int GetATask(Task *(& info))   // 检查是否有新任务, 取出并置空
	{
		int ret = -1;
		EnterCriticalSection(&CS_Link);   // 进入临界区
		if (start)
		{
			ret = 0;
			info = start->info;   // 获取任务，每次都从链表的第一个结点
			Node *temp = start;  
			if (temp == last) last = NULL;   // bug!  链表最后一个点
			start = start->next;
			delete temp;         //删除已经取走的结点
			--number;
		}
		LeaveCriticalSection(&CS_Link);   // 离开临界区
		return ret;
	}
	int AddATask(Task *info)   // 加载/缓冲任务
	{
		int ret = -1;
		EnterCriticalSection(&CS_Link);   // 进入临界区
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
		LeaveCriticalSection(&CS_Link);   // 离开临界区
		return ret;
	}
	bool DelAllTask(int SessionId = -1)
	{
		bool ret = true;
		EnterCriticalSection(&CS_Link);   // 进入临界区
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
		LeaveCriticalSection(&CS_Link);   // 离开临界区
		return ret;
	}

	bool DelCurSessAllTask(int SessionId )
	{
		bool ret = true;
		EnterCriticalSection(&CS_Link);   // 进入临界区
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
		LeaveCriticalSection(&CS_Link);   // 离开临界区
		return ret;
	}

	bool IsEmpty()
	{
		if (start == NULL && number == 0) return true;
		else return false;
	}


	// 以下主要为识别结果缓存整序
	int InsertATask(Task*(&newTask), bool bGet = true, int ShouldSentFileIdx = -1, int TotalSavedFileIdx = -1)   // 加载/缓冲结果, 071221增加是否取数据的选项
	{
		int ret = -1;
		if (ShouldSentFileIdx < 0) ShouldSentFileIdx = sentFileIdx;
		if (!bCallBackSorted)
			if (newTask) return 1;
			else return 0;
		EnterCriticalSection(&CS_Link);   // 进入临界区
		// old思想：当存入恰当取出则统计取出总数，否则顺序插入；
		//    引入特殊情形只存不取机制，潜在bug：一旦耽搁，将永不能取出
		// 改为顺序插入，判断是否合适取出，主要为了代码简洁易理解（相当于两块顺序颠倒）
		// 1，顺序插入
		if (newTask)   // 顺序插入
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
		// 2，判断能否取出
		Node *temp = start;
		temp = start;
		if (start && start->info->saveFileIdx == sentFileIdx && start->info->saveFileIdx <= ShouldSentFileIdx)   // 直接取结果
		{
			int i = 0;
			while (temp)   //预计算满足需要的个数
			{
				if (temp->info && temp->info->saveFileIdx == sentFileIdx + i)   // 推荐遍历代码
					++i, temp = temp->next;
				else if (temp->info == NULL)   // 跳过无效结点
					temp = temp->next;
				else break;
			}
			if (!bGet && TotalSavedFileIdx > 0 && TotalSavedFileIdx == sentFileIdx + i) bGet = true;   // 终点处理
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
		LeaveCriticalSection(&CS_Link);   // 离开临界区
		return ret;
	}
	// 设置保留限制！080410 syq add for 242's 80 task
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