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

#include <IceUtil/RecMutex.h>
#include <IceUtil/Exception.h>

using namespace std;

#ifdef _WIN32

IceUtil::RecMutex::RecMutex() :
    _count(0)
{
    InitializeCriticalSection(&_mutex);
}

IceUtil::RecMutex::~RecMutex()
{
    assert(_count == 0);
    DeleteCriticalSection(&_mutex);
}

bool
IceUtil::RecMutex::lock() const
{
    EnterCriticalSection(&_mutex);
    if(++_count > 1)
    {
	LeaveCriticalSection(&_mutex);
	return false;
    }
    return true;
}

bool
IceUtil::RecMutex::trylock() const
{
    if(!TryEnterCriticalSection(&_mutex))
    {
	throw ThreadLockedException(__FILE__, __LINE__);
    }
    if(++_count > 1)
    {
	LeaveCriticalSection(&_mutex);
	return false;
    }
    return true;
}

bool
IceUtil::RecMutex::unlock() const
{
    if(--_count == 0)
    {
	LeaveCriticalSection(&_mutex);
	return true;
    }
    return false;
}

void
IceUtil::RecMutex::unlock(LockState& state) const
{
    state.count = _count;
    _count = 0;
    LeaveCriticalSection(&_mutex);
}

void
IceUtil::RecMutex::lock(LockState& state) const
{
    EnterCriticalSection(&_mutex);
    _count = state.count;
}
#else

IceUtil::RecMutex::RecMutex() :
    _count(0)
{
    int rc;

#if _POSIX_VERSION >= 199506L

    pthread_mutexattr_t attr;

    rc = pthread_mutexattr_init(&attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }

    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }

#elif defined(__linux__)

    const pthread_mutexattr_t attr = { PTHREAD_MUTEX_RECURSIVE_NP };

#else

    const pthread_mutexattr_t attr = { PTHREAD_MUTEX_RECURSIVE };

#endif
    
    rc = pthread_mutex_init(&_mutex, &attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }

#if _POSIX_VERSION >= 199506L

    rc = pthread_mutexattr_destroy(&attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }

#endif
}

IceUtil::RecMutex::~RecMutex()
{
    assert(_count == 0);
    int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
}

bool
IceUtil::RecMutex::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    if(++_count > 1)
    {
	rc = pthread_mutex_unlock(&_mutex);
	assert(rc == 0);
	return false;
    }
    return true;
}

bool
IceUtil::RecMutex::trylock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0)
    {
	if(rc == EBUSY)
	{
	    throw ThreadLockedException(__FILE__, __LINE__);
	}
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    if(++_count > 1)
    {
	rc = pthread_mutex_unlock(&_mutex);
	assert(rc == 0);
	return false;
    }
    return true;
}

bool
IceUtil::RecMutex::unlock() const
{
    if(--_count == 0)
    {
	int rc = 0; // Prevent warnings when NDEBUG is defined.
	rc = pthread_mutex_unlock(&_mutex);
	assert(rc == 0);
	return true;
    }
    return false;
}

void
IceUtil::RecMutex::unlock(LockState& state) const
{
    state.mutex = &_mutex;
    state.count = _count;
    _count = 0;
}

void
IceUtil::RecMutex::lock(LockState& state) const
{
    _count = state.count;
}

#endif

bool
IceUtil::RecMutex::willUnlock() const
{
    return _count == 1;
}
