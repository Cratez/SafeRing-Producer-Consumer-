#include "SafeRing.h"
#include "TimeoutException.h"
#include <exception>
#include <Windows.h>
#include <iostream>

//CTOR
SafeRing::SafeRing(int capacity) :
	mCapacity(capacity),
	mSize(0),
	mHead(0),
	mTail(0)
{
	//declare buffer, this may throw if too large
	try {
		mBuffer = new int[capacity];
	}
	catch (std::bad_alloc& ba) {
		//logging just for tracability
		std::wcout << "Bad allocation. Please try a smaller capacity" << std::endl;

		//throwing back the catch
		throw ba;
	}

	//Create the ManualResetEvents and mutex locks
	mhMutex = CreateMutex(
		NULL,				// default security attributes
		FALSE,				// initially not owned
		NULL);				// unnamed mutex

	mhHasItemEvent = CreateEvent(
		NULL,				// default security attributes
		TRUE,				// manual reset event
		FALSE,				// initital state
		TEXT("ProduceEvent")// event name
	);

	mhHasCapacityEvent = CreateEvent(
		NULL,				// default security attributes 
		TRUE,				// manual reset event
		TRUE,				// initital state
		TEXT("ConsumeEvent")// event name
	);

	//arrays for WaitForMultipleObjects, to avoid repeated instantiation and copying
	InsertHandles[0] = mhMutex; InsertHandles[1] = mhHasCapacityEvent;
	RemoveHandles[0] = mhMutex; InsertHandles[1] = mhHasItemEvent;
}

//DCTOR
SafeRing::~SafeRing()
{
	delete[] mBuffer;
	closeHandles();
}

/*=======================================================================================
SafeRing::Insert
Params:
	int value:	The value to be inserted into the data structure
	int msTimeout: Optional timeout for the lock wait. Will ignore if negative
Purpose:
	This is a thread safe method for inserting values into the data structure.
*///=====================================================================================
void SafeRing::Insert(int value, int msTimeout)
{
	DEBUGLOG(std::wcout << L"Thread waiting on locks, ID: " << GetCurrentThreadId() << std::endl);
	DWORD result = WaitForMultipleObjects(
		2,
		InsertHandles,
		TRUE,
		msTimeout >= 0 ? msTimeout : INFINITE
	);

	switch (result) 
	{
	case WAIT_OBJECT_0: //we acquired the lock and had capacity
		DEBUGLOG(std::wcout << L"Thread acquired locks, ID: " << GetCurrentThreadId() << std::endl);

		mBuffer[mTail] = value;
		mTail = (mTail + 1) % mCapacity;
		mSize++;

		std::wcout << L"Inserted " << value << std::endl;

		SetEvent(mhHasItemEvent);

		if (mSize == mCapacity) {
			DEBUGLOG(std::wcout << L"**DEBUG: Resetting capacity event. Size/Capacity: " << mSize << L"/" << mCapacity << std::endl);
			ResetEvent(mhHasCapacityEvent);
		}

		break;
		//we timed out
	case WAIT_TIMEOUT:
		throw TimeoutException(msTimeout);
		break;
	default:
		std::wcout << L"Unkown result occured in SafeRing::Insert from WaitForMultipleObjects" << std::endl;
		break;
	}

	DEBUGLOG(std::wcout << L"Thread releasing locks, ID: " << GetCurrentThreadId() << std::endl);
	if (ReleaseMutex(mhMutex) == FALSE) {
		std::wcout << L"Failed to release Mutex in SafeRing::Insert" << std::endl;
	}

}

/*=======================================================================================
SafeRing::Remove
Params:
	int msTimeout: Optional timeout for the lock wait. Will ignore if negative
Purpose:
	This is a thread safe method for removing values from the array (SafeRing) data structure.
*///=====================================================================================
int SafeRing::Remove(int msTimeout)
{
	DEBUGLOG(std::wcout << L"Thread waiting on locks, ID: " << GetCurrentThreadId() << std::endl);
	DWORD result = WaitForMultipleObjects(
		2,
		RemoveHandles,
		TRUE,
		msTimeout >= 0 ? msTimeout : INFINITE
	);
	DEBUGLOG(std::wcout << L"Thread acquired locks, ID: " << GetCurrentThreadId() << std::endl);

	if (result == WAIT_TIMEOUT) {
		//throw exception, failed to obtain a lock
		throw TimeoutException(msTimeout);
	}

	//this should probably be reference if other than int
	int i = mBuffer[mHead];
	mHead = (mHead + 1) % mCapacity;
	mSize--;

	//log the removal
	std::wcout << L"Removed " << i << std::endl;

	SetEvent(mhHasCapacityEvent);

	if (mSize == 0) {
		DEBUGLOG(std::wcout << L"Resetting HasItems Event: Thread Id:" << GetCurrentThreadId() << std::endl);
		ResetEvent(mhHasItemEvent);
	}

	DEBUGLOG(std::wcout << L"Thread releasing locks, ID: " << GetCurrentThreadId() << std::endl);
	ReleaseMutex(mhMutex);

	return i;
}

/*=======================================================================================
SafeRing::Count
Purpose:
	Thread safe method to get the number of elements in the data structure
*///=====================================================================================
int SafeRing::Count()
{
	DEBUGLOG(std::wcout << L"Thread waiting on locks, ID: " << GetCurrentThreadId() << std::endl);
	DWORD dwWaitResult = WaitForSingleObject(
		mhMutex,		// handle to mutex
		INFINITE);  // no time-out interval
	DEBUGLOG(std::wcout << L"Thread acquired locks, ID: " << GetCurrentThreadId() << std::endl);

	int i = mSize;

	DEBUGLOG(std::wcout << L"Thread releasing locks, ID: " << GetCurrentThreadId() << std::endl);
	if (!ReleaseMutex(mhMutex)) {
		std::wcout << L"Failed to release mutex lock " << GetLastError() << std::endl;
		//throw..?
	}

	return i;
}

//private close handles method
void SafeRing::closeHandles()
{
	CloseHandle(mhHasCapacityEvent);
	CloseHandle(mhHasItemEvent);
	CloseHandle(mhMutex);
}
