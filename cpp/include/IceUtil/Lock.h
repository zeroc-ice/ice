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

#ifndef ICE_UTIL_LOCK_H
#define ICE_UTIL_LOCK_H

#include <IceUtil/Config.h>

namespace IceUtil
{

//
// Forward declarations.
//
class Cond;

//
// We must name this LockT instead of Lock, because otherwise some
// compilers (such as Sun Forte 6.2) have problems with constructs
// such as:
//
// class Foo
// {
//     // ...
//     typedef Lock<Mutex> Lock;
// }
//
template <typename T>
class LockT
{
public:
    
    LockT(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.lock();
    }

    ~LockT()
    {
	_mutex.unlock();
    }

private:

    const T& _mutex;

    friend class Cond;
};

//
// Must be name TryLockT, not TryLock. See the comment for LockT for
// an explanation.
//
template <typename T>
class TryLockT
{
public:

    TryLockT(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.trylock();
    }
    
    ~TryLockT()
    {
	_mutex.unlock();
    }

private:

    const T& _mutex;

    friend class Cond;
};

} // End namespace IceUtil

#endif
