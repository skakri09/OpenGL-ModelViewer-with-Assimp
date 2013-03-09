/********************************************************************
    created:    9:3:2013   14:25
    filename:   ThreadingException.h
    author:     Kristian Skarseth
    
    purpose:    
*********************************************************************/
#ifndef ThreadingException_h__
#define ThreadingException_h__

#include <stdexcept>
#include <string>
#include <iostream>
#include <sstream>

class ThreadingException : public std::runtime_error
{
public:
	ThreadingException(const char* file, unsigned int line, const char* msg):std::runtime_error(msg)
	{
		std::cerr << file << ":" << line << ": " << msg << std::endl;
	}

	ThreadingException(const char* file, unsigned int line, const std::string msg):std::runtime_error(msg)
	{
		std::cerr << file << ":" << line << ": " << msg << std::endl;
	}
};

#define THREADING_EXCEPTION(msg) throw ThreadingException(__FILE__, __LINE__, msg)
#endif // ThreadingException_h__