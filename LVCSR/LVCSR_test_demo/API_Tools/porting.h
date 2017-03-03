#ifndef _PORTING_H_
#define _PORTING_H_


//Multi-thread
#ifndef WIN32
#include <pthread.h>
#include <sys/resource.h>
#include <dirent.h>
#include <unistd.h>
#include <semaphore.h>
#define CRITICAL_SECTION pthread_mutex_t
void InitializeCriticalSection(CRITICAL_SECTION*x);
#define EnterCriticalSection pthread_mutex_lock
#define LeaveCriticalSection pthread_mutex_unlock
#define DeleteCriticalSection pthread_mutex_destroy
#define  HANDLE int
#define Sleep(n) usleep(n*1000)
typedef unsigned long DWORD;
#define  _access access
#define stricmp strcasecmp

#endif


#endif
