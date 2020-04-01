//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_THREAD_EXCEPTION_H
#define ICE_UTIL_THREAD_EXCEPTION_H

#include <IceUtil/Exception.h>
#include <IceUtil/Time.h>

namespace IceUtil
{

class ICE_API ThreadSyscallException : public SyscallExceptionHelper<ThreadSyscallException>
{
public:

    ThreadSyscallException(const char*, int, int);
    virtual std::string ice_id() const;

};

class ICE_API ThreadLockedException : public ExceptionHelper<ThreadLockedException>
{
public:

    ThreadLockedException(const char*, int);
    virtual std::string ice_id() const;

};

class ICE_API ThreadStartedException : public ExceptionHelper<ThreadStartedException>
{
public:

    ThreadStartedException(const char*, int);
    virtual std::string ice_id() const;

};

class ICE_API ThreadNotStartedException : public ExceptionHelper<ThreadNotStartedException>
{
public:

    ThreadNotStartedException(const char*, int);
    virtual std::string ice_id() const;

};

class ICE_API BadThreadControlException : public ExceptionHelper<BadThreadControlException>
{
public:

    BadThreadControlException(const char*, int);
    virtual std::string ice_id() const;

};

class ICE_API InvalidTimeoutException : public ExceptionHelper<InvalidTimeoutException>
{
public:

    InvalidTimeoutException(const char*, int, const Time&);
    virtual std::string ice_id() const;
    virtual void ice_print(std::ostream&) const;

private:

    Time _timeout;
};

}

#endif
