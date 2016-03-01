// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ThreadException.h>

using namespace std;

IceUtil::ThreadSyscallException::ThreadSyscallException(const char* file, int line, int err ): 
    SyscallException(file, line, err)
{
}

string
IceUtil::ThreadSyscallException::ice_id() const
{
    return "::IceUtil::ThreadSyscallException";
}

#ifndef ICE_CPP11_MAPPING
IceUtil::ThreadSyscallException*
IceUtil::ThreadSyscallException::ice_clone() const
{
    return new ThreadSyscallException(*this);
}
#endif

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
IceUtil::ThreadLockedException::ice_id() const
{
    return "::IceUtil::ThreadLockedException";
}

#ifndef ICE_CPP11_MAPPING
IceUtil::ThreadLockedException*
IceUtil::ThreadLockedException::ice_clone() const
{
    return new ThreadLockedException(*this);
}
#endif

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
IceUtil::ThreadStartedException::ice_id() const
{
    return "::IceUtil::ThreadStartedException";
}

#ifndef ICE_CPP11_MAPPING
IceUtil::ThreadStartedException*
IceUtil::ThreadStartedException::ice_clone() const
{
    return new ThreadStartedException(*this);
}
#endif

void
IceUtil::ThreadStartedException::ice_throw() const
{
    throw *this;
}

IceUtil::ThreadNotStartedException::ThreadNotStartedException(const char* file, int line) :
    Exception(file, line)
{
}

string
IceUtil::ThreadNotStartedException::ice_id() const
{
    return "::IceUtil::ThreadNotStartedException";
}

#ifndef ICE_CPP11_MAPPING
IceUtil::ThreadNotStartedException*
IceUtil::ThreadNotStartedException::ice_clone() const
{
    return new ThreadNotStartedException(*this);
}
#endif

void
IceUtil::ThreadNotStartedException::ice_throw() const
{
    throw *this;
}


IceUtil::BadThreadControlException::BadThreadControlException(const char* file, int line) :
    Exception(file, line)
{
}

string
IceUtil::BadThreadControlException::ice_id() const
{
    return "::IceUtil::BadThreadControlException";
}

#ifndef ICE_CPP11_MAPPING
IceUtil::BadThreadControlException*
IceUtil::BadThreadControlException::ice_clone() const
{
    return new BadThreadControlException(*this);
}
#endif

void
IceUtil::BadThreadControlException::ice_throw() const
{
    throw *this;
}

IceUtil::InvalidTimeoutException::InvalidTimeoutException(const char* file, int line, 
                                                          const IceUtil::Time& timeout) : 
    Exception(file, line),
    _timeout(timeout)
{
}

string
IceUtil::InvalidTimeoutException::ice_id() const
{
    return "::IceUtil::InvalidTimeoutException";
}

void
IceUtil::InvalidTimeoutException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    os << ":\ninvalid timeout: " << _timeout << " seconds";
}

#ifndef ICE_CPP11_MAPPING
IceUtil::InvalidTimeoutException*
IceUtil::InvalidTimeoutException::ice_clone() const
{
    return new InvalidTimeoutException(*this);
}
#endif

void
IceUtil::InvalidTimeoutException::ice_throw() const
{
    throw *this;
}
