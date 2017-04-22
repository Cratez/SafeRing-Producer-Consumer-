#include "SafeRing.h"
#include "TimeoutException.h"
#include <exception>
#include <Windows.h>
#include <iostream>

#if defined(DEBUG) || defined(IOLOCKING)
HANDLE ghIOLock = CreateMutex(
	NULL,
	FALSE,
	NULL
);
#endif

//CTOR
SafeRing::SafeRing(int capacity) :
	mCapacity(capacity),
	mHead(0),
	mTail(0)
{
	//declare buffer, this may throw if too large
	try {
		mBuffer = new int[capacity];
	}
	catch (std::bad_alloc& ba) {
		//logging just for tracability
		IOSYNC(std::wcout << "Bad allocation. Please try a smaller capacity" << std::endl);

		//throwing back the catch
		throw ba;
	}

	//edit lock
	mhInsertMutex = CreateMutex(
		NULL,				// default security attributes
		FALSE,				// initially not owned
		NULL);				// unnamed mutex

	mhRemoveMutex = CreateMutex(
		NULL,				// default security attributes
		FALSE,				// initially not owned
		NULL);				// unnamed mutex

	mhHasItem = CreateSemaphore(
		NULL,				// default security attributes
		0,					// initial amount
		capacity,			//max amount
		TEXT("HasItem")
	);

	mhHasCapacity = CreateSemaphore(
		NULL,				// default security attributes
		capacity,			// initial amount
		capacity,			// max amount
		TEXT("HasCapacity")
	);
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
	HANDLE insertHandles[2] = { mhHasCapacity, mhInsertMutex };

	DEBUGLOG(std::wcout << L"Thread waiting on InsertMutex and HasCapacity, ID: " << GetCurrentThreadId() << std::endl);

	//wait for there to be capacity, and for write lock to be available
	DWORD result = WaitForMultipleObjects(
		2,
		insertHandles,
		TRUE,
		msTimeout >= 0 ? msTimeout : INFINITE
	);

	//act upon wait result
	switch (result)
	{
	case WAIT_OBJECT_0: //we acquired the lock and had capacity
		DEBUGLOG(std::wcout << L"Thread acquired locks, ID: " << GetCurrentThreadId() << std::endl);

		mBuffer[mTail] = value;
		mTail = (mTail + 1) % mCapacity;

		IOSYNC(std::wcout << L"Inserted " << value << std::endl);

		//add one to mhHasItem semaphore
		ReleaseSemaphore(
			mhHasItem,	// HasItem semaphore
			1,			// amount to add
			NULL		// Dont care about previous amount
		);
		break;

	case WAIT_TIMEOUT: //we timed out
		DEBUGLOG(std::wcout << L"Thread timed out, ID: " << GetCurrentThreadId() << std::endl);
		throw (DWORD)WAIT_TIMEOUT;
		break;

	default:
		IOSYNC(std::wcout << L"Unkown result occured in SafeRing::Insert from WaitForMultipleObjects" << std::endl);
		throw WAIT_FAILED;
		break;
	}

	//release the insert mutex back so another insert can occur
	DEBUGLOG(std::wcout << L"Thread releasing locks, ID: " << GetCurrentThreadId() << std::endl);
	if (ReleaseMutex(mhInsertMutex) == FALSE) {
		IOSYNC(std::wcout << L"Failed to release Mutex in SafeRing::Insert" << std::endl);
	}

}

