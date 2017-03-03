
#ifndef _CYCLINK_H_
#define _CYCLINK_H_

#include <windows.h>
#include <stdio.h>
#define  PoolSize 10

class CycLine
{
	struct Cell
	{
		bool bFree;       // �Ƿ����, ���������/�����
		Task *info;       // could be within *info, serialNumber, sessionId...
		bool bFull;       // �Ƿ��п��õ�ʶ������/���
		HANDLE hthread;   // �洢�߳̾��, ���ڹر�
		Cell()
		{
			bFree = true;
			info = NULL;
			bFull = false;
			hthread = NULL;
		};
	};
	Cell *aCycLine;
	int  size;
	int  nextNumber;     // ��Ϊ��Դ, ��һ�����ܿ��еı��
	int  firstNumber;    // ��Ϊ�ֿ�, �׸�����洢��λ��
	int  lastNumber;     // �ɴ���һ�������λ��
	CRITICAL_SECTION CS_Cyc;
public:
	CycLine(int cSize = PoolSize)
	{
		aCycLine = new Cell[cSize];
		size = cSize;
		firstNumber = 0;
		nextNumber  = 0;
		lastNumber  = 0;
#ifndef WIN32
		pthread_mutexattr_t mutexattr;  // Mutex Attribute
		// Set the mutex as a recursive mutex
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
		// Create the mutex with the attributes set
#endif
		InitializeCriticalSection(&CS_Cyc);
#ifndef WIN32
		pthread_mutexattr_destroy(&mutexattr);  // destroy the attribute
#endif
	}
	~CycLine()
	{
		for (int i=0; i<size; ++i)
			if (aCycLine[i].info) free(aCycLine[i].info);
		if (aCycLine) delete []aCycLine;
		DeleteCriticalSection(&CS_Cyc);
	}
	int GetAFreeCell()
	{
		int ret = -1;
		EnterCriticalSection(&CS_Cyc);   // �����ٽ���
		int i=nextNumber;
		for (i=nextNumber; i<size; ++i)
		{
			if (aCycLine[i].bFree)
			{
				ret = i;
				break;   // ȥ��������break����bug������ʵ��ֻ�����һ�����������
			}
		}
		if (ret < 0)
		{
			for (i=0; i<nextNumber; ++i)
				if (aCycLine[i].bFree)
				{
					ret = i;
					break;
				}
		}
		if (ret >= 0)
		{
			aCycLine[ret].bFree = false;
			nextNumber = ret + 1;
			if (nextNumber >= size) nextNumber = 0;
		}
		LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
		return ret;
	}
	int SetAFreeCell(int number, bool tofree = true)
	{
		int ret = -1;
		if (number>=size || number<0) return ret;
		EnterCriticalSection(&CS_Cyc);   // �����ٽ���
		if ((aCycLine[number].bFree) ^ tofree)
		{
			aCycLine[number].bFree = tofree;
			ret = number;
		}
		LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
		return ret;
	}
	bool IsACellWork(int number)
	{
		if (number>=size || number<0) return false;
		return !(aCycLine[number].bFree);
	}
	//  [3/11/2014 Administrator]
	bool IsACellValid(int number)
	{
		if (number>=size || number<0) return false;
		return true;
	}
	bool IsIdle()
	{
		bool ret = true;
		EnterCriticalSection(&CS_Cyc);   // �����ٽ���
		for (int i=0; i<size; ++i)
			if (!(aCycLine[i].bFree))
			{
				ret = false;
				break;
			}
			LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
			return ret;
	}	
	int SetAClearCell(int number, bool toclear = true)
	{
		int ret = -1;
		if (number>=size || number<0) return ret;
		EnterCriticalSection(&CS_Cyc);   // �����ٽ���
		if ((aCycLine[number].bFull) ^ !toclear)
		{
			aCycLine[number].bFull = !toclear;
			ret = number;
		}
		LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
		return ret;
	}
	int SetAHandle(int number, HANDLE hnewthread)
	{
		int ret = -1;
		if (number>=size || number<0) return ret;
		EnterCriticalSection(&CS_Cyc);   // �����ٽ���
		if ((aCycLine[number].hthread) != NULL)
		{
#ifdef WIN32
			//			WaitForSingleObject(aCycLine[number].hthread, INFINITE);
			CloseHandle(aCycLine[number].hthread);
#endif
		}
		aCycLine[number].hthread = hnewthread;
		ret = number;
		LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
		return ret;
	}
	int SetATaskInfo(int number, int sessionId)
	{
		int ret = -1;
		if (number>=size || number<0) return ret;
		EnterCriticalSection(&CS_Cyc);   // �����ٽ���
		if (!(aCycLine[number].bFree))
		{
			if (aCycLine[number].info == NULL)
				aCycLine[number].info = (Task*)malloc(sizeof(Task)*1);
			aCycLine[number].info->sessionId = sessionId;
			ret = number;
		}
		LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
		return ret;
	}
	int WaitForASessionStop(int sessionId)
	{
		int ret = -1;
		for (int i=0; i<size; ++i)
		{
			if (aCycLine[i].bFree) continue;
			while (aCycLine[i].info == NULL) Sleep(10);   // �ȴ�SetATaskInfo��ֵ
			EnterCriticalSection(&CS_Cyc);   // �����ٽ���
			if (aCycLine[i].info->sessionId != sessionId) 
			{
				LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
				continue;
			}
			LeaveCriticalSection(&CS_Cyc);   // �뿪�ٽ���
			printf("Waiting for Cell %d to stop ", i + 1);
			while(!(aCycLine[i].bFree))
			{
				printf(".");
				if (aCycLine[i].info->sessionId != sessionId) break;   // ��ֹ���ı�
				Sleep(500);
			}
			printf(" Ok!\n");
			ret = i + 1;
		}
		return ret;
	}
};
#endif