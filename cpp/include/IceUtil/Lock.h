// **********************************************************************
//
// Copyright (c) 2002
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

template <typename T>
class Lock
{
public:

    Lock(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.lock();
    }

    ~Lock()
    {
	_mutex.unlock();
    }

private:

    const T& _mutex;

    friend class Cond;
};

template <typename T>
class TryLock
{
public:

    TryLock(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.trylock();
    }

    ~TryLock()
    {
	_mutex.unlock();
    }

private:

    const T& _mutex;

    friend class Cond;
};

} // End namespace IceUtil

#endif
