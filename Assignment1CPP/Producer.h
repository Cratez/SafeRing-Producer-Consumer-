#pragma once
#include "SafeRing.h"
#include <Windows.h>

//Producer class. Produces X items until done.
class Producer
{
public:
	Producer(int itemsToProduce, SafeRing & safeRing, int timeout = -1);
	void Start();
	HANDLE FinishedEvent() const;

private:
	const int mItemsToProduce;
	const int mTimeout;
	int mItemsProduced;
	SafeRing& mSafeRing;

	HANDLE mhDoneProducingEvent;
};

