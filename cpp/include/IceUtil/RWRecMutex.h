// **********************************************************************
//
// Copyright (c) 2001
// IONA Technologies, Inc.
// Waltham, MA, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_RW_REC_MUTEX_H
#define ICE_UTIL_RW_REC_MUTEX_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Cond.h>

namespace IceUtil
{

template <typename T>
class RLock
{
public:

    RLock(T& mutex) :
	_mutex(mutex)
    {
	_mutex.readLock();
    }

    ~RLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

template <typename T>
class ConstRLock
{
public:

    ConstRLock(const T& mutex) :
	_mutex(const_cast<T>(mutex))
    {
	_mutex.readLock();
    }

    ~ConstRLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

template <typename T>
class TryRLock
{
public:

    TryRLock(T& mutex) :
	_mutex(mutex)
    {
	_mutex.tryReadLock();
    }

    ~TryRLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

template <typename T>
class ConstTryRLock
{
public:

    ConstTryRLock(const T& mutex) :
	_mutex(const_cast<T>(mutex))
    {
	_mutex.tryReadLock();
    }

    ~ConstTryRLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

template <typename T>
class WLock
{
public:

    WLock(T& mutex) :
	_mutex(mutex)
    {
	_mutex.writeLock();
    }

    ~WLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

template <typename T>
class ConstWLock
{
public:

    ConstWLock(const T& mutex) :
	_mutex(const_cast<T>(mutex))
    {
	_mutex.writeLock();
    }

    ~ConstWLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

template <typename T>
class TryWLock
{
public:

    TryWLock(T& mutex) :
	_mutex(mutex)
    {
	_mutex.tryWriteLock();
    }

    ~TryWLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};


template <typename T>
class ConstTryWLock
{
public:

    ConstTryWLock(const T& mutex) :
	_mutex(const_cast<T>(mutex))
    {
	_mutex.tryWriteLock();
    }

    ~ConstTryWLock()
    {
	_mutex.unlock();
    }

private:

    T& _mutex;
};

//
// Concurrency primitive that allows many readers & one writer access
// to a data structure. Writers have priority over readers. The
// structure is not strictly fair in that there is no absolute queue
// of waiting writers - that is managed by a condition variable.
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

    typedef ConstRLock<RWRecMutex> ConstRLock;
    typedef ConstTryRLock<RWRecMutex> ConstTryRLock;
    typedef ConstWLock<RWRecMutex> ConstWLock;
    typedef ConstTryWLock<RWRecMutex> ConstTryWLock;


    RWRecMutex();
    ~RWRecMutex();

    //
    // Note that readLock/writeLock & unlock in general should not be
    // used directly. Instead use RLock & WLock.
    //

    //
    // Acquire a read lock.
    //
    void readLock();

    //
    // Try to acquire a read lock.
    //
    void tryReadLock();

    //
    // Acquire a write lock.
    //
    void writeLock();

    //
    // Acquire a write lock.
    //
    void tryWriteLock();

    //
    // Unlock the reader/writer lock.
    //
    void unlock();

private:

    // noncopyable
    RWRecMutex(const RWRecMutex&);
    void operator=(const RWRecMutex&);

    //
    // Number of readers holding the lock. -1 means a writer has the
    // lock.
    //
    int	_count;

    //
    // Number of waiting writers.
    //
    unsigned int _waitingWriters;

    //
    // Internal mutex.
    //
    Mutex _mutex;

    //
    // Two condition variables for waiting readers & writers.
    //
    Cond _readers;
    Cond _writers;
};

} // End namespace IceUtil

#endif