/*=======================================================================================
SafeRing::Remove
Params:
	int msTimeout: Optional timeout for the lock wait. Will ignore if negative
Purpose:
	This is a thread safe method for removing values from the array (SafeRing) data structure.
*///=====================================================================================
int SafeRing::Remove(int msTimeout, HANDLE abortEvent)
{
	DWORD result;
	HANDLE handles[2] = { mhHasItem, abortEvent };
	int eventCount = 1;

	//count to pass into WaitForMultiple to declare two
	if (abortEvent != NULL) {
		eventCount = 2;
	}

	//wait for HasItem semaphore to be available, abort to occur, or timeout to occur

	DEBUGLOG(std::wcout << L"Thread waiting on hasItem, ID: " << GetCurrentThreadId() << std::endl);
	result = WaitForMultipleObjects(
		eventCount,
		handles,
		FALSE,
		msTimeout >= 0 ? msTimeout : INFINITE
	);

	switch (result) {
	case WAIT_OBJECT_0: //HasItem returned true
		//if we have gotten this far, we know we have an item to remove so can comfortably wait on remove mutex
		__try {
			DEBUGLOG(std::wcout << L"Thread waiting on RemoveMutex, ID: " << GetCurrentThreadId() << std::endl);
			result = WaitForSingleObject(
				mhRemoveMutex,	// the remove mutex lock.
				INFINITE		// the wait is infinite, but we are ensured there is an item to remove
			);
			DEBUGLOG(std::wcout << L"Thread acquired RemoveMutex, ID: " << GetCurrentThreadId() << std::endl);

			//this should probably be reference if other than int
			int i = mBuffer[mHead];
			mHead = (mHead + 1) % mCapacity;

			//log the removal
			IOSYNC(std::wcout << L"Removed " << i << std::endl);

			//add one to mhHasItem semaphore
			ReleaseSemaphore(
				mhHasCapacity,	// HasCapacity semaphore
				1,				// amount to add
				NULL			// Dont care about previous amount
			);

			//this will trigger the finally
			return i;
		}
		__finally {
			//release the mutex
			DEBUGLOG(std::wcout << L"Thread releasing RemoveMutex, ID: " << GetCurrentThreadId() << std::endl);
			if (ReleaseMutex(mhRemoveMutex) == FALSE) {
				IOSYNC(std::wcout << L"Failed to release Mutex in SafeRing::Remove" << std::endl);
			}
		}
		break;

	case WAIT_OBJECT_0 + 1: //abort event for this thread was triggered
		DEBUGLOG(std::wcout << L"Thread aborting, ID: " << GetCurrentThreadId() << std::endl);
		throw WAIT_ABANDONED;
		break;

	case WAIT_TIMEOUT: //thread timed out
		DEBUGLOG(std::wcout << L"Thread timed out on Remove, ID: " << GetCurrentThreadId() << std::endl);
		throw (DWORD)WAIT_TIMEOUT;
		break;

	default: //unkown issue
		std::wcout << L"Unexpected return from WaitForMultipleObjects ThreadId:" << GetCurrentThreadId()
			<< L" Value:" << result << std::endl;
		throw WAIT_FAILED;
		break;
	}
}

/*=======================================================================================
SafeRing::Count
Purpose:
	Thread safe method to get the number of elements in the data structure
*///=====================================================================================
int SafeRing::Count()
{
	HANDLE handles[2] = { mhInsertMutex, mhRemoveMutex };

	DEBUGLOG(std::wcout << L"Thread waiting on locks, ID: " << GetCurrentThreadId() << std::endl);
	WaitForMultipleObjects(
		2,			// handle count
		handles,	// handles
		TRUE,		// wait for all
		INFINITE	// don't timeout
	);

	DEBUGLOG(std::wcout << L"Thread acquired locks, ID: " << GetCurrentThreadId() << std::endl);

	//capture size
	int i;
	if (mTail >= mHead) {
		// mTail may be index 6 (unwritten), and mHead 1, which leaves index 1,2,3,4,5 as available
		// 6-1 = size of 5
		i = mTail - mHead;
	}
	else {
		// if head is index 6, and tail is index 0 (head points to a element, tail to next available empty space)
		// then capacity - head + tail gives us the size
		i = mCapacity - mHead + mTail;
	}

	DEBUGLOG(std::wcout << L"Thread releasing locks, ID: " << GetCurrentThreadId() << std::endl);
	if (!ReleaseMutex(mhInsertMutex) || !ReleaseMutex(mhRemoveMutex)) {
		IOSYNC(std::wcout << L"Failed to release mutex lock " << GetLastError() << std::endl);
	}

	return i;
}

// private close handles method
void SafeRing::closeHandles()
{
	DEBUGLOG(std::wcout << L"Closing handles" << std::endl);
	CloseHandle(mhInsertMutex);
	CloseHandle(mhRemoveMutex);
	CloseHandle(mhHasItem);
	CloseHandle(mhHasCapacity);
}
