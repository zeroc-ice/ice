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

#include <IceUtil/ThreadException.h>

using namespace std;

IceUtil::ThreadSyscallException::ThreadSyscallException(const char* file, int line) : 
    Exception(file, line),
#ifdef _WIN32
    _error(GetLastError())
#else
    _error(errno)
#endif
{
}
    
string
IceUtil::ThreadSyscallException::ice_name() const
{
    return "IceUtil::ThreadSyscallException";
}

void
IceUtil::ThreadSyscallException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    if(_error != 0)
    {
	os << ":\nthread syscall exception: ";
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
	    os << msg;
	    LocalFree(lpMsgBuf);
	}
	else
	{
	    os << "unknown thread error";
	}
#else
        os << strerror(_error);
#endif
    }
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

IceUtil::ThreadLockedException::ThreadLockedException(const char* file, int line) :
    Exception(file, line)
{
}

string
IceUtil::ThreadLockedException::ice_name() const
{
    return "IceUtil::ThreadLockedException";
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

string
IceUtil::ThreadStartedException::ice_name() const
{
    return "IceUtil::ThreadStartedException";
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
