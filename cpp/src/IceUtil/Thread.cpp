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

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/ThreadException.h>

using namespace std;

#ifdef _WIN32

IceUtil::ThreadControl::ThreadControl() :
    _handle(new HandleWrapper(0)),
    _id(GetCurrentThreadId()),
    _detached(false)
{
    HANDLE proc = GetCurrentProcess();
    HANDLE current = GetCurrentThread();
    int rc = DuplicateHandle(proc, current, proc, &_handle->handle, SYNCHRONIZE, TRUE, 0);
    if(rc == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
}

IceUtil::ThreadControl::ThreadControl(const HandleWrapperPtr& handle, unsigned int id) :
    _handle(handle),
    _id(id),
    _detached(false)
{
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    return _id == rhs._id;
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return _id != rhs._id;
}

bool
IceUtil::ThreadControl::operator<(const ThreadControl& rhs) const
{
    return _id != rhs._id;
}

IceUtil::ThreadId
IceUtil::ThreadControl::id() const
{
    return _id;
}

void
IceUtil::ThreadControl::join()
{
    if(_detached)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    int rc = WaitForSingleObject(_handle->handle, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    _detached = true;
}

void
IceUtil::ThreadControl::detach()
{
    if(_detached)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    _detached = true;
}

bool
IceUtil::ThreadControl::isAlive() const
{
    DWORD rc;
    if(GetExitCodeThread(_handle->handle, &rc) == 0)
    {
	return false;
    }
    return rc == STILL_ACTIVE;
}

void
IceUtil::ThreadControl::sleep(const Time& timeout)
{
    timeval tv = timeout;
    long msec = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    Sleep(msec);
}

void
IceUtil::ThreadControl::yield()
{
    //
    // A value of zero causes the thread to relinquish the remainder
    // of its time slice to any other thread of equal priority that is
    // ready to run.
    //
    Sleep(0);
}

IceUtil::Thread::Thread() :
    _started(false),
    _id(0),
    _handle(new HandleWrapper(0))
{
}

IceUtil::Thread::~Thread()
{
}

IceUtil::Thread::ThreadId
IceUtil::Thread::id() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return _id;
}

static void*
startHook(void* arg)
{
    try
    {
	IceUtil::Thread* rawThread = static_cast<IceUtil::Thread*>(arg);

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	IceUtil::ThreadPtr thread = rawThread;

	//
	// See the comment in IceUtil::Thread::start() for details.
	//
	rawThread->__decRef();
	thread->run();
    }
    catch(const IceUtil::Exception& e)
    {
	cerr << "IceUtil::Thread::run(): uncaught exception: ";
	cerr << e << endl;
    }
    return 0;
}

#include <process.h>

IceUtil::ThreadControl
IceUtil::Thread::start()
{
    IceUtil::Mutex::Lock lock(_stateMutex);

    if(_started)
    {
	throw ThreadStartedException(__FILE__, __LINE__);
    }

    //
    // It's necessary to increment the reference count since
    // pthread_create won't necessarily call the thread function until
    // later. If the user does (new MyThread)->start() then the thread
    // object could be deleted before the thread object takes
    // ownership. It's also necessary to increment the reference count
    // prior to calling pthread_create since the thread itself calls
    // __decRef().
    //
    __incRef();
    
    _handle->handle = (HANDLE)_beginthreadex(0, 0, (unsigned int (__stdcall*)(void*))startHook, (LPVOID)this, 0, &_id);
    if(_handle->handle == 0)
    {
	__decRef();
	throw ThreadSyscallException(__FILE__, __LINE__);
    }

    _started = true;
			
    return ThreadControl(_handle, _id);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_handle, _id);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	if(!_started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    {
	IceUtil::Mutex::Lock lock(rhs._stateMutex);
	if(!rhs._started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    return _id == rhs._id;
}

bool
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	if(!_started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    {
	IceUtil::Mutex::Lock lock(rhs._stateMutex);
	if(!rhs._started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    return _id != rhs._id;
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	if(!_started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    {
	IceUtil::Mutex::Lock lock(rhs._stateMutex);
	if(!rhs._started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    return _id < rhs._id;
}

#else

IceUtil::ThreadControl::ThreadControl(pthread_t id) :
    _id(id),
    _detached(false)
{
}

IceUtil::ThreadControl::ThreadControl() :
    _id(pthread_self()),
    _detached(false)
{
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    return pthread_equal(_id, rhs._id);
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !pthread_equal(_id, rhs._id);
}

bool
IceUtil::ThreadControl::operator<(const ThreadControl& rhs) const
{
    // NOTE: Linux specific
    return _id < rhs._id;
}

IceUtil::ThreadId
IceUtil::ThreadControl::id() const
{
    return _id;
}

void
IceUtil::ThreadControl::join()
{
    if(_detached)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    void* ignore = 0;
    int rc = pthread_join(_id, &ignore);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    _detached = true;
}

void
IceUtil::ThreadControl::detach()
{
    if(_detached)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    int rc = pthread_detach(_id);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__);
    }
    _detached = true;
}

bool
IceUtil::ThreadControl::isAlive() const
{
    int policy;
    struct sched_param param;
    return pthread_getschedparam(_id, &policy, &param) == 0;
}

void
IceUtil::ThreadControl::sleep(const Time& timeout)
{
    struct timeval tv = timeout;
    struct timespec ts;
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000L;
    nanosleep(&ts, 0);
}

void
IceUtil::ThreadControl::yield()
{
    sched_yield();
}

IceUtil::Thread::Thread() :
    _started(false),
    _id(0)
{
}

IceUtil::Thread::~Thread()
{
}

IceUtil::ThreadId
IceUtil::Thread::id() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return _id;
}

extern "C" {
static void*
startHook(void* arg)
{
    try
    {
	IceUtil::Thread* rawThread = static_cast<IceUtil::Thread*>(arg);

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	IceUtil::ThreadPtr thread = rawThread;

	//
	// See the comment in IceUtil::Thread::start() for details.
	//
	rawThread->__decRef();
	thread->run();
    }
    catch(const IceUtil::Exception& e)
    {
	cerr << "IceUtil::Thread::run(): uncaught exception: ";
	cerr << e << endl;
    }
    return 0;
}
}

IceUtil::ThreadControl
IceUtil::Thread::start()
{
    IceUtil::Mutex::Lock lock(_stateMutex);

    if(_started)
    {
	throw ThreadStartedException(__FILE__, __LINE__);
    }

    //
    // It's necessary to increment the reference count since
    // pthread_create won't necessarily call the thread function until
    // later. If the user does (new MyThread)->start() then the thread
    // object could be deleted before the thread object takes
    // ownership. It's also necessary to increment the reference count
    // prior to calling pthread_create since the thread itself calls
    // __decRef().
    //
    __incRef();
    int rc = pthread_create(&_id, 0, startHook, this);
    if(rc != 0)
    {
	__decRef();
	throw ThreadSyscallException(__FILE__, __LINE__);
    }

    _started = true;

    return ThreadControl(_id);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_id);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	if(!_started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    {
	IceUtil::Mutex::Lock lock(rhs._stateMutex);
	if(!rhs._started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    return pthread_equal(_id, rhs._id);
}

bool
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	if(!_started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    {
	IceUtil::Mutex::Lock lock(rhs._stateMutex);
	if(!rhs._started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    return !pthread_equal(_id, rhs._id);
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	if(!_started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    {
	IceUtil::Mutex::Lock lock(rhs._stateMutex);
	if(!rhs._started)
	{
	    throw ThreadNotStartedException(__FILE__, __LINE__);
	}
    }
    // NOTE: Linux specific
    return _id < rhs._id;
}

#endif
