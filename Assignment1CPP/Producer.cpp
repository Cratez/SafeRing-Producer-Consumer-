/*
Author:	John Harrison
File:	producer.cpp
Class:	CST352 Operating Systems

Note:	cpp file for producer class
*/
#include "Producer.h"
#include "TimeoutException.h"
#include "utils.h"
#include <iostream>
#include <Windows.h>

Producer::Producer(int itemsToProduce, SafeRing & safeRing, int timeout) :
	mItemsToProduce(itemsToProduce),
	mItemsProduced(0),
	mSafeRing(safeRing),
	mTimeout(timeout)
{
	mhDoneProducingEvent = CreateEvent(
		NULL,		// Default security attributes
		TRUE,		// manual reset event
		FALSE,		// initial state
		NULL		// event name (unnamed)
	);
}
Producer::~Producer() { CloseHandle(mhDoneProducingEvent); }

/// <summary>
/// Starts this instance.
/// </summary>
void Producer::Start()
{
	DEBUGLOG(std::wcout << L"Thread starting to produce, Thread ID: " << GetCurrentThreadId() << std::endl);

	// The document states to "produce" X items, 
	// so I count a production on each loop. If they fail to insert, work was still done and we move on to next item.
	while (mItemsProduced++ < mItemsToProduce) {
		int value = get_rand_uniform<1,1000>();
		DEBUGLOG(std::wcout << L"Thread produced " << value << L" Thread ID: " << GetCurrentThreadId() << std::endl);
		try {
			mSafeRing.Insert(value,mTimeout);		
		}
		catch (DWORD ex) {
			if (ex == WAIT_FAILED) {
				DEBUGLOG(std::wcout << L"**WAIT_FAILED caught on Insert, Thread ID: " << GetCurrentThreadId() << std::endl);
			}
			else if (ex == WAIT_TIMEOUT) {
				DEBUGLOG(std::wcout << L"**WAIT_TIMEOUT caught on Insert, Thread ID: " << GetCurrentThreadId() << std::endl);
			}
			else {
				//if we don't know the error, I want to let it go up. 
				DEBUGLOG(std::wcout << L"**UNKNOWN (DWORD) caught " << ex << L" Thread ID: " << GetCurrentThreadId() << std::endl);
				throw ex;
			}
		}
		catch (TimeoutException& ex) {
			IOSYNC(std::wcout << L"Inserter timed out after " << ex.timeout() << "ms" << std::endl);
			DEBUGLOG(std::wcout << L"**TimeoutException caught on Remove, Thread ID: " << GetCurrentThreadId() << std::endl);
		}
		//delay
		Sleep(value);
	}

	//set the event to let main thread know we are done.
	//this is completely unrequired since the more efficient method is simply to monitor the thread handles for completion.
	SetEvent(mhDoneProducingEvent);

	DEBUGLOG(std::wcout << L"Thread finished producing, Thread ID: " << GetCurrentThreadId() << std::endl);
}

/// <summary>
/// Finisheds the event.
/// This is entirely useless. Thread handle is more useful
/// </summary>
/// <returns>Handle to the finished event</returns>
HANDLE Producer::FinishedEvent() const
{
	return mhDoneProducingEvent;
}

