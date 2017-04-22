#pragma once
#include <Windows.h>

//#define DEBUG
#define IOLOCKING

#if defined(DEBUG) || defined(IOLOCKING)
extern HANDLE ghIOLock;
#endif

//this is to simplify the debug lines
#ifdef  DEBUG
#define DEBUGLOG(x){						\
WaitForSingleObject(ghIOLock,INFINITE);	\
x;											\
ReleaseMutex(ghIOLock);					\
}
#else
#define DEBUGLOG(x)
#endif //  DEBUG

//io syncing for appearance
#ifdef IOLOCKING
#define IOSYNC(x) {						\
WaitForSingleObject(ghIOLock,INFINITE);	\
x;										\
ReleaseMutex(ghIOLock);					\
}
#else
#define IOSYNC(x)
#endif

/*=======================================================================================
SafeRing::Count
Purpose:
Thread safe method to get the number of elements in the data structure
*///=====================================================================================
class SafeRing{
public:
	SafeRing(int capacity);
	~SafeRing();
	int Count();
	void Insert(int value, int msTimeout = -1);
	int Remove(int msTimeout = -1 , HANDLE abortEvent = NULL);

private:
	HANDLE mhHasCapacity;
	HANDLE mhHasItem;
	HANDLE mhInsertMutex;
	HANDLE mhRemoveMutex;
	int* mBuffer;
	int mCapacity;
	int mHead;
	int mTail;

	void closeHandles();
};