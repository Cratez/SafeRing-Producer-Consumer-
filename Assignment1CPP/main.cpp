/*
Author:	John Harrison
File:	main.cpp
Class:	CST352 Operating Systems

Note:	File containing main entry point as described by the assignment notes
*/

#include <Windows.h>	/* Threads & locks */
#include <iostream>		/* IO */
#include "SafeRing.h"	/* Safe ring class*/
#include "Consumer.h"	/* Consumer class*/
#include "Producer.h"	/* Producer class*/
#include "utils.h"		/* misc utils */

#define PRODUCER_COUNT 2
#define CONSUMER_COUNT 2
#define ITEMS_TO_PRODUCE 10
#define MS_TIMEOUT 10
#define ARRAY_SIZE 5
#define TESTMODE FALSE

#if TESTMODE == TRUE
#include <string>
using namespace std;
HANDLE lock1, lock2;

string threadNames[10];

struct randStuct {
	int index;
	HANDLE lock;
};

DWORD __stdcall randFunc(LPVOID arg) {
	randStuct* args = (randStuct*)arg;
	while (true) {
		IOSYNC(cout << threadNames[args->index] << " waiting " << args->lock << endl);
		WaitForSingleObject(args->lock, INFINITE);
		IOSYNC(cout << threadNames[args->index] << " got " << args->lock << endl);
		ReleaseMutex(args->lock);
		IOSYNC(cout << threadNames[args->index] << " released " << args->lock << endl);
		//Sleep(10);
	}
	
}

int main() {
	lock1 = CreateMutex(NULL, FALSE, NULL);
	lock2 = CreateMutex(NULL, FALSE, NULL);
	HANDLE handles[2] = { lock1, lock2 };
	for (int i = 0; i < 10; i++) {
		threadNames[i] = "Thread" + to_string(i);
	}

	for (int i = 0; i <= 4; i++) {
		randStuct* st = new randStuct();
		st->index = i;
		st->lock = lock1;
		CreateThread(NULL, NULL, randFunc, st, 0, NULL);
	}

	for (int i = 5; i <= 9; i++) {
		randStuct* st = new randStuct();
		st->index = i;
		st->lock = lock2;
		CreateThread(NULL, NULL, randFunc, st, 0, NULL);
	}

	HANDLE cthread = GetCurrentThread();
	SetThreadPriority(cthread, GetThreadPriority(cthread) + 2);
	IOSYNC(cout << "Main waiting for locks" << endl);
	WaitForMultipleObjects(2, handles, TRUE, INFINITE);
	IOSYNC(cout << "Main got the locks!" << endl);
	getchar();
}

#else
/// <summary>
/// Mains the specified argc.
/// </summary>
/// <param name="argc">The argc.</param>
/// <param name="argv">The argv.</param>
/// <returns>Result code (always default :) )</returns>
int main(int argc, char** argv)
{
	SafeRing safeRing(ARRAY_SIZE);
	HANDLE producerThreads[PRODUCER_COUNT];
	HANDLE consumerThreads[CONSUMER_COUNT];
	Producer* producers[PRODUCER_COUNT];
	Consumer* consumers[CONSUMER_COUNT];
	
	//create and start the producers
	for (int i = 0; i < PRODUCER_COUNT; i++) {
		producers[i] = new Producer(
			ITEMS_TO_PRODUCE,
			safeRing,
			MS_TIMEOUT
		);

		producerThreads[i] = CreateThread(
			NULL,
			NULL,
			start_template<Producer>,
			producers[i],
			0,
			NULL
		);
	}

	//create and start the consumers
	for (int i = 0; i < CONSUMER_COUNT; i++) {
		consumers[i] = new Consumer(
			safeRing,
			MS_TIMEOUT
		);

		consumerThreads[i] = CreateThread(
			NULL,
			NULL,
			start_template<Consumer>,
			consumers[i],
			0,
			NULL
		);
	}

	//wait on the producers to exit first
	WaitForMultipleObjects(
		PRODUCER_COUNT,
		producerThreads,
		TRUE,
		INFINITE
	);
	
	//this is not the most elegant method, but otherwise I need to rework to add a manual reset event
	while (safeRing.Count() != 0) {
		DEBUGLOG(std::wcout << L"Waiting for SafeRing::Count to be 0" << std::endl);
		Sleep(1000);
	}

	//abort the threads
	for (int i = 0; i < CONSUMER_COUNT; i++) {
		IOSYNC(std::wcout << L"Sending stop call to consumer thread, ID: " << GetThreadId(consumerThreads[i]) << std::endl);
		consumers[i]->Stop();
	}

	IOSYNC(std::wcout << L"Done. -John Harrison"<< std::endl);

	//pause. Crude but it works for a simple console pause
	system("pause");

	//delete our stuff
	for (auto deletable : producers)
		delete deletable;
	for (auto deletable : consumers)
		delete deletable;

	for (HANDLE h : producerThreads) {
		CloseHandle(h);
	}
	for (HANDLE h : consumerThreads) {
		CloseHandle(h);
	}
}
#endif