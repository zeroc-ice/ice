// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Cond.h>

#ifndef _WIN32
#    include <sys/time.h>
#endif

#ifdef _WIN32

Ice::Semaphore::Semaphore(long initial)
{
    _sem = CreateSemaphore(0, initial, 0x7fffffff, 0);
    if(_sem == INVALID_HANDLE_VALUE)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

Ice::Semaphore::~Semaphore()
{
    CloseHandle(_sem);
}

void
Ice::Semaphore::wait() const
{
    int rc = WaitForSingleObject(_sem, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

bool
Ice::Semaphore::timedWait(const Time& timeout) const
{
    timeval tv = timeout;
    long msec = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

    int rc = WaitForSingleObject(_sem, msec);
    if(rc != WAIT_TIMEOUT && rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
    return rc != WAIT_TIMEOUT;
}

void
Ice::Semaphore::post(int count) const
{
    int rc = ReleaseSemaphore(_sem, count, 0);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

Ice::Cond::Cond() :
    _gate(1),
    _blocked(0),
    _unblocked(0),
    _toUnblock(0)
{
}

Ice::Cond::~Cond()
{
}

void
Ice::Cond::signal()
{
    wake(false);
}

void
Ice::Cond::broadcast()
{
    wake(true);
}

void
Ice::Cond::wake(bool broadcast)
{
    //
    // Lock gate & mutex.
    //
    _gate.wait();
    _internal.lock();

    if(_unblocked != 0)
    {
	_blocked -= _unblocked;
	_unblocked = 0;
    }

    if(_blocked > 0)
    {
	//
	// Unblock some number of waiters.
	//
	_toUnblock = (broadcast) ? _blocked : 1;
	_internal.unlock();
	_queue.post();
    }
    else
    {
	//
	// Otherwise no blocked waiters, release gate & mutex.
	//
	_gate.post();
	_internal.unlock();
    }
}

void
Ice::Cond::preWait() const
{
    _gate.wait();
    _blocked++;
    _gate.post();
}

void
Ice::Cond::postWait(bool timedOut) const
{
    _internal.lock();
    _unblocked++;

    if(_toUnblock != 0)
    {
	bool last = --_toUnblock == 0;
	_internal.unlock();
	
	if(timedOut)
	{
	    _queue.wait();
	}
	
	if(last)
	{
	    _gate.post();
	}
	else
	{
	    _queue.post();
	}
    }
    else
    {
	_internal.unlock();
    }
}

void
Ice::Cond::dowait() const
{
    try
    {
	_queue.wait();
	postWait(false);
    }
    catch(...)
    {
	postWait(false);
	throw;
    }
}

bool
Ice::Cond::timedDowait(const Time& timeout) const
{
    try
    {
	bool rc = _queue.timedWait(timeout);
	postWait(!rc);
	return rc;
    }
    catch(...)
    {
	postWait(false);
	throw;
    }
}

#else

Ice::Cond::Cond()
{
    int rc;

    pthread_condattr_t attr;

    rc = pthread_condattr_init(&attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    rc = pthread_cond_init(&_cond, &attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    rc = pthread_condattr_destroy(&attr);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

Ice::Cond::~Cond()
{
    int rc = 0;
    rc = pthread_cond_destroy(&_cond);
    assert(rc == 0);
}

void
Ice::Cond::signal()
{
    int rc = pthread_cond_signal(&_cond);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
Ice::Cond::broadcast()
{
    int rc = pthread_cond_broadcast(&_cond);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

#endif
