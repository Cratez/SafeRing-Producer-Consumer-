#pragma once
#include <random>

//Random number generator between 1 and 1000
int GetRand();

//template start function to spawn a thread onto a class method "Start"
template<typename T>
DWORD __stdcall StartTemplate(LPVOID arg)
{
	if (!arg)
		return 0;
	T *class_ptr = (T*)arg;
	class_ptr->Start();
	return 1;
}
