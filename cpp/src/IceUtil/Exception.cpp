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
IceUtil::Exception::ice_name() const
{
    static const string name("IceUtil::Exception");
    return name;
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
}

string
IceUtil::NullHandleException::ice_name() const
{
    static const string name("IceUtil::NullHandleException");
    return name;
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

IceUtil::SyscallException::SyscallException(const string& error, const char* file, int line) : 
    Exception(file, line),
    _error(error)
{
}
    
string
IceUtil::SyscallException::ice_name() const
{
    return "IceUtil::SyscallException";
}

void
IceUtil::SyscallException::ice_print(ostream& os) const
{
    os << _error << ": ";
    Exception::ice_print(os);
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

#ifdef _WIN32
string
IceUtil::SyscallException::errorToString(DWORD error)
{
    LPVOID lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL,
		  error,
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
		  (LPTSTR)&lpMsgBuf,
		  0,
		  NULL);
    string result = (LPCTSTR)lpMsgBuf;
    LocalFree( lpMsgBuf );
    return result;
}
#endif

IceUtil::LockedException::LockedException(const char* file, int line) :
    Exception(file, line)
{
}

string
IceUtil::LockedException::ice_name() const
{
    static const string name = "IceUtil::LockedException";
    return name;
}

IceUtil::Exception*
IceUtil::LockedException::ice_clone() const
{
    return new LockedException(*this);
}

void
IceUtil::LockedException::ice_throw() const
{
    throw *this;
}
