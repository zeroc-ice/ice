// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ThreadException.h>
#include <IceE/SafeStdio.h>

using namespace std;

IceE::ThreadSyscallException::ThreadSyscallException(const char* file, int line, int err ): 
    Exception(file, line),
    _error(err)
{
}
    
const char* IceE::ThreadSyscallException::_name = "IceE::ThreadSyscallException";

const string
IceE::ThreadSyscallException::ice_name() const
{
    return _name;
}

string
IceE::ThreadSyscallException::toString() const
{
    string out = Exception::toString();
    if(_error != 0)
    {
	out += ":\nthread syscall exception: ";
#ifdef _WIN32
	LPVOID lpMsgBuf = 0;
	DWORD ok = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				 FORMAT_MESSAGE_FROM_SYSTEM |
				 FORMAT_MESSAGE_IGNORE_INSERTS,
				 NULL,
				 _error,
				 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				 (LPTSTR)&lpMsgBuf,
				 0,
				 NULL);
	
	if(ok)
	{
	    LPCTSTR msg = (LPCTSTR)lpMsgBuf;
	    assert(msg && strlen((char*)msg) > 0);
	    out += reinterpret_cast<const char*>(msg);
	    LocalFree(lpMsgBuf);
	}
	else
	{
	    out += "unknown thread error: ";
	    out += IceE::printfToString("error=%d", _error);
	}
#else
        out += strerror(_error);
#endif
    }
    return out;
}

IceE::Exception*
IceE::ThreadSyscallException::ice_clone() const
{
    return new ThreadSyscallException(*this);
}

void
IceE::ThreadSyscallException::ice_throw() const
{
    throw *this;
}

int
IceE::ThreadSyscallException::error() const
{
    return _error;
}


IceE::ThreadLockedException::ThreadLockedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceE::ThreadLockedException::_name = "IceE::ThreadLockedException";

const string
IceE::ThreadLockedException::ice_name() const
{
    return _name;
}

IceE::Exception*
IceE::ThreadLockedException::ice_clone() const
{
    return new ThreadLockedException(*this);
}

void
IceE::ThreadLockedException::ice_throw() const
{
    throw *this;
}

IceE::ThreadStartedException::ThreadStartedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceE::ThreadStartedException::_name = "IceE::ThreadStartedException";

const string
IceE::ThreadStartedException::ice_name() const
{
    return _name;
}

IceE::Exception*
IceE::ThreadStartedException::ice_clone() const
{
    return new ThreadStartedException(*this);
}

void
IceE::ThreadStartedException::ice_throw() const
{
    throw *this;
}

IceE::ThreadNotStartedException::ThreadNotStartedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceE::ThreadNotStartedException::_name = "IceE::ThreadNotStartedException";

const string
IceE::ThreadNotStartedException::ice_name() const
{
    return _name;
}

IceE::Exception*
IceE::ThreadNotStartedException::ice_clone() const
{
    return new ThreadNotStartedException(*this);
}

void
IceE::ThreadNotStartedException::ice_throw() const
{
    throw *this;
}
