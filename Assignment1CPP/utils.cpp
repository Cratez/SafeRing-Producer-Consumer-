/*
Author:	John Harrison
File:	utils.cpp
Class:	CST352 Operating Systems

Note:	cpp file for utils header
*/

#include "utils.h"

#if defined(DEBUG) || defined(IOLOCKING)
/// <summary>
/// The global io lock
/// </summary>
HANDLE ghIOLock = CreateMutex(
	NULL,
	FALSE,
	NULL
);
#endif