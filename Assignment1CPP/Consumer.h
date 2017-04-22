#pragma once

#include "SafeRing.h"
#include <atomic>

class Consumer
{
public:
	Consumer(SafeRing& safeRing, int msTimeout = -1);
	~Consumer();
	void Start();
	void Stop();

private:
	const int mTimeout;
	SafeRing& mSafeRing;
	HANDLE mAbortEvent;
	std::atomic<bool> mAbort;
};

