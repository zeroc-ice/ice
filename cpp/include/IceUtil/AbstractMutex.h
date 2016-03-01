// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_ABSTRACT_MUTEX_H
#define ICE_UTIL_ABSTRACT_MUTEX_H

#include <IceUtil/Config.h>
#include <IceUtil/Lock.h>


namespace IceUtil
{

class ICE_UTIL_API AbstractMutex
{
public:
    
    typedef LockT<AbstractMutex> Lock;
    typedef TryLockT<AbstractMutex> TryLock;

    virtual ~AbstractMutex()
    {
    }

    virtual void lock() const = 0;
    virtual void unlock() const = 0;
    virtual bool tryLock() const = 0;
};

template <typename T>
class AbstractMutexI : public AbstractMutex, public T
{
public:

    typedef LockT<AbstractMutexI> Lock;
    typedef TryLockT<AbstractMutexI> TryLock;

    virtual void lock() const
    {
        T::lock();
    }

    virtual void unlock() const
    {
        T::unlock();
    }

    virtual bool tryLock() const
    {
        return T::tryLock();
    }

    virtual ~AbstractMutexI()
    {}
};

template <typename T>
class AbstractMutexReadI : public AbstractMutex, public T
{
public:

    typedef LockT<AbstractMutexReadI> Lock;
    typedef TryLockT<AbstractMutexReadI> TryLock;

    virtual void lock() const
    {
        T::readLock();
    }

    virtual void unlock() const
    {
        T::unlock();
    }

    virtual bool tryLock() const
    {
        return T::tryReadLock();
    }

    virtual ~AbstractMutexReadI()
    {}
};

template <typename T>
class AbstractMutexWriteI : public AbstractMutex, public T
{
public:

    typedef LockT<AbstractMutexWriteI> Lock;
    typedef TryLockT<AbstractMutexWriteI> TryLock;

    virtual void lock() const
    {
        T::writeLock();
    }

    virtual void unlock() const
    {
        T::unlock();
    }

    virtual bool tryLock() const
    {
        return T::tryWriteLock();
    }

    virtual ~AbstractMutexWriteI()
    {}
};

}

#endif
