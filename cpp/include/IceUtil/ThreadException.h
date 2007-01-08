// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    ThreadSyscallException(const char*, int, int);
    virtual std::string ice_name() const;
    virtual void ice_print(std::ostream&) const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

    int error() const;
private:

    const int _error;
    static const char* _name;
};

class ICE_UTIL_API ThreadLockedException : public Exception
{
public:

    ThreadLockedException(const char*, int);
    virtual std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API ThreadStartedException : public Exception
{
public:

    ThreadStartedException(const char*, int);
    virtual std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API ThreadNotStartedException : public Exception
{
public:

    ThreadNotStartedException(const char*, int);
    virtual std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};

class ICE_UTIL_API BadThreadControlException : public Exception
{
public:

    BadThreadControlException(const char*, int);
    virtual std::string ice_name() const;
    virtual Exception* ice_clone() const;
    virtual void ice_throw() const;

private:

    static const char* _name;
};
    
}

#endif

