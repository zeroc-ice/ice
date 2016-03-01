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

class ICE_UTIL_API ThreadSyscallException : public SyscallException
{
public:

    ThreadSyscallException(const char*, int, int);
    virtual std::string ice_name() const;
    virtual ThreadSyscallException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API ThreadLockedException : public Exception
{
public:

    ThreadLockedException(const char*, int);
    virtual std::string ice_name() const;
    virtual ThreadLockedException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API ThreadStartedException : public Exception
{
public:

    ThreadStartedException(const char*, int);
    virtual std::string ice_name() const;
    virtual ThreadStartedException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API ThreadNotStartedException : public Exception
{
public:

    ThreadNotStartedException(const char*, int);
    virtual std::string ice_name() const;
    virtual ThreadNotStartedException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API BadThreadControlException : public Exception
{
public:

    BadThreadControlException(const char*, int);
    virtual std::string ice_name() const;
    virtual BadThreadControlException* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API InvalidTimeoutException : public Exception
{
public:

    InvalidTimeoutException(const char*, int, const Time&);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual InvalidTimeoutException* ice_clone() const;
    virtual void ice_throw() const;

private:
    
    Time _timeout;
    static const char* _name;
};
    
}

#endif

