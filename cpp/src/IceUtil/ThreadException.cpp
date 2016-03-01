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
    
const char* IceUtil::ThreadSyscallException::_name = "IceUtil::ThreadSyscallException";

string
IceUtil::ThreadSyscallException::ice_name() const
{
    return _name;
}

IceUtil::ThreadSyscallException*
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

const char* IceUtil::ThreadLockedException::_name = "IceUtil::ThreadLockedException";

string
IceUtil::ThreadLockedException::ice_name() const
{
    return _name;
}

IceUtil::ThreadLockedException*
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

IceUtil::ThreadStartedException*
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

IceUtil::ThreadNotStartedException*
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

IceUtil::BadThreadControlException*
IceUtil::BadThreadControlException::ice_clone() const
{
    return new BadThreadControlException(*this);
}

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
    
const char* IceUtil::InvalidTimeoutException::_name = "IceUtil::InvalidTimeoutException";

string
IceUtil::InvalidTimeoutException::ice_name() const
{
    return _name;
}

void
IceUtil::InvalidTimeoutException::ice_print(ostream& os) const
{
    Exception::ice_print(os);
    os << ":\ninvalid timeout: " << _timeout << " seconds";
}

IceUtil::InvalidTimeoutException*
IceUtil::InvalidTimeoutException::ice_clone() const
{
    return new InvalidTimeoutException(*this);
}

void
IceUtil::InvalidTimeoutException::ice_throw() const
{
    throw *this;
}
