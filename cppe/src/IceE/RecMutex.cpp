// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/RecMutex.h>
#include <IceE/Exception.h>

using namespace std;

#ifdef _WIN32

#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400

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

void
IceUtil::RecMutex::lock() const
{
    EnterCriticalSection(&_mutex);
    if(++_count > 1)
    {
	LeaveCriticalSection(&_mutex);
    }
}

bool
IceUtil::RecMutex::tryLock() const
{
    if(!TryEnterCriticalSection(&_mutex))
    {
	return false;
    }
    if(++_count > 1)
    {
	LeaveCriticalSection(&_mutex);
    }
    return true;
}

void
IceUtil::RecMutex::unlock() const
{
    if(--_count == 0)
    {
	LeaveCriticalSection(&_mutex);
    }
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

#   else

IceUtil::RecMutex::RecMutex() :
    _count(0)
{
    _mutex = CreateMutex(0, false, 0);
    if(_mutex == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

IceUtil::RecMutex::~RecMutex()
{
    assert(_count == 0);
    BOOL rc = CloseHandle(_mutex);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

void
IceUtil::RecMutex::lock() const
{
    DWORD rc = WaitForSingleObject(_mutex, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	if(rc == WAIT_FAILED)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
	else
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, 0);
	}
    }
    
    if(++_count > 1)
    {
	BOOL rc2 = ReleaseMutex(_mutex);
	if(rc2 == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
    }
}

bool
IceUtil::RecMutex::tryLock() const
{
    DWORD rc = WaitForSingleObject(_mutex, 0);
    if(rc != WAIT_OBJECT_0)
    {
	return false;
    }
    if(++_count > 1)
    {
	BOOL rc2 = ReleaseMutex(_mutex);
	if(rc2 == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
    }
    return true;
}

void
IceUtil::RecMutex::unlock() const
{
    if(--_count == 0)
    {
	BOOL rc = ReleaseMutex(_mutex);
	if(rc == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
    }
}

void
IceUtil::RecMutex::unlock(LockState& state) const
{
    state.count = _count;
    _count = 0;
    BOOL rc = ReleaseMutex(_mutex);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

void
IceUtil::RecMutex::lock(LockState& state) const
{
    DWORD rc = WaitForSingleObject(_mutex, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	if(rc == WAIT_FAILED)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
	else
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, 0);
	}
    }
    
    _count = state.count;
}

#   endif

#else

IceUtil::RecMutex::RecMutex() :
    _count(0)
{
    int rc;

#if defined(__linux) && !defined(__USE_UNIX98)
    const pthread_mutexattr_t attr = { PTHREAD_MUTEX_RECURSIVE_NP };
#else
    pthread_mutexattr_t attr;
    rc = pthread_mutexattr_init(&attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif
    
    rc = pthread_mutex_init(&_mutex, &attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

#if defined(__linux) && !defined(__USE_UNIX98)
// Nothing to do
#else
    rc = pthread_mutexattr_destroy(&attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
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

void
IceUtil::RecMutex::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    if(++_count > 1)
    {
	rc = pthread_mutex_unlock(&_mutex);
	assert(rc == 0);
    }
}

bool
IceUtil::RecMutex::tryLock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    bool result = (rc == 0);
    if(!result)
    {
	if(rc != EBUSY)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    } 
    else if(++_count > 1)
    {
	rc = pthread_mutex_unlock(&_mutex);
	if(rc != 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    }
    return result;
}

void
IceUtil::RecMutex::unlock() const
{
    if(--_count == 0)
    {
	int rc = 0; // Prevent warnings when NDEBUG is defined.
	rc = pthread_mutex_unlock(&_mutex);
	assert(rc == 0);
    }
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
