// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
class RLock
{
public:

    RLock(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.readLock();
    }

    ~RLock()
    {
	_mutex.unlock();
    }

    void upgrade()
    {
	_mutex.upgrade();
    }

private:

    const T& _mutex;
};

template <typename T>
class TryRLock
{
public:

    TryRLock(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.tryReadLock();
    }

    ~TryRLock()
    {
	_mutex.unlock();
    }

    void upgrade()
    {
	_mutex.upgrade();
    }

private:

    const T& _mutex;
};

template <typename T>
class WLock
{
public:

    WLock(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.writeLock();
    }

    ~WLock()
    {
	_mutex.unlock();
    }

private:

    const T& _mutex;
};

template <typename T>
class TryWLock
{
public:

    TryWLock(const T& mutex) :
	_mutex(mutex)
    {
	_mutex.tryWriteLock();
    }

    ~TryWLock()
    {
	_mutex.unlock();
    }

private:

    const T& _mutex;
};

//
// Concurrency primitive that allows many readers & one writer access
// to a data structure. Writers have priority over readers. The
// structure is not strictly fair in that there is no absolute queue
// of waiting writers - that is managed by a condition variable.
//
// Both Reader & Writer mutexes can be recursively locked. Calling
// WLock (or TryWLock) while holding a read lock promotes the reader
// to a writer lock.
//
class ICE_UTIL_API RWRecMutex
{
public:

    //
    // RLock (reader) & WLock (writer) typedefs.
    //
    typedef RLock<RWRecMutex> RLock;
    typedef TryRLock<RWRecMutex> TryRLock;
    typedef WLock<RWRecMutex> WLock;
    typedef TryWLock<RWRecMutex> TryWLock;

    RWRecMutex();
    ~RWRecMutex();

    //
    // Note that readLock/writeLock & unlock in general should not be
    // used directly. Instead use RLock & WLock.
    //

    //
    // Acquire a read lock.
    //
    void readLock() const;

    //
    // Try to acquire a read lock.
    //
    void tryReadLock() const;

    //
    // Acquire a write lock.
    //
    void writeLock() const;

    //
    // Acquire a write lock.
    //
    void tryWriteLock() const;

    //
    // Unlock the reader/writer lock.
    //
    void unlock() const;

    //
    // Upgrade the read lock to a writer lock. Note that this method
    // can only be called if the reader lock is not held recursively.
    //
    void upgrade() const;

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
    // If there is an active writer this is a control for thread.
    //
    mutable ThreadControl _writerControl;

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
