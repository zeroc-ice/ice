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
    _theFile(0),
    _theLine(0)
{
}
    
IceUtil::Exception::Exception(const char* file, int line) :
    _theFile(file),
    _theLine(line)
{
}
    
IceUtil::Exception::~Exception()
{
}

string
IceUtil::Exception::_name() const
{
    return "IceUtil::Exception";
}

void
IceUtil::Exception::_print(ostream& out) const
{
    if (_file() && _line() > 0)
    {
	out << _file() << ':' << _line() << ": ";
    }
    out << _name();
}

IceUtil::Exception*
IceUtil::Exception::_clone() const
{
    return new Exception(*this);
}

void
IceUtil::Exception::_throw() const
{
    throw *this;
}

const char*
IceUtil::Exception::_file() const
{
    return _theFile;
}

int
IceUtil::Exception::_line() const
{
    return _theLine;
}

ostream&
IceUtil::operator<<(ostream& out, const IceUtil::Exception& ex)
{
    ex._print(out);
    return out;
}

IceUtil::NullHandleException::NullHandleException(const char* file, int line) :
    Exception(file, line)
{
}

string
IceUtil::NullHandleException::_name() const
{
    return "IceUtil::NullHandleException";
}

string
IceUtil::NullHandleException::_description() const
{
    return "operation call on null handle";
}

IceUtil::Exception*
IceUtil::NullHandleException::_clone() const
{
    return new NullHandleException(*this);
}

void
IceUtil::NullHandleException::_throw() const
{
    throw *this;
}
