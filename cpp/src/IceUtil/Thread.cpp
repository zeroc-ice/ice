// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Exception.h>

using namespace std;

#ifdef _WIN32

IceUtil::ThreadControl::ThreadControl() :
    _handle(new HandleWrapper(0)),
    _id(GetCurrentThreadId())
{
    HANDLE proc = GetCurrentProcess();
    HANDLE current = GetCurrentThread();
    int rc = DuplicateHandle(proc, current, proc, &_handle->handle, SYNCHRONIZE, TRUE, 0);
    if (rc == 0)
    {
	throw SyscallException(SyscallException::errorToString(GetLastError()), __FILE__, __LINE__);
    }
}

IceUtil::ThreadControl::ThreadControl(const HandleWrapperPtr& handle, unsigned id) :
    _handle(handle),
    _id(id)
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

void
IceUtil::ThreadControl::join()
{
    int rc = WaitForSingleObject(_handle->handle, INFINITE);
    if (rc != WAIT_OBJECT_0)
    {
	throw SyscallException(SyscallException::errorToString(GetLastError()), __FILE__, __LINE__);
    }
}

void
IceUtil::ThreadControl::sleep(long msec)
{
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
    _id(0),
    _handle(new HandleWrapper(0))
{
}

IceUtil::Thread::~Thread()
{
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
    
    _handle->handle = (HANDLE)_beginthreadex(0, 0, (unsigned (__stdcall*)(void*))startHook, (LPVOID)this, 0, &_id);
    if (_handle->handle == 0)
    {
	__decRef();
	throw SyscallException(SyscallException::errorToString(GetLastError()), __FILE__, __LINE__);
    }
			
    return ThreadControl(_handle, _id);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl()
{
    return ThreadControl(_handle, _id);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    return _id == rhs._id;
}

bool
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    return _id != rhs._id;
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    return _id < rhs._id;
}

#else

IceUtil::ThreadControl::ThreadControl(pthread_t id) :
    _id(id)
{
}

IceUtil::ThreadControl::ThreadControl() :
    _id(pthread_self())
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

void
IceUtil::ThreadControl::join()
{
    void* ignore = 0;
    int rc = pthread_join(_id, &ignore);
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
}

void
IceUtil::ThreadControl::sleep(long msec)
{
    struct timespec tv;
    tv.tv_sec = msec/1000;
    tv.tv_nsec = (msec % 1000)* 1000000;
    nanosleep(&tv, 0);
}

void
IceUtil::ThreadControl::yield()
{
    sched_yield();
}

IceUtil::Thread::Thread()
{
}

IceUtil::Thread::~Thread()
{
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

IceUtil::ThreadControl
IceUtil::Thread::start()
{
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
    if (rc != 0)
    {
	__decRef();
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
    return ThreadControl(_id);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl()
{
    return ThreadControl(_id);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    return pthread_equal(_id, rhs._id);
}

bool
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    return !pthread_equal(_id, rhs._id);
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    // NOTE: Linux specific
    return _id < rhs._id;
}

#endif

#ifdef never
void ice_atomic_inc(ice_atomic_t *v)
{
    __asm__ __volatile__(
	"lock ; incl %0"
	:"=m" (v->counter)
	:"m" (v->counter));
}

int ice_atomic_dec_and_test(ice_atomic_t *v)
{
    
    unsigned char c;
    __asm__ __volatile__(
	"lock ; decl %0; sete %1"
	:"=m" (v->counter), "=qm" (c)
	:"m" (v->counter) : "memory");
    return c != 0;
}

int ice_atomic_exchange_add(int i, ice_atomic_t* v)
{
    int tmp = i;
    __asm__ __volatile__(
	"lock ; xadd %0,(%2)"
	:"+r"(tmp), "=m"(v->counter)
	:"r"(v), "m"(v->counter)
	: "memory");
    return tmp + i;
}
#endif
