#pragma once
#include <Windows.h>

#define DEBUG

//this is to simplify the debug lines
#ifdef  DEBUG
#define DEBUGLOG(x) x
#else
#define DEBUGLOG(x)
#endif //  DEBUG

class SafeRing{
public:
	SafeRing(int capacity);
	~SafeRing();
	int Count();
	void Insert(int value, int msTimeout = -1);
	int Remove(int msTimeout = -1);

private:
	HANDLE InsertHandles[2];
	HANDLE RemoveHandles[2];

	HANDLE mhHasCapacityEvent;
	HANDLE mhHasItemEvent;
	HANDLE mhMutex;
	int* mBuffer;
	int mCapacity;
	int mSize;
	int mHead;
	int mTail;

	void closeHandles();
};