#include "Producer.h"
#include "TimeoutException.h"
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
		int value = (rand() % 1000) + 1;
		DEBUGLOG(std::wcout << L"Thread produced " << value << L" Thread ID: " << GetCurrentThreadId() << std::endl);
		try {
			mSafeRing.Insert(value,mTimeout);
			Sleep(value);
		}
		catch (TimeoutException&) {
			std::wcout << L"Timeout Exception Occured! Failed to insert " << value << L" Thread ID: " << GetCurrentThreadId() << std::endl;
		}
	}

	//set the event to let main thread know we are done.
	SetEvent(mhDoneProducingEvent);

	DEBUGLOG(std::wcout << L"Thread finished producing, Thread ID: " << GetCurrentThreadId() << std::endl);
}

HANDLE Producer::FinishedEvent() const
{
	return mhDoneProducingEvent;
}

