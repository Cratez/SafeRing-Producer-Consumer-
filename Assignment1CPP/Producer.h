/*
Author:	John Harrison
File:	producer.h
Class:	CST352 Operating Systems

Note:	Header file for producer class class
*/

#pragma once
#include "SafeRing.h"
#include <Windows.h>

//Producer class. Produces X items until done.
/// <summary>
/// Producer class. Produces items into data structure until complete
/// </summary>
class Producer
{
public:
	Producer(int itemsToProduce, SafeRing & safeRing, int timeout = -1);
	~Producer();
	void Start();
	HANDLE FinishedEvent() const;

private:
	const int mItemsToProduce;
	const int mTimeout;
	int mItemsProduced;
	SafeRing& mSafeRing;

	HANDLE mhDoneProducingEvent;
};

