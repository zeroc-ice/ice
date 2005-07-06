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

Ice::ThreadSyscallException::ThreadSyscallException(const char* file, int line, int err ): 
    Exception(file, line),
    _error(err)
{
}
    
const char* Ice::ThreadSyscallException::_name = "Ice::ThreadSyscallException";

const string
Ice::ThreadSyscallException::ice_name() const
{
    return _name;
}

string
Ice::ThreadSyscallException::toString() const
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
	    out += Ice::printfToString("error=%d", _error);
	}
#else
        out += strerror(_error);
#endif
    }
    return out;
}

Ice::Exception*
Ice::ThreadSyscallException::ice_clone() const
{
    return new ThreadSyscallException(*this);
}

void
Ice::ThreadSyscallException::ice_throw() const
{
    throw *this;
}

int
Ice::ThreadSyscallException::error() const
{
    return _error;
}


Ice::ThreadLockedException::ThreadLockedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* Ice::ThreadLockedException::_name = "Ice::ThreadLockedException";

const string
Ice::ThreadLockedException::ice_name() const
{
    return _name;
}

Ice::Exception*
Ice::ThreadLockedException::ice_clone() const
{
    return new ThreadLockedException(*this);
}

void
Ice::ThreadLockedException::ice_throw() const
{
    throw *this;
}

Ice::ThreadStartedException::ThreadStartedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* Ice::ThreadStartedException::_name = "Ice::ThreadStartedException";

const string
Ice::ThreadStartedException::ice_name() const
{
    return _name;
}

Ice::Exception*
Ice::ThreadStartedException::ice_clone() const
{
    return new ThreadStartedException(*this);
}

void
Ice::ThreadStartedException::ice_throw() const
{
    throw *this;
}

Ice::ThreadNotStartedException::ThreadNotStartedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* Ice::ThreadNotStartedException::_name = "Ice::ThreadNotStartedException";

const string
Ice::ThreadNotStartedException::ice_name() const
{
    return _name;
}

Ice::Exception*
Ice::ThreadNotStartedException::ice_clone() const
{
    return new ThreadNotStartedException(*this);
}

void
Ice::ThreadNotStartedException::ice_throw() const
{
    throw *this;
}
