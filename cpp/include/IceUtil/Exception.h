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

    Exception() :
	_file(0),
	_line(0)
    {
    }
    
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
	    _file = ex._file;
	    _line = ex._line;
	}
	
	return *this;
    }

    virtual std::string _name() const
    {
	return "IceUtil::Exception";
    }

    virtual std::string _description() const
    {
	return "unknown Ice exception";
    }

    virtual Exception* _clone() const
    {
	return new Exception(*this);
    }

    virtual void _throw() const
    {
	throw *this;
    }
    
private:
    
    const char* _file;
    int _line;
    friend std::ostream& operator<<(std::ostream&, const Exception&);
};

inline std::ostream&
operator<<(std::ostream& out, const Exception& ex)
{
    if (ex._file && ex._line > 0)
    {
	out << ex._file << ':' << ex._line << ": ";
    }
    out << ex._name() << ": " << ex._description();
    return out;
}

}

#endif
