/*
Author:	John Harrison
File:	consumer.cpp
Class:	CST352 Operating Systems

Note:	cpp file for consumer class
*/
#include "Consumer.h"
#include "utils.h"
#include "TimeoutException.h"
#include <iostream>


Consumer::Consumer(SafeRing& safeRing, int msTimeout) :
	mTimeout(msTimeout),
	mSafeRing(safeRing),
	mAbort(false)
{
	mAbort = false;
	mAbortEvent = CreateEvent(
		NULL,	// default security settings
		TRUE,	// manual reset
		FALSE,	// initial state
		NULL	// no name
	);
}

Consumer::~Consumer() {
	CloseHandle(mAbortEvent);
}

/// <summary>
/// Starts this instance.
/// </summary>
void Consumer::Start()
{
	DEBUGLOG(std::wcout << L"Thread starting to consume, Thread ID: " << GetCurrentThreadId() << std::endl);
	while (!mAbort) {
		int delayTime = get_rand_uniform<1, 1000>();
		try {
			int returnVal = mSafeRing.Remove(mTimeout, mAbortEvent);		
		}
		catch (DWORD ex) {
			if (ex == WAIT_FAILED) {
				DEBUGLOG(std::wcout << L"**WAIT_FAILED caught on Remove, Thread ID: " << GetCurrentThreadId() << std::endl);
			}
			else if (WAIT_ABANDONED) {
				DEBUGLOG(std::wcout << L"**WAIT_ABANDONED caught on Remove, Thread ID: " << GetCurrentThreadId() << std::endl);
			}
			else if (ex == WAIT_TIMEOUT) { //legacy, since pete made me change it to a class again.
				DEBUGLOG(std::wcout << L"**WAIT_TIMEOUT caught on Remove, Thread ID: " << GetCurrentThreadId() << std::endl);
			}
			else {
				//if we don't know the error, I want to let it go up. 
				DEBUGLOG(std::wcout << L"**UNKNOWN (DWORD) exception " << ex << L" Thread ID: " << GetCurrentThreadId() << std::endl);
				throw ex;
			}
		}
		catch (TimeoutException& ex) {
			IOSYNC(std::wcout << L"Consumer timed out after " << ex.timeout() << "ms" << std::endl);
			DEBUGLOG(std::wcout << L"**TimeoutException caught on Remove, Thread ID: " << GetCurrentThreadId() << std::endl);
		}
		//delay
		Sleep(delayTime);
	}
	DEBUGLOG(std::wcout << L"Thread finished consuming, Thread ID: " << GetCurrentThreadId() << std::endl);
}

/// <summary>
/// Stops this instance.
/// </summary>
void Consumer::Stop()
{
	DEBUGLOG(std::wcout << L"STOP issued" << std::endl);
	//set the flag so we exit the loop
	mAbort = true;

	//notify the abort event to exit any thread waiting on lock
	SetEvent(mAbortEvent);
}

