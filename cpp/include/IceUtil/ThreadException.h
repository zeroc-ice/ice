// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_THREAD_EXCEPTION_H
#define ICE_UTIL_THREAD_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <IceUtil/Time.h>

namespace IceUtil
{

class ICE_API ThreadSyscallException : public SyscallException
{
public:

    ThreadSyscallException(const char*, int, int);
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual ThreadSyscallException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

class ICE_API ThreadLockedException : public Exception
{
public:

    ThreadLockedException(const char*, int);
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual ThreadLockedException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

class ICE_API ThreadStartedException : public Exception
{
public:

    ThreadStartedException(const char*, int);
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual ThreadStartedException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

class ICE_API ThreadNotStartedException : public Exception
{
public:

    ThreadNotStartedException(const char*, int);
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual ThreadNotStartedException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

class ICE_API BadThreadControlException : public Exception
{
public:

    BadThreadControlException(const char*, int);
    virtual std::string ice_id() const;
#ifndef ICE_CPP11_MAPPING
    virtual BadThreadControlException* ice_clone() const;
#endif
    virtual void ice_throw() const;
};

class ICE_API InvalidTimeoutException : public Exception
{
public:

    InvalidTimeoutException(const char*, int, const Time&);
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;
#ifndef ICE_CPP11_MAPPING
    virtual InvalidTimeoutException* ice_clone() const;
#endif
    virtual void ice_throw() const;

private:

    Time _timeout;
};
    
}

#endif

