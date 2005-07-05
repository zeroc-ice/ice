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

namespace IceE
{

bool ICEE_API nullHandleAbort = false;

};

IceE::Exception::Exception() :
    _file(0),
    _line(0)
{
}
    
IceE::Exception::Exception(const char* file, int line) :
    _file(file),
    _line(line)
{
}
    
IceE::Exception::~Exception()
{
}

const char* IceE::Exception::_name = "IceE::Exception";

const string
IceE::Exception::ice_name() const
{
    return _name;
}

string
IceE::Exception::toString() const
{
    string out;
    if(_file && _line > 0)
    {
	out += IceE::printfToString("%s:%d: ", _file, _line);
    }
    out += ice_name();
    return out;
}

IceE::Exception*
IceE::Exception::ice_clone() const
{
    return new Exception(*this);
}

void
IceE::Exception::ice_throw() const
{
    throw *this;
}

const char*
IceE::Exception::ice_file() const
{
    return _file;
}

int
IceE::Exception::ice_line() const
{
    return _line;
}

IceE::NullHandleException::NullHandleException(const char* file, int line) :
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

const char* IceE::NullHandleException::_name = "IceE::NullHandleException";

const string
IceE::NullHandleException::ice_name() const
{
    return _name;
}

IceE::Exception*
IceE::NullHandleException::ice_clone() const
{
    return new NullHandleException(*this);
}

void
IceE::NullHandleException::ice_throw() const
{
    throw *this;
}
