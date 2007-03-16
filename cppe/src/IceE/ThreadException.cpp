// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/ThreadException.h>
#include <IceE/SafeStdio.h>

using namespace std;

IceUtil::ThreadSyscallException::ThreadSyscallException(const char* file, int line, int err ): 
    Exception(file, line),
    _error(err)
{
}
    
const char* IceUtil::ThreadSyscallException::_name = "IceUtil::ThreadSyscallException";

string
IceUtil::ThreadSyscallException::ice_name() const
{
    return _name;
}

string
IceUtil::ThreadSyscallException::toString() const
{
    string out = Exception::toString();
    if(_error != 0)
    {
	out += ":\nthread syscall exception: ";
#ifdef _WIN32_WCE
	out += Ice::printfToString("thread error: %d", _error);
#elif defined(_WIN32)
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

IceUtil::Exception*
IceUtil::ThreadSyscallException::ice_clone() const
{
    return new ThreadSyscallException(*this);
}

void
IceUtil::ThreadSyscallException::ice_throw() const
{
    throw *this;
}

int
IceUtil::ThreadSyscallException::error() const
{
    return _error;
}


IceUtil::ThreadLockedException::ThreadLockedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::ThreadLockedException::_name = "IceUtil::ThreadLockedException";

string
IceUtil::ThreadLockedException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::ThreadLockedException::ice_clone() const
{
    return new ThreadLockedException(*this);
}

void
IceUtil::ThreadLockedException::ice_throw() const
{
    throw *this;
}

IceUtil::ThreadStartedException::ThreadStartedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::ThreadStartedException::_name = "IceUtil::ThreadStartedException";

string
IceUtil::ThreadStartedException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::ThreadStartedException::ice_clone() const
{
    return new ThreadStartedException(*this);
}

void
IceUtil::ThreadStartedException::ice_throw() const
{
    throw *this;
}

IceUtil::ThreadNotStartedException::ThreadNotStartedException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::ThreadNotStartedException::_name = "IceUtil::ThreadNotStartedException";

string
IceUtil::ThreadNotStartedException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::ThreadNotStartedException::ice_clone() const
{
    return new ThreadNotStartedException(*this);
}

void
IceUtil::ThreadNotStartedException::ice_throw() const
{
    throw *this;
}

IceUtil::BadThreadControlException::BadThreadControlException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::BadThreadControlException::_name = "IceUtil::BadThreadControlException";

string
IceUtil::BadThreadControlException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::BadThreadControlException::ice_clone() const
{
    return new BadThreadControlException(*this);
}

void
IceUtil::BadThreadControlException::ice_throw() const
{
    throw *this;
}

