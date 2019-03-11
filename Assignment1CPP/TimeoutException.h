/*
Author:	John Harrison
File:	TimeoutException.h
Class:	CST352 Operating Systems

Note:	Header file for timeout exception class
*/

#pragma once
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>

using namespace std;
//Note: Was using error message stuff, but it was being dumb. Left it alone
/// <summary>
/// Timeout exception class
/// </summary>
/// <seealso cref="runtime_error" />
class TimeoutException : public runtime_error {
public:
	TimeoutException(int msTimeout) : runtime_error("Timeout Exception") {
		this->msTimeout = msTimeout;
	}
#ifdef OLD

	//	ostringstream ss;
	//	ss << runtime_error::what() << ": after " << msTimeout << " miliseconds";

	//	//store defaul message
	//	mMessage = ss.str().c_str();

	//	//get wMessage
	//	size_t string_size = strnlen_s(
	//		mMessage,	// get the non unicode string
	//		100			// arbitrary max length for this
	//	) + 1;			// +1 for termination

	//	size_t out_size;

	//	mWMessage = new wchar_t[string_size];
	//	//mbstowcs(mWMessage, mMessage, string_size);
	//	mbstowcs_s(&out_size, mWMessage, string_size, mMessage, string_size - 1);
	//}
	//~TimeoutException() {
	//	delete[] mWMessage;
	//}

	//virtual const char* what() const throw()
	//{
	//	return mMessage;
	//}

	//const wchar_t* wWhat() const throw() {
	//	return mWMessage;
	//}
#endif // OLD

	const int timeout() {
		return msTimeout;
	}

private:
	int msTimeout;

#ifdef OLD
	//wchar_t* mWMessage;
	//const char* mMessage;	//pointer to a const char, NOT a constant character pointer
#endif
};