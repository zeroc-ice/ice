// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_EXCEPTION_H
#define ICE_UTIL_EXCEPTION_H

#include <IceUtil/Config.h>

namespace IceUtil
{

class Exception
{
public:

    Exception(const char* file, int line) :
	_file(file),
	_line(line)
    {
    }
    
    virtual ~Exception()
    {
    }

    Exception(const Exception& ex)
    {
	_file = ex._file;
	_line = ex._line;
    }

    Exception& operator=(const Exception& ex)
    {
	if (this != &ex)
	{
	    _line = ex._line;
	    _file = ex._file;
	}
	
	return *this;
    }

    virtual std::string toString() const
    {
	return debugInfo() + "unknown Ice exception";
    }

    virtual Exception* clone() const
    {
	return new Exception(*this);
    }

    virtual void raise() const
    {
	throw *this;
    }


protected:

    std::string debugInfo() const
    {
	std::ostringstream s;
	s << _file << ':' << _line << ": ";
	return s.str();
    }

private:

    const char* _file;
    int _line;
};

inline std::ostream&
operator<<(std::ostream& out, const Exception& ex)
{
    std::string s = ex.toString();
    return out << s;
}

}

#endif
