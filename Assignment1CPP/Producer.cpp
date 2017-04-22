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

void Producer::Start()
{
	DEBUGLOG(std::wcout << L"Thread starting to produce, Thread ID: " << GetCurrentThreadId() << std::endl);

	//The document states to "produce" X items, so I count a production on each loop. If they fail to insert, work was still done.
	while (mItemsProduced++ < mItemsToProduce) {
		int value = GetRand();
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
		//delay
		Sleep(value);
	}

	//set the event to let main thread know we are done.
	SetEvent(mhDoneProducingEvent);

	DEBUGLOG(std::wcout << L"Thread finished producing, Thread ID: " << GetCurrentThreadId() << std::endl);
}

HANDLE Producer::FinishedEvent() const
{
	return mhDoneProducingEvent;
}

