// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Exception.h>

using namespace std;

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
IceUtil::Exception::_ice_name() const
{
    return "IceUtil::Exception";
}

void
IceUtil::Exception::_ice_print(ostream& out) const
{
    if (_file && _line > 0)
    {
	out << _file << ':' << _line << ": ";
    }
    out << _ice_name();
}

IceUtil::Exception*
IceUtil::Exception::_ice_clone() const
{
    return new Exception(*this);
}

void
IceUtil::Exception::_ice_throw() const
{
    throw *this;
}

const char*
IceUtil::Exception::_ice_file() const
{
    return _file;
}

int
IceUtil::Exception::_ice_line() const
{
    return _line;
}

ostream&
IceUtil::operator<<(ostream& out, const IceUtil::Exception& ex)
{
    ex._ice_print(out);
    return out;
}

IceUtil::NullHandleException::NullHandleException(const char* file, int line) :
    Exception(file, line)
{
}

string
IceUtil::NullHandleException::_ice_name() const
{
    return "IceUtil::NullHandleException";
}

string
IceUtil::NullHandleException::_ice_description() const
{
    return "operation call on null handle";
}

IceUtil::Exception*
IceUtil::NullHandleException::_ice_clone() const
{
    return new NullHandleException(*this);
}

void
IceUtil::NullHandleException::_ice_throw() const
{
    throw *this;
}
