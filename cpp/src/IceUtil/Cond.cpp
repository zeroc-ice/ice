// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Cond.h>

#ifndef WIN32
#    include <sys/time.h>
#endif

#ifdef WIN32

IceUtil::Semaphore::Semaphore(long initial)
{
    _sem = CreateSemaphore(0, initial, 0x7fffffff, 0);
    if (_sem == INVALID_HANDLE_VALUE)
    {
	throw SyscallException(SyscallException::errorToString(GetLastError()), __FILE__, __LINE__);
    }
}

IceUtil::Semaphore::~Semaphore()
{
    CloseHandle(_sem);
}

bool
IceUtil::Semaphore::wait(long timeout)
{
    if (timeout < 0)
    {
	timeout = INFINITE;
    }
    int rc = WaitForSingleObject(_sem, timeout);
    if (rc != WAIT_TIMEOUT && rc != WAIT_OBJECT_0)
    {
	throw SyscallException(SyscallException::errorToString(GetLastError()), __FILE__, __LINE__);
    }
    return rc != WAIT_TIMEOUT;
}

void
IceUtil::Semaphore::post(int count)
{
    int rc = ReleaseSemaphore(_sem, count, 0);
    if (rc == 0)
    {
	throw SyscallException(SyscallException::errorToString(GetLastError()), __FILE__, __LINE__);
    }
}

IceUtil::Cond::Cond() :
    _gate(1),
    _blocked(0),
    _unblocked(0),
    _toUnblock(0)
{
}

IceUtil::Cond::~Cond()
{
}

void
IceUtil::Cond::signal()
{
    wake(false);
}

void
IceUtil::Cond::broadcast()
{
    wake(true);
}

void
IceUtil::Cond::wake(bool broadcast)
{
    //
    // Lock gate & mutex.
    //
    _gate.wait();
    _internal.lock();

    if (_unblocked != 0)
    {
	_blocked -= _unblocked;
	_unblocked = 0;
    }

    if (_blocked > 0)
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
IceUtil::Cond::preWait()
{
    _gate.wait();
    _blocked++;
    _gate.post();
}

void
IceUtil::Cond::postWait(bool timedout)
{
    _internal.lock();
    _unblocked++;

    if (_toUnblock != 0)
    {
	bool last = --_toUnblock == 0;
	_internal.unlock();
	
	if (timedout)
	{
	    _queue.wait();
	}
	
	if (last)
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

bool
IceUtil::Cond::dowait(long timeout)
{
    try
    {
	bool rc = _queue.wait(timeout);
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

IceUtil::Cond::Cond()
{
    int rc = pthread_cond_init(&_cond, 0);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
}

IceUtil::Cond::~Cond()
{
    int rc = 0;
    rc = pthread_cond_destroy(&_cond);
    assert(rc == 0);
}

void
IceUtil::Cond::signal()
{
    int rc = pthread_cond_signal(&_cond);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
}

void
IceUtil::Cond::broadcast()
{
    int rc = pthread_cond_broadcast(&_cond);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
}

namespace IceUtil
{

struct timespec
msecToTimespec(
    long timeout
)
{
    assert(timeout >= 0);
    struct timeval tv;
    gettimeofday(&tv, 0);
    //                       123456789 - 10^9
    const long oneBillion = 1000000000;

    struct timespec abstime;

    abstime.tv_sec = tv.tv_sec + (timeout/1000);
    abstime.tv_nsec = (tv.tv_usec * 1000) + ((timeout%1000) * 1000000);
    if (abstime.tv_nsec > oneBillion)
    {
	++abstime.tv_sec;
	abstime.tv_nsec -= oneBillion;
    }
    
    return abstime;
}

} // End namespace IceUtil

#endif
