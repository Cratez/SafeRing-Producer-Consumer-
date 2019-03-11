/*
Author:	John Harrison
File:	utils.h
Class:	CST352 Operating Systems

Note:	Header file for utilities/misc that are used in this project
For simplicity of linkage, templates are listed in entirely here. Not separated into .h and .inc
*/
#pragma once
#include <random>
#include <Windows.h>

//Define debug to print out all the debug log lines
//disable IOLocking if you don't want IO to be pretty.
//#define DEBUG
#define IOLOCKING

#if defined(DEBUG) || defined(IOLOCKING)
extern HANDLE ghIOLock;
#endif

//this is to simplify the debug lines
#ifdef  DEBUG
#define DEBUGLOG(x){						\
WaitForSingleObject(ghIOLock,INFINITE);		\
x;											\
ReleaseMutex(ghIOLock);						\
}
#else
#define DEBUGLOG(x)
#endif

//io syncing for appearance
#ifdef IOLOCKING
#define IOSYNC(x) {						\
WaitForSingleObject(ghIOLock,INFINITE);	\
x;										\
ReleaseMutex(ghIOLock);					\
}
#else
#define IOSYNC(x)
#endif

//get_rand_uniform template

/// <summary>
/// Get_rand_uniforms this instance.
/// </summary>
/// <returns>value between low and high</returns>
template<int low, int high>
int get_rand_uniform() {
	static std::default_random_engine sGenerator;
	static std::uniform_int_distribution<int> sDistribution(low, high);

	return sDistribution(sGenerator);
}

//template start function to spawn a thread onto a class method "Start"
/// <summary>
/// Start_templates the specified argument.
/// </summary>
/// <param name="arg">The argument.</param>
/// <returns>Indicator of the successful start</returns>
template<typename T>
DWORD __stdcall start_template(LPVOID arg)
{
	if (!arg)
		return 0;
	T *class_ptr = (T*)arg;
	class_ptr->Start();
	return 1;
}