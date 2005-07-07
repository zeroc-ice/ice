// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Thread.h>
#include <IceE/Time.h>
#include <IceE/ThreadException.h>

#include <stdio.h>

using namespace std;

#ifdef _WIN32

Ice::ThreadControl::ThreadControl()
{
    Ice::Mutex::Lock lock(_stateMutex);
    _handle = new HandleWrapper(GetCurrentThread(), false);
    _id = GetCurrentThreadId();
}

Ice::ThreadControl::ThreadControl(const HandleWrapperPtr& handle, ThreadId id)
{
    Ice::Mutex::Lock lock(_stateMutex);
    _handle = handle;
    _id = id;
}

Ice::ThreadControl::ThreadControl(const ThreadControl& tc)
{
    ThreadId id;
    HandleWrapperPtr handle;
    {
	Ice::Mutex::Lock lock(tc._stateMutex);
	id = tc._id;
	handle = tc._handle;
    }
    Ice::Mutex::Lock lock(_stateMutex);
    _handle = handle;
    _id = id;
}

Ice::ThreadControl&
Ice::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	ThreadId id;
	HandleWrapperPtr handle;
	{
	    Ice::Mutex::Lock lock(rhs._stateMutex);
	    handle = rhs._handle;
	    id = rhs._id;
	}
	Ice::Mutex::Lock lock(_stateMutex);
	_handle = handle;
	_id = id;
    }
    return *this;
}

bool
Ice::ThreadControl::operator==(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    Ice::Mutex::Lock lock(_stateMutex);
    return _id == id;
}

bool
Ice::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

bool
Ice::ThreadControl::operator<(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    Ice::Mutex::Lock lock(_stateMutex);
    return _id < id;
}

Ice::ThreadId
Ice::ThreadControl::id() const
{
    Ice::Mutex::Lock lock(_stateMutex);
    return _id;
}

void
Ice::ThreadControl::join()
{
    HandleWrapperPtr handle;
    {
	Ice::Mutex::Lock lock(_stateMutex);
	handle = _handle;
    }
    int rc = WaitForSingleObject(handle->handle, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

void
Ice::ThreadControl::detach()
{
    // No-op: Windows doesn't have the concept of detaching a thread.
}

bool
Ice::ThreadControl::isAlive() const
{
    HandleWrapperPtr handle;
    {
	Ice::Mutex::Lock lock(_stateMutex);
	handle = _handle;
    }
    DWORD rc;
    if(GetExitCodeThread(handle->handle, &rc) == 0)
    {
	return false;
    }
    return rc == STILL_ACTIVE;
}

void
Ice::ThreadControl::sleep(const Time& timeout)
{
    long msec = (long)timeout.toMilliSeconds();
    Sleep(msec);
}

void
Ice::ThreadControl::yield()
{
    //
    // A value of zero causes the thread to relinquish the remainder
    // of its time slice to any other thread of equal priority that is
    // ready to run.
    //
    Sleep(0);
}

Ice::Thread::Thread()
{
    Ice::Mutex::Lock lock(_stateMutex);
    _started = false;
    _id = 0;
    _handle = new HandleWrapper(0);
}

Ice::Thread::~Thread()
{
}

Ice::ThreadId
Ice::Thread::id() const
{
    Ice::Mutex::Lock lock(_stateMutex);
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
	Ice::Thread* rawThread = static_cast<Ice::Thread*>(arg);

        //
        // Initialize the random number generator in each thread.
        //
        unsigned int seed = static_cast<unsigned int>(Ice::Time::now().toMicroSeconds());
        srand(seed);            

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	Ice::ThreadPtr thread = rawThread;

	//
	// See the comment in Ice::Thread::start() for details.
	//
	rawThread->__decRef();
	thread->run();
    }
    catch(const Ice::Exception& e)
    {
	fprintf(stderr, "Ice::Thread::run(): uncaught exception: %s\n", e.toString());
    }
    return 0;
}

#ifndef _WIN32_WCE
#include <process.h>
#endif

Ice::ThreadControl
Ice::Thread::start(size_t stackSize)
{
    //
    // Keep this alive for the duration of start
    //
    Ice::ThreadPtr keepMe = this;

    Ice::Mutex::Lock lock(_stateMutex);

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

#ifndef _WIN32_WCE
    _handle->handle = (HANDLE)_beginthreadex(
	0, stackSize, (unsigned int (__stdcall*)(void*))startHook, (LPVOID)this, 0, &_id);
#else
    _handle->handle = CreateThread(
	0, stackSize, (unsigned long (__stdcall*)(void*))startHook, (LPVOID)this, 0, &_id);
#endif
    if(_handle->handle == 0)
    {
	__decRef();
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }

    _started = true;
			
    return ThreadControl(_handle, _id);
}

Ice::ThreadControl
Ice::Thread::getThreadControl() const
{
    Ice::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_handle, _id);
}

bool
Ice::Thread::operator==(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    Ice::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }

    //
    // We perform the comparison within the scope of the lock, otherwise the hardware has no
    // way of knowing that it might have to flush a cache line.
    //
    return _id == id;
}

bool
Ice::Thread::operator!=(const Thread& rhs) const
{
    return !operator==(rhs);
}

bool
Ice::Thread::operator<(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    Ice::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }

    //
    // We perform the comparison within the scope of the lock, otherwise the hardware has no
    // way of knowing that it might have to flush a cache line.
    //
    return _id < id;
}

#else

Ice::ThreadControl::ThreadControl(ThreadId id)
{
    Ice::Mutex::Lock lock(_stateMutex);
    _id = id;
}

Ice::ThreadControl::ThreadControl()
{
    Ice::Mutex::Lock lock(_stateMutex);
    _id = pthread_self();
}

Ice::ThreadControl::ThreadControl(const ThreadControl& tc)
{
    ThreadId id = tc.id();
    Ice::Mutex::Lock lock(_stateMutex);
    _id = id;
}

Ice::ThreadControl&
Ice::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	ThreadId id = rhs.id();
	Ice::Mutex::Lock lock(_stateMutex);
	_id = id;
    }
    return *this;
}

bool
Ice::ThreadControl::operator==(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    Ice::Mutex::Lock lock(_stateMutex);
    return pthread_equal(_id, id);
}

bool
Ice::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

bool
Ice::ThreadControl::operator<(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    Ice::Mutex::Lock lock(_stateMutex);
    // NOTE: Linux specific
    return _id < id;
}

Ice::ThreadId
Ice::ThreadControl::id() const
{
    Ice::Mutex::Lock lock(_stateMutex);
    return _id;
}

void
Ice::ThreadControl::join()
{
    ThreadId id;
    {
	Ice::Mutex::Lock lock(_stateMutex);
	id = _id;
    }
    void* ignore = 0;
    int rc = pthread_join(id, &ignore);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
Ice::ThreadControl::detach()
{
    ThreadId id;
    {
	Ice::Mutex::Lock lock(_stateMutex);
	id = _id;
    }
    int rc = pthread_detach(id);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

bool
Ice::ThreadControl::isAlive() const
{
    int policy;
    int ret;
    struct sched_param param;
    Ice::Mutex::Lock lock(_stateMutex);

    ret = pthread_getschedparam(_id, &policy, &param);
#ifdef __APPLE__
    if (ret == 0) 
    {
	ret = pthread_setschedparam(_id, policy, &param);
    }
#endif 
    return (ret == 0);
}

void
Ice::ThreadControl::sleep(const Time& timeout)
{
    struct timeval tv = timeout;
    struct timespec ts;
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000L;
    nanosleep(&ts, 0);
}

void
Ice::ThreadControl::yield()
{
    sched_yield();
}

Ice::Thread::Thread()
{
    Ice::Mutex::Lock lock(_stateMutex);
    _started = false;
    _id = 0;
}

Ice::Thread::~Thread()
{
}

Ice::ThreadId
Ice::Thread::id() const
{
    Ice::Mutex::Lock lock(_stateMutex);
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
	Ice::Thread* rawThread = static_cast<Ice::Thread*>(arg);

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	Ice::ThreadPtr thread = rawThread;

	//
	// See the comment in Ice::Thread::start() for details.
	//
	rawThread->__decRef();
	thread->run();
    }
    catch(const Ice::Exception& e)
    {
	//fprintf(stderr, "Ice::Thread::run(): uncaught exception: %s\n", e.toString());
    }
    catch(...)
    {
	fprintf(stderr, "Ice::Thread::run(): uncaught exception\n");
    }
    return 0;
}
}

Ice::ThreadControl
Ice::Thread::start(size_t stackSize)
{
    //
    // Keep this alive for the duration of start
    //
    Ice::ThreadPtr keepMe = this;

    Ice::Mutex::Lock lock(_stateMutex);

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

    if(stackSize > 0)
    {
	pthread_attr_t attr;
	int rc = pthread_attr_init(&attr);
	if(rc != 0)
	{
	    __decRef();
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
	rc = pthread_attr_setstacksize(&attr, stackSize);
	if(rc != 0)
	{
	    __decRef();
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
	rc = pthread_create(&_id, &attr, startHook, this);
	if(rc != 0)
	{
	    __decRef();
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    }
    else
    {
	int rc = pthread_create(&_id, 0, startHook, this);
	if(rc != 0)
	{
	    __decRef();
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    }

    _started = true;

    return ThreadControl(_id);
}

Ice::ThreadControl
Ice::Thread::getThreadControl() const
{
    Ice::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_id);
}

bool
Ice::Thread::operator==(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    Ice::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }

    //
    // We perform the comparison within the scope of the lock, otherwise the hardware has no
    // way of knowing that it might have to flush a cache line.
    //
    return pthread_equal(_id, id);
}

bool
Ice::Thread::operator!=(const Thread& rhs) const
{
    return !operator==(rhs);
}

bool
Ice::Thread::operator<(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    Ice::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }

    //
    // We perform the comparison within the scope of the lock, otherwise the hardware has no
    // way of knowing that it might have to flush a cache line.
    //
    // NOTE: Linux specific
    //
    return _id < id;
}

#endif
