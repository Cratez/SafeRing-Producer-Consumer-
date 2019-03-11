/*
Author:	John Harrison
File:	consumer.h
Class:	CST352 Operating Systems

Note:	Header file for consumer class
*/
#pragma once

#include "SafeRing.h"
#include <atomic>

/// <summary>
/// Consumer class. Consumes items from data structure until stopped
/// </summary>
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

