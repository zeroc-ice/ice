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

IceE::ThreadControl::ThreadControl()
{
    IceE::Mutex::Lock lock(_stateMutex);
    _handle = new HandleWrapper(GetCurrentThread(), false);
    _id = GetCurrentThreadId();
}

IceE::ThreadControl::ThreadControl(const HandleWrapperPtr& handle, ThreadId id)
{
    IceE::Mutex::Lock lock(_stateMutex);
    _handle = handle;
    _id = id;
}

IceE::ThreadControl::ThreadControl(const ThreadControl& tc)
{
    ThreadId id;
    HandleWrapperPtr handle;
    {
	IceE::Mutex::Lock lock(tc._stateMutex);
	id = tc._id;
	handle = tc._handle;
    }
    IceE::Mutex::Lock lock(_stateMutex);
    _handle = handle;
    _id = id;
}

IceE::ThreadControl&
IceE::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	ThreadId id;
	HandleWrapperPtr handle;
	{
	    IceE::Mutex::Lock lock(rhs._stateMutex);
	    handle = rhs._handle;
	    id = rhs._id;
	}
	IceE::Mutex::Lock lock(_stateMutex);
	_handle = handle;
	_id = id;
    }
    return *this;
}

bool
IceE::ThreadControl::operator==(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceE::Mutex::Lock lock(_stateMutex);
    return _id == id;
}

bool
IceE::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

bool
IceE::ThreadControl::operator<(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceE::Mutex::Lock lock(_stateMutex);
    return _id < id;
}

IceE::ThreadId
IceE::ThreadControl::id() const
{
    IceE::Mutex::Lock lock(_stateMutex);
    return _id;
}

void
IceE::ThreadControl::join()
{
    HandleWrapperPtr handle;
    {
	IceE::Mutex::Lock lock(_stateMutex);
	handle = _handle;
    }
    int rc = WaitForSingleObject(handle->handle, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

void
IceE::ThreadControl::detach()
{
    // No-op: Windows doesn't have the concept of detaching a thread.
}

bool
IceE::ThreadControl::isAlive() const
{
    HandleWrapperPtr handle;
    {
	IceE::Mutex::Lock lock(_stateMutex);
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
IceE::ThreadControl::sleep(const Time& timeout)
{
    timeval tv = timeout;
    long msec = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
    Sleep(msec);
}

void
IceE::ThreadControl::yield()
{
    //
    // A value of zero causes the thread to relinquish the remainder
    // of its time slice to any other thread of equal priority that is
    // ready to run.
    //
    Sleep(0);
}

IceE::Thread::Thread()
{
    IceE::Mutex::Lock lock(_stateMutex);
    _started = false;
    _id = 0;
    _handle = new HandleWrapper(0);
}

IceE::Thread::~Thread()
{
}

IceE::ThreadId
IceE::Thread::id() const
{
    IceE::Mutex::Lock lock(_stateMutex);
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
	IceE::Thread* rawThread = static_cast<IceE::Thread*>(arg);

        //
        // Initialize the random number generator in each thread.
        //
        unsigned int seed = static_cast<unsigned int>(IceE::Time::now().toMicroSeconds());
        srand(seed);            

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	IceE::ThreadPtr thread = rawThread;

	//
	// See the comment in IceE::Thread::start() for details.
	//
	rawThread->__decRef();
	thread->run();
    }
    catch(const IceE::Exception& e)
    {
	fprintf(stderr, "IceE::Thread::run(): uncaught exception: %s\n", e.toString());
    }
    return 0;
}

#ifndef _WIN32_WCE
#include <process.h>
#endif

IceE::ThreadControl
IceE::Thread::start(size_t stackSize)
{
    //
    // Keep this alive for the duration of start
    //
    IceE::ThreadPtr keepMe = this;

    IceE::Mutex::Lock lock(_stateMutex);

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

IceE::ThreadControl
IceE::Thread::getThreadControl() const
{
    IceE::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_handle, _id);
}

bool
IceE::Thread::operator==(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceE::Mutex::Lock lock(_stateMutex);
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
IceE::Thread::operator!=(const Thread& rhs) const
{
    return !operator==(rhs);
}

bool
IceE::Thread::operator<(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceE::Mutex::Lock lock(_stateMutex);
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

IceE::ThreadControl::ThreadControl(ThreadId id)
{
    IceE::Mutex::Lock lock(_stateMutex);
    _id = id;
}

IceE::ThreadControl::ThreadControl()
{
    IceE::Mutex::Lock lock(_stateMutex);
    _id = pthread_self();
}

IceE::ThreadControl::ThreadControl(const ThreadControl& tc)
{
    ThreadId id = tc.id();
    IceE::Mutex::Lock lock(_stateMutex);
    _id = id;
}

IceE::ThreadControl&
IceE::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	ThreadId id = rhs.id();
	IceE::Mutex::Lock lock(_stateMutex);
	_id = id;
    }
    return *this;
}

bool
IceE::ThreadControl::operator==(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceE::Mutex::Lock lock(_stateMutex);
    return pthread_equal(_id, id);
}

bool
IceE::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

bool
IceE::ThreadControl::operator<(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceE::Mutex::Lock lock(_stateMutex);
    // NOTE: Linux specific
    return _id < id;
}

IceE::ThreadId
IceE::ThreadControl::id() const
{
    IceE::Mutex::Lock lock(_stateMutex);
    return _id;
}

void
IceE::ThreadControl::join()
{
    ThreadId id;
    {
	IceE::Mutex::Lock lock(_stateMutex);
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
IceE::ThreadControl::detach()
{
    ThreadId id;
    {
	IceE::Mutex::Lock lock(_stateMutex);
	id = _id;
    }
    int rc = pthread_detach(id);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

bool
IceE::ThreadControl::isAlive() const
{
    int policy;
    int ret;
    struct sched_param param;
    IceE::Mutex::Lock lock(_stateMutex);

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
IceE::ThreadControl::sleep(const Time& timeout)
{
    struct timeval tv = timeout;
    struct timespec ts;
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000L;
    nanosleep(&ts, 0);
}

void
IceE::ThreadControl::yield()
{
    sched_yield();
}

IceE::Thread::Thread()
{
    IceE::Mutex::Lock lock(_stateMutex);
    _started = false;
    _id = 0;
}

IceE::Thread::~Thread()
{
}

IceE::ThreadId
IceE::Thread::id() const
{
    IceE::Mutex::Lock lock(_stateMutex);
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
	IceE::Thread* rawThread = static_cast<IceE::Thread*>(arg);

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	IceE::ThreadPtr thread = rawThread;

	//
	// See the comment in IceE::Thread::start() for details.
	//
	rawThread->__decRef();
	thread->run();
    }
    catch(const IceE::Exception& e)
    {
	//fprintf(stderr, "IceE::Thread::run(): uncaught exception: %s\n", e.toString());
    }
    catch(...)
    {
	fprintf(stderr, "IceE::Thread::run(): uncaught exception\n");
    }
    return 0;
}
}

IceE::ThreadControl
IceE::Thread::start(size_t stackSize)
{
    //
    // Keep this alive for the duration of start
    //
    IceE::ThreadPtr keepMe = this;

    IceE::Mutex::Lock lock(_stateMutex);

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

IceE::ThreadControl
IceE::Thread::getThreadControl() const
{
    IceE::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_id);
}

bool
IceE::Thread::operator==(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceE::Mutex::Lock lock(_stateMutex);
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
IceE::Thread::operator!=(const Thread& rhs) const
{
    return !operator==(rhs);
}

bool
IceE::Thread::operator<(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceE::Mutex::Lock lock(_stateMutex);
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
