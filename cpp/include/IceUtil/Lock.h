// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    Lock(T& mutex) :
	_mutex(mutex)
    {
	_mutex.lock();
    }

    ~Lock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;

    friend class Cond;
};

template <typename T>
class TryLock
{
public:

    TryLock(T& mutex) :
	_mutex(mutex)
    {
	_mutex.trylock();
    }

    ~TryLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;

    friend class Cond;
};

//
// This is for use when a class derives from Mutex, Monitor or
// RecMutex. Otherwise declare the concurrency primitive mutable.
//
template <typename T>
class ConstLock
{
public:

    ConstLock(const T& mutex) :
	_mutex(const_cast<T&>(mutex))
    {
	_mutex.lock();
    }

    ~ConstLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;

    friend class Cond;
};

template <typename T>
class ConstTryLock
{
public:

    ConstTryLock(const T& mutex) :
	_mutex(const_cast<T>(mutex))
    {
	_mutex.trylock();
    }

    ~ConstTryLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;

    friend class Cond;
};

} // End namespace IceUtil

#endif
