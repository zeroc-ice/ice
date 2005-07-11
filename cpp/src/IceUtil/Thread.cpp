// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Time.h>
#include <IceUtil/ThreadException.h>

using namespace std;

#ifdef _WIN32

IceUtil::ThreadControl::ThreadControl()
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _handle = new HandleWrapper(GetCurrentThread(), false);
    _id = GetCurrentThreadId();
}

IceUtil::ThreadControl::ThreadControl(const HandleWrapperPtr& handle, ThreadId id)
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _handle = handle;
    _id = id;
}

IceUtil::ThreadControl::ThreadControl(const ThreadControl& tc)
{
    ThreadId id;
    HandleWrapperPtr handle;
    {
	IceUtil::Mutex::Lock lock(tc._stateMutex);
	id = tc._id;
	handle = tc._handle;
    }
    IceUtil::Mutex::Lock lock(_stateMutex);
    _handle = handle;
    _id = id;
}

IceUtil::ThreadControl&
IceUtil::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	ThreadId id;
	HandleWrapperPtr handle;
	{
	    IceUtil::Mutex::Lock lock(rhs._stateMutex);
	    handle = rhs._handle;
	    id = rhs._id;
	}
	IceUtil::Mutex::Lock lock(_stateMutex);
	_handle = handle;
	_id = id;
    }
    return *this;
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _id == id;
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

bool
IceUtil::ThreadControl::operator<(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _id < id;
}

IceUtil::ThreadId
IceUtil::ThreadControl::id() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _id;
}

void
IceUtil::ThreadControl::join()
{
    HandleWrapperPtr handle;
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	handle = _handle;
    }
    int rc = WaitForSingleObject(handle->handle, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

void
IceUtil::ThreadControl::detach()
{
    // No-op: Windows doesn't have the concept of detaching a thread.
}

bool
IceUtil::ThreadControl::isAlive() const
{
    HandleWrapperPtr handle;
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
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
IceUtil::ThreadControl::sleep(const Time& timeout)
{
    Sleep(static_cast<long>(timeout.toMilliSeconds()));
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

IceUtil::Thread::Thread()
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _started = false;
    _id = 0;
    _handle = new HandleWrapper(0);
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

static void*
startHook(void* arg)
{
    try
    {
	IceUtil::Thread* rawThread = static_cast<IceUtil::Thread*>(arg);

        //
        // Initialize the random number generator in each thread.
        //
        unsigned int seed = static_cast<unsigned int>(IceUtil::Time::now().toMicroSeconds());
        srand(seed);            
#ifndef _WIN32
	srand48(seed);
#endif

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
IceUtil::Thread::start(size_t stackSize)
{
    //
    // Keep this alive for the duration of start
    //
    IceUtil::ThreadPtr keepMe = this;

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
    
    _handle->handle = (HANDLE)_beginthreadex(
	0, stackSize, (unsigned int (__stdcall*)(void*))startHook, (LPVOID)this, 0, &_id);

    if(_handle->handle == 0)
    {
	__decRef();
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
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
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceUtil::Mutex::Lock lock(_stateMutex);
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
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    return !operator==(rhs);
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceUtil::Mutex::Lock lock(_stateMutex);
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

IceUtil::ThreadControl::ThreadControl(ThreadId id)
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _id = id;
}

IceUtil::ThreadControl::ThreadControl()
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _id = pthread_self();
}

IceUtil::ThreadControl::ThreadControl(const ThreadControl& tc)
{
    ThreadId id = tc.id();
    IceUtil::Mutex::Lock lock(_stateMutex);
    _id = id;
}

IceUtil::ThreadControl&
IceUtil::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	ThreadId id = rhs.id();
	IceUtil::Mutex::Lock lock(_stateMutex);
	_id = id;
    }
    return *this;
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceUtil::Mutex::Lock lock(_stateMutex);
    return pthread_equal(_id, id);
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

bool
IceUtil::ThreadControl::operator<(const ThreadControl& rhs) const
{
    ThreadId id = rhs.id();
    IceUtil::Mutex::Lock lock(_stateMutex);
    // NOTE: Linux specific
    return _id < id;
}

IceUtil::ThreadId
IceUtil::ThreadControl::id() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _id;
}

void
IceUtil::ThreadControl::join()
{
    ThreadId id;
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
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
IceUtil::ThreadControl::detach()
{
    ThreadId id;
    {
	IceUtil::Mutex::Lock lock(_stateMutex);
	id = _id;
    }
    int rc = pthread_detach(id);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

bool
IceUtil::ThreadControl::isAlive() const
{
    int policy;
    int ret;
    struct sched_param param;
    IceUtil::Mutex::Lock lock(_stateMutex);

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

IceUtil::Thread::Thread()
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _started = false;
    _id = 0;
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
    catch(...)
    {
	cerr << "IceUtil::Thread::run(): uncaught exception" << endl;
    }
    return 0;
}
}

IceUtil::ThreadControl
IceUtil::Thread::start(size_t stackSize)
{
    //
    // Keep this alive for the duration of start
    //
    IceUtil::ThreadPtr keepMe = this;

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
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceUtil::Mutex::Lock lock(_stateMutex);
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
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    return !operator==(rhs);
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    //
    // Get rhs ID.
    //
    ThreadId id = rhs.id();

    //
    // Check that this thread was started.
    //
    IceUtil::Mutex::Lock lock(_stateMutex);
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
