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

#ifndef ICE_UTIL_RW_REC_MUTEX_H
#define ICE_UTIL_RW_REC_MUTEX_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Cond.h>
#include <IceUtil/Thread.h>

namespace IceUtil
{

template <typename T>
class RLockT
{
public:

    RLockT(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.readlock();
	_acquired = true;
    }

    ~RLockT()
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
	_mutex.readlock();
	_acquired = true;
    }

    bool tryAcquire() const
    {
	if (_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_acquired = _mutex.tryReadlock();
	return _acquired;
    }

    bool timedTryAcquire(const Time& timeout) const
    {
	if (_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_acquired = _mutex.timedTryReadlock(timeout);
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

    void
    upgrade() const
    {
	_mutex.upgrade();
    }

    void
    timedUpgrade(const Time& timeout) const
    {
	_mutex.timedUpgrade(timeout);
    }

protected:
    
    // TryRLockT's constructors

    RLockT(const T& mutex, bool) :
	_mutex(mutex)
    {
	_acquired = _mutex.tryReadlock();
    }


    RLockT(const T& mutex, const Time& timeout) :
	_mutex(mutex)
    {
	_acquired = _mutex.timedTryReadlock(timeout);
    }


private:

    // Not implemented; prevents accidental use.
    //
    RLockT(const RLockT&);
    RLockT& operator=(const RLockT&);

    const T& _mutex;
    mutable bool _acquired;
};

template <typename T>
class TryRLockT : public RLockT<T>
{
public:

    TryRLockT(const T& mutex) :
	RLockT<T>(mutex, true)
    {
    }

    TryRLockT(const T& mutex, const Time& timeout) :
	RLockT<T>(mutex, timeout)
    {
    }
};

template <typename T>
class WLockT
{
public:

    WLockT(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.writelock();
	_acquired = true;
    }

    ~WLockT()
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
	_mutex.writelock();
	_acquired = true;
    }

    bool tryAcquire() const
    {
	if (_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_acquired = _mutex.tryWritelock();
	return _acquired;
    }

    bool timedTryAcquire(const Time& timeout) const
    {
	if (_acquired)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	_acquired = _mutex.timedTryWritelock(timeout);
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

    // TryWLockT's constructor

    WLockT(const T& mutex, bool) :
	_mutex(mutex)
    {
	_acquired = _mutex.tryWritelock();
    }

    WLockT(const T& mutex, const Time& timeout) :
	_mutex(mutex)
    {
	_acquired = _mutex.timedTryWritelock(timeout);
    }

private:

    // Not implemented; prevents accidental use.
    //
    WLockT(const WLockT&);
    WLockT& operator=(const WLockT&);

    const T& _mutex;
    mutable bool _acquired;
};

template <typename T>
class TryWLockT : public WLockT<T>
{
public:

    TryWLockT(const T& mutex) :
	WLockT<T>(mutex, true)
    {
    }

    TryWLockT(const T& mutex, const Time& timeout) :
	WLockT<T>(mutex, timeout)
    {
    }
};

//
// Concurrency primitive that allows many readers & one writer access
// to a data structure. Writers have priority over readers. The
// structure is not strictly fair in that there is no absolute queue
// of waiting writers - that is managed by a condition variable.
//
// Both Reader & Writer mutexes can be recursively locked. Calling
// upgrade() or timedUpgrade() while holding a read lock promotes
// the reader to a writer lock.
//
class ICE_UTIL_API RWRecMutex
{
public:

    //
    // RLock (reader) & WLock (writer) typedefs.
    //
    typedef RLockT<RWRecMutex> RLock;
    typedef TryRLockT<RWRecMutex> TryRLock;
    typedef WLockT<RWRecMutex> WLock;
    typedef TryWLockT<RWRecMutex> TryWLock;

    RWRecMutex();
    ~RWRecMutex();

    //
    // Note that readlock/writelock & unlock in general should not be
    // used directly. Instead use RLock & WLock.
    //

    //
    // Acquire a read lock.
    //
    void readlock() const;

    //
    // Try to acquire a read lock.
    //
    bool tryReadlock() const;

    //
    // Try to acquire a read lock for upto the given timeout.
    //
    bool timedTryReadlock(const Time&) const;

    //
    // Acquire a write lock.
    //
    void writelock() const;

    //
    // Acquire a write lock.
    //
    bool tryWritelock() const;

    //
    // Acquire a write lock for up to the given timeout.
    //
    bool timedTryWritelock(const Time&) const;

    //
    // Unlock the reader/writer lock.
    //
    void unlock() const;

    //
    // Upgrade the read lock to a writer lock. Note that this method
    // can only be called if the reader lock is not held recursively.
    //
    void upgrade() const;

    //
    // Upgrade the read lock to a writer lock for up to the given
    // timeout Note that this method can only be called if the reader
    // lock is not held recursively.
    //
    void timedUpgrade(const Time&) const;

private:

    // noncopyable
    RWRecMutex(const RWRecMutex&);
    void operator=(const RWRecMutex&);

    //
    // Number of readers holding the lock. A positive number indicates
    // readers are active. A negative number means that a writer is
    // active.
    //
    mutable int _count;

    //
    // If there is an active writer this is the ID of the writer thread.
    //
    mutable ThreadId _writerId;

    //
    // Number of waiting writers.
    //
    mutable unsigned int _waitingWriters;

    //
    // Internal mutex.
    //
    Mutex _mutex;

    //
    // Two condition variables for waiting readers & writers.
    //
    mutable Cond _readers;
    mutable Cond _writers;
};

} // End namespace IceUtil

#endif
