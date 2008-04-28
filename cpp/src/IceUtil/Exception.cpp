// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Exception.h>
#include <IceUtil/StaticMutex.h>
#include <IceUtil/StringUtil.h>
#include <ostream>
#include <cstdlib>

using namespace std;

namespace IceUtil
{

bool ICE_DECLSPEC_EXPORT nullHandleAbort = false;

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

void
IceUtil::Exception::ice_print(ostream& out) const
{
    if(_file && _line > 0)
    {
        out << _file << ':' << _line << ": ";
    }
    out << ice_name();
}

const char*
IceUtil::Exception::what() const throw()
{
    try
    {
        StaticMutex::Lock lock(globalMutex);
        {
            if(_str.empty())
            {
                stringstream s;
                ice_print(s);
                _str = s.str(); // Lazy initialization.
            }
        }
        return _str.c_str();
    }
    catch(...)
    {
    }
    return "";
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
    _reason(r)
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

void
IceUtil::IllegalArgumentException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ": " << _reason;
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

string
IceUtil::IllegalArgumentException::reason() const
{
    return _reason;
}

IceUtil::SyscallException::SyscallException(const char* file, int line, int err ): 
    Exception(file, line),
    _error(err)
{
}
    
const char* IceUtil::SyscallException::_name = "IceUtil::SyscallException";

string
IceUtil::SyscallException::ice_name() const
{
    return _name;
}

void
IceUtil::SyscallException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    if(_error != 0)
    {
        os << ":\nsyscall exception: " << IceUtilInternal::errorToString(_error);
    }
}

IceUtil::Exception*
IceUtil::SyscallException::ice_clone() const
{
    return new SyscallException(*this);
}

void
IceUtil::SyscallException::ice_throw() const
{
    throw *this;
}

int
IceUtil::SyscallException::error() const
{
    return _error;
}

