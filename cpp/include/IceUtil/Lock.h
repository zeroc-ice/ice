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
#include <IceUtil/ThreadException.h>

namespace IceUtil
{

//
// Forward declarations.
//
class Cond;


// LockT and TryLockT are the preferred construct to lock/trylock/unlock
// simple and recursive mutexes. You typically allocate them on the
// stack to hold a lock on a mutex.
// LockT and TryLockT are not recursive: you cannot acquire several times 
// in a row a lock with the same Lock or TryLock object.
// 
// We must name this LockT instead of Lock, because otherwise some
// compilers (such as Sun C++ 5.4) have problems with constructs
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
	_acquired = true;
    }

    ~LockT()
    {
	if (_acquired)
	{
	    _mutex.unlock();
	}
    }
    
    void acquire() const
    {
	if (_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_mutex.lock();
	_acquired = true;
    }


    bool tryAcquire() const
    {
	if (_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_acquired = _mutex.trylock();
	return _acquired;
    }

    void release() const
    {
	if (!_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_mutex.unlock();
	_acquired = false;
    }

    bool acquired() const
    {
	return _acquired;
    }
   
protected:
    
    // TryLockT's contructor
    LockT(const T& mutex, bool) :
	_mutex(mutex)
    {
	_acquired = _mutex.trylock();
    }

private:
    
    // Not implemented; prevents accidental use.
    //
    LockT(const LockT&);
    LockT& operator=(const LockT&);

    const T& _mutex;
    mutable bool _acquired;

    friend class Cond;
};

//
// Must be name TryLockT, not TryLock. See the comment for LockT for
// an explanation.
//
template <typename T>
class TryLockT : public LockT<T>
{
public:

    TryLockT(const T& mutex) :
	LockT<T>(mutex, true)
    {}
};

} // End namespace IceUtil

#endif
