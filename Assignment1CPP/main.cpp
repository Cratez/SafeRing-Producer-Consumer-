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
#define ARRAY_SIZE 10

//template function to start the threads into the class
template<typename T>
DWORD __stdcall StartTemplate(LPVOID arg)
{
	if (!arg)
		return 0;
	T *class_ptr = (T*)arg;
	class_ptr->Start();
	return 1;
}

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
			StartTemplate<Producer>,
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
			StartTemplate<Consumer>,
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
	getchar();

	//delete our stuff
	delete[] producers;
	delete[] consumers;
	for (HANDLE h : producerThreads) {
		CloseHandle(h);
	}
	for (HANDLE h : consumerThreads) {
		CloseHandle(h);
	}
}