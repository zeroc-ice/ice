// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_THREAD_EXCEPTION_H
#define ICE_UTIL_THREAD_EXCEPTION_H

#include <IceUtil/Exception.h>

namespace IceUtil
{

class ICE_UTIL_API ThreadSyscallException : public Exception
{
public:

    ThreadSyscallException(const char*, int);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    const int _error;
};

class ICE_UTIL_API ThreadLockedException : public Exception
{
public:

    ThreadLockedException(const char*, int);
    virtual std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;
};
    
}

#endif

