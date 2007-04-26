// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Exception.h>
#include <IceE/SafeStdio.h>
#include <IceE/StaticMutex.h>

using namespace std;

namespace IceUtil
{

bool nullHandleAbort = false;

};

IceUtil::Exception::Exception() :
    _file(0),
    _line(0)
{
}
    
IceUtil::Exception::Exception(const char* file, int line) :
    _file(file),
    _line(line)
{
}
    
IceUtil::Exception::~Exception() throw()
{
}

const char* IceUtil::Exception::_name = "IceUtil::Exception";

string
IceUtil::Exception::ice_name() const
{
    return _name;
}

string
IceUtil::Exception::toString() const
{
    string out;
    if(_file && _line > 0)
    {
        out += Ice::printfToString("%s:%d: ", _file, _line);
    }
    out += ice_name();
    return out;
}

const char*
IceUtil::Exception::what() const throw()
{
    StaticMutex::Lock lock(globalMutex);
    {
        if(_str.empty())
        {
            _str = toString(); // Lazy initialization.
        }
    }
    return _str.c_str();
}

IceUtil::Exception*
IceUtil::Exception::ice_clone() const
{
    return new Exception(*this);
}

void
IceUtil::Exception::ice_throw() const
{
    throw *this;
}

const char*
IceUtil::Exception::ice_file() const
{
    return _file;
}

int
IceUtil::Exception::ice_line() const
{
    return _line;
}

IceUtil::NullHandleException::NullHandleException(const char* file, int line) :
    Exception(file, line)
{
    if(nullHandleAbort)
    {
#ifdef _WIN32_WCE
	//
	// WinCE does not appear to have abort()
	//
	exit(-1);
#else
	abort();
#endif
    }
}

IceUtil::NullHandleException::~NullHandleException() throw()
{
}

const char* IceUtil::NullHandleException::_name = "IceUtil::NullHandleException";

string
IceUtil::NullHandleException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::NullHandleException::ice_clone() const
{
    return new NullHandleException(*this);
}

void
IceUtil::NullHandleException::ice_throw() const
{
    throw *this;
}

IceUtil::IllegalArgumentException::IllegalArgumentException(const char* file, int line) :
    Exception(file, line)
{
}

IceUtil::IllegalArgumentException::IllegalArgumentException(const char* file, int line, const string& r) :
    Exception(file, line),
    reason(r)
{
}

IceUtil::IllegalArgumentException::~IllegalArgumentException() throw()
{
}

const char* IceUtil::IllegalArgumentException::_name = "IceUtil::IllegalArgumentException";

string
IceUtil::IllegalArgumentException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::IllegalArgumentException::ice_clone() const
{
    return new IllegalArgumentException(*this);
}

void
IceUtil::IllegalArgumentException::ice_throw() const
{
    throw *this;
}
