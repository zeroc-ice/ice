// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/Exception.h>

using namespace std;

namespace IceUtil
{

bool ICE_UTIL_API nullHandleAbort = false;

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
    
IceUtil::Exception::~Exception()
{
}

string
IceUtil::Exception::ice_name() const
{
    return "IceUtil::Exception";
}

void
IceUtil::Exception::ice_print(ostream& out) const
{
    if(_file && _line > 0)
    {
	out << _file << ':' << _line << ": ";
    }
    out << ice_name();
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

ostream&
IceUtil::operator<<(ostream& out, const IceUtil::Exception& ex)
{
    ex.ice_print(out);
    return out;
}

IceUtil::NullHandleException::NullHandleException(const char* file, int line) :
    Exception(file, line)
{
    if(nullHandleAbort)
    {
	abort();
    }
}

string
IceUtil::NullHandleException::ice_name() const
{
    return "IceUtil::NullHandleException";
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
