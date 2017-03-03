#include "porting.h"

#ifndef WIN32
void InitializeCriticalSection(CRITICAL_SECTION*x)
{
	pthread_mutexattr_t mutexattr;  // Mutex Attribute
	// Set the mutex as a recursive mutex
	pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
	//pthread_mutex_init(x,&mutexattr);
	pthread_mutex_init(x,NULL);//1-18-2010, JGao for different version of Pthread
	// Create the mutex with the attributes set
	pthread_mutexattr_destroy(&mutexattr);  // destroy the attribute
}
#endif