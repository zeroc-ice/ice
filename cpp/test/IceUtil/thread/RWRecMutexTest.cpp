// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <RWRecMutexTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const std::string rwRecMutexTestName("read/write recursive mutex");

class RWRecMutexTestThread : public Thread
{
public:
    
    RWRecMutexTestThread(RWRecMutex& m) :
	_mutex(m),
	_trylock(false)
    {
    }

    void
    waitTrylock()
    {
	Mutex::Lock lock(_trylockMutex);
	while (!_trylock)
	{
	    _trylockCond.wait(lock);
	}
    }

protected:

    RWRecMutex& _mutex;
    bool _trylock;

    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _trylockCond;
    Mutex _trylockMutex;
};

class RWRecMutexReadTestThread : public RWRecMutexTestThread
{
public:
    
    RWRecMutexReadTestThread(RWRecMutex& m) :
	RWRecMutexTestThread(m)
    {
    }

    virtual void run()
    {
	try
	{
	    RWRecMutex::TryRLock lock(_mutex);
	    // Expected
	}
	catch(const LockedException&)
	{
	    test(false);
	}

	_trylock = true;
	_trylockCond.signal();

	RWRecMutex::RLock lock(_mutex);
    }
};

class RWRecMutexWriteTestThread : public RWRecMutexTestThread
{
public:
    
    RWRecMutexWriteTestThread(RWRecMutex& m) :
	RWRecMutexTestThread(m)
    {
    }

    virtual void run()
    {
	try
	{
	    RWRecMutex::TryWLock lock(_mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

	RWRecMutex::WLock lock(_mutex);
    }
};

typedef Handle<RWRecMutexTestThread> RWRecMutexTestThreadPtr;

RWRecMutexTest::RWRecMutexTest() :
    TestBase(rwRecMutexTestName)
{
}

void
RWRecMutexTest::run()
{
    RWRecMutex mutex;
    RWRecMutexTestThreadPtr t;
    ThreadControl control;

    // TEST: TryLock (read)
    {
	RWRecMutex::RLock rlock(mutex);
	RWRecMutex::RLock rlock2(mutex);

	try
	{
	    RWRecMutex::TryRLock rlock2(mutex);
	}
	catch(const LockedException&)
	{
	    test(false);
	}

	try
	{
	    RWRecMutex::TryWLock wlock(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
    }

    // TEST: TryLock (write)
    {
	RWRecMutex::WLock wlock(mutex);
	
	// TEST: TryLock
	try
	{
	    RWRecMutex::TryRLock rlock(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
	try
	{
	    RWRecMutex::TryWLock wlock(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
    }

    // TEST: read lock
    {
	RWRecMutex::RLock rlock(mutex);

	// TEST: Start thread, try to acquire the mutex.
	t = new RWRecMutexReadTestThread(mutex);
	control = t->start();
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();

    // TEST: write lock
    {
	RWRecMutex::WLock wlock(mutex);

	// TEST: Start thread, try to acquire the mutex.
	t = new RWRecMutexWriteTestThread(mutex);
	control = t->start();
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();

    // TEST: Lock precedence. Writers have precedence over readers.
    {
	RWRecMutex::RLock rlock(mutex);

	// Start thread that tries to acquire write lock
	t = new RWRecMutexWriteTestThread(mutex);
	control = t->start();

	// TEST: Wait until the trylock has been tested. The thread is
	// now waiting on a write lock.
	t->waitTrylock();

	// It's necessary for a small sleep here to ensure that the
	// thread is actually waiting on a write lock.
	ThreadControl::sleep(1000);

	try
	{
	    RWRecMutex::TryRLock rlock2(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();
}
