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

IceUtil::ThreadControl::ThreadControl() :
    _handle(0),
    _id(GetCurrentThreadId())
{
    HANDLE currentThread = GetCurrentThread();
    HANDLE currentProcess = GetCurrentProcess();   

    if(DuplicateHandle(currentProcess,
		       currentThread,
		       currentProcess,
		       &_handle,
		       0,
		       FALSE,
		       DUPLICATE_SAME_ACCESS) == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

IceUtil::ThreadControl::ThreadControl(HANDLE handle, DWORD id) :
    _id(id)
{
    HANDLE currentProcess = GetCurrentProcess();    

    if(DuplicateHandle(currentProcess,
		       handle,
		       currentProcess,
		       &_handle,
		       0,
		       FALSE,
		       DUPLICATE_SAME_ACCESS) == 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}
  
IceUtil::ThreadControl::ThreadControl(const ThreadControl& tc) :
    _handle(0)
{
    operator=(tc);
}

IceUtil::ThreadControl::~ThreadControl()
{
    assert(_handle != 0);
    bool ok = CloseHandle(_handle);
    assert(ok);
}

IceUtil::ThreadControl&
IceUtil::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	if(_handle != 0 && CloseHandle(_handle) == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}	
	_id = rhs._id;

	HANDLE currentProcess = GetCurrentProcess(); 
	if(DuplicateHandle(currentProcess,
			   rhs._handle,
			   currentProcess,
			   &_handle,
			   0,
			   FALSE,
			   DUPLICATE_SAME_ACCESS) == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
    }
    return *this;
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


void
IceUtil::ThreadControl::join()
{
    int rc = WaitForSingleObject(_handle, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

void
IceUtil::ThreadControl::detach()
{
    // On Windows, a thread is cleaned up when the last handle to this thread
    // is closed.
}

DWORD
IceUtil::ThreadControl::id() const
{
    return _id;
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

IceUtil::Thread::Thread() :
    _started(false),
    _running(false),
    _handle(0),
    _id(0)
{
}

IceUtil::Thread::~Thread()
{
    if(_handle != 0)
    {
	bool ok = CloseHandle(_handle);
	assert(ok);
    }
}

static unsigned int
WINAPI startHook(void* arg)
{
    // Ensure that the thread doesn't go away until run() has
    // completed.
    //
    IceUtil::ThreadPtr thread;

    try
    {
	IceUtil::Thread* rawThread = static_cast<IceUtil::Thread*>(arg);

        //
        // Initialize the random number generator in each thread.
        //
        unsigned int seed = static_cast<unsigned int>(IceUtil::Time::now().toMicroSeconds());
        srand(seed);            

	//
	// Ensure that the thread doesn't go away until run() has
	// completed.
	//
	thread = rawThread;

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
    thread->_done();
   
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
    
    unsigned int id;
    
    _handle = 
       reinterpret_cast<HANDLE>(
          _beginthreadex(0, 
			 static_cast<unsigned int>(stackSize), 
			 startHook, this, 0, &id));
    _id = id;

    if(_handle == 0)
    {
	__decRef();
	throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }

    _started = true;
    _running = true;
			
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
    return this == &rhs;
}

bool
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    return this != &rhs;
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    return this < &rhs;
}

bool
IceUtil::Thread::isAlive() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _running;
}

void
IceUtil::Thread::_done()
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _running = false;
}


#else

IceUtil::ThreadControl::ThreadControl(pthread_t thread) :
    _thread(thread)
{
}

IceUtil::ThreadControl::ThreadControl() :
    _thread(pthread_self())
{
}

IceUtil::ThreadControl::ThreadControl(const ThreadControl& tc) :
    _thread(tc._thread)
{
}

IceUtil::ThreadControl::~ThreadControl()
{
}

IceUtil::ThreadControl&
IceUtil::ThreadControl::operator=(const ThreadControl& rhs)
{
    if(&rhs != this)
    {
	_thread = rhs._thread;
    }
    return *this;
}

bool
IceUtil::ThreadControl::operator==(const ThreadControl& rhs) const
{
    return pthread_equal(_thread, rhs._thread) != 0;
}

bool
IceUtil::ThreadControl::operator!=(const ThreadControl& rhs) const
{
    return !operator==(rhs);
}

void
IceUtil::ThreadControl::join()
{
    void* ignore = 0;
    int rc = pthread_join(_thread, &ignore);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
IceUtil::ThreadControl::detach()
{
    int rc = pthread_detach(_thread);
    if(rc != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

pthread_t
IceUtil::ThreadControl::id() const
{
    return _thread;;
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
    _running(false)
{
}

IceUtil::Thread::~Thread()
{
}


extern "C" 
{
static void*
startHook(void* arg)
{
    //
    // Ensure that the thread doesn't go away until run() has
    // completed.
    //
    IceUtil::ThreadPtr thread;

    try
    {
	IceUtil::Thread* rawThread = static_cast<IceUtil::Thread*>(arg);

	thread = rawThread;

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
    thread->_done();
    
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
	rc = pthread_create(&_thread, &attr, startHook, this);
	if(rc != 0)
	{
	    __decRef();
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    }
    else
    {
	int rc = pthread_create(&_thread, 0, startHook, this);
	if(rc != 0)
	{
	    __decRef();
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    }

    _started = true;
    _running = true;

    return ThreadControl(_thread);
}

IceUtil::ThreadControl
IceUtil::Thread::getThreadControl() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    if(!_started)
    {
	throw ThreadNotStartedException(__FILE__, __LINE__);
    }
    return ThreadControl(_thread);
}

bool
IceUtil::Thread::operator==(const Thread& rhs) const
{
    return this == &rhs;
}

bool
IceUtil::Thread::operator!=(const Thread& rhs) const
{
    return this != &rhs;
}

bool
IceUtil::Thread::operator<(const Thread& rhs) const
{
    return this < &rhs;
}

bool
IceUtil::Thread::isAlive() const
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    return _running;
}

void
IceUtil::Thread::_done()
{
    IceUtil::Mutex::Lock lock(_stateMutex);
    _running = false;
}

#endif
