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

class Exception;
std::ostream& printException(std::ostream&, const Exception&);

class Exception
{
public:

    Exception() :
	_theFile(0),
	_theLine(0)
    {
    }
    
    Exception(const char* file, int line) :
	_theFile(file),
	_theLine(line)
    {
    }
    
    virtual ~Exception()
    {
    }

    Exception(const Exception& ex)
    {
	_theFile = ex._theFile;
	_theLine = ex._theLine;
    }

    Exception& operator=(const Exception& ex)
    {
	if (this != &ex)
	{
	    _theFile = ex._theFile;
	    _theLine = ex._theLine;
	}
	
	return *this;
    }

    virtual std::string _name() const
    {
	return "IceUtil::Exception";
    }

    virtual std::ostream& _print(std::ostream& out) const
    {
	//
	// Double dispatch, to allow user code to modify the behavior
	// of _print() by providing specialized versions of
	// IceUtil::printException() for generated exceptions derived
	// from this base exception.
	//
	return printException(out, *this);
    }

    virtual Exception* _clone() const
    {
	return new Exception(*this);
    }

    virtual void _throw() const
    {
	throw *this;
    }

    const char* _file() const
    {
	return _theFile;
    }

    int _line() const
    {
	return _theLine;
    }
    
private:
    
    const char* _theFile;
    int _theLine;
};

inline std::ostream&
printException(std::ostream& out, const Exception& ex)
{
    if (ex._file() && ex._line() > 0)
    {
	out << ex._file() << ':' << ex._line() << ": ";
    }
    return out << ex._name();
}

inline std::ostream&
operator<<(std::ostream& out, const Exception& ex)
{
    return ex._print(out);
}

}

#endif
