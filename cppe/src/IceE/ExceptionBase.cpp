// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Exception.h>
#include <IceE/SafeStdio.h>

using namespace std;

namespace Ice
{

bool ICEE_API nullHandleAbort = false;

};

Ice::Exception::Exception() :
    _file(0),
    _line(0)
{
}
    
Ice::Exception::Exception(const char* file, int line) :
    _file(file),
    _line(line)
{
}
    
Ice::Exception::~Exception()
{
}

const char* Ice::Exception::_name = "Ice::Exception";

const string
Ice::Exception::ice_name() const
{
    return _name;
}

string
Ice::Exception::toString() const
{
    string out;
    if(_file && _line > 0)
    {
	out += Ice::printfToString("%s:%d: ", _file, _line);
    }
    out += ice_name();
    return out;
}

Ice::Exception*
Ice::Exception::ice_clone() const
{
    return new Exception(*this);
}

void
Ice::Exception::ice_throw() const
{
    throw *this;
}

const char*
Ice::Exception::ice_file() const
{
    return _file;
}

int
Ice::Exception::ice_line() const
{
    return _line;
}

Ice::NullHandleException::NullHandleException(const char* file, int line) :
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

const char* Ice::NullHandleException::_name = "Ice::NullHandleException";

const string
Ice::NullHandleException::ice_name() const
{
    return _name;
}

Ice::Exception*
Ice::NullHandleException::ice_clone() const
{
    return new NullHandleException(*this);
}

void
Ice::NullHandleException::ice_throw() const
{
    throw *this;
}
