#pragma once
#include <iostream>
#include <exception>
#include <stdexcept>
#include <sstream>
#include <string>


using namespace std;

class TimeoutException : public runtime_error {
public:
	TimeoutException(int msTimeout) : runtime_error("Timeout Exception") {
		this->msTimeout = msTimeout;
	}

	virtual const char* what() const throw()
	{
		ostringstream ss;
		ss << runtime_error::what() << ": after " << msTimeout << " miliseconds";

		return ss.str().c_str();
	}

private:
	int msTimeout;
};