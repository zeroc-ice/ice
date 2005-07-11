// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_THREAD_EXCEPTION_H
#define ICEE_THREAD_EXCEPTION_H

#include <IceE/Exception.h>

namespace IceUtil
{

class ICEE_API ThreadSyscallException : public Exception
{
public:

    ThreadSyscallException(const char*, int, int);
    virtual const std::string ice_name() const;
    virtual std::string toString() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

    int error() const;
private:

    const int _error;
    static const char* _name;
};

class ICEE_API ThreadLockedException : public Exception
{
public:

    ThreadLockedException(const char*, int);
    virtual const std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICEE_API ThreadStartedException : public Exception
{
public:

    ThreadStartedException(const char*, int);
    virtual const std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICEE_API ThreadNotStartedException : public Exception
{
public:

    ThreadNotStartedException(const char*, int);
    virtual const std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};
    
}

#endif

