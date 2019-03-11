/*
Author:	John Harrison
File:	safering.h
Class:	CST352 Operating Systems

Note:	Header file for safering class
*/
#pragma once
#include <Windows.h>
#include "utils.h"


/// <summary>
/// Safe ring class.
/// Thread safe data collection
/// </summary>
class SafeRing {
public:
	SafeRing(int capacity);
	~SafeRing();
	int Count();
	void Insert(int value, int msTimeout = -1);
	int Remove(int msTimeout = -1 , HANDLE abortEvent = NULL);

private:
	HANDLE mhHasCapacity;
	HANDLE mhHasItem;
	HANDLE mhInsertMutex;
	HANDLE mhRemoveMutex;
	int* mBuffer;
	int mCapacity;
	int mHead;
	int mTail;

	void closeHandles();
};