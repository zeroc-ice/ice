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

#include <RecMutexTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string recMutexTestName("recursive mutex");

class RecMutexTestThread : public Thread
{
public:
    
    RecMutexTestThread(RecMutex& m) :
	_mutex(m),
	_trylock(false)
    {
    }

    virtual void run()
    {
	try
	{
	    RecMutex::TryLock lock(_mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

	RecMutex::Lock lock(_mutex);
    }

    void
    waitTrylock()
    {
	Mutex::Lock lock(_trylockMutex);
	while(!_trylock)
	{
	    _trylockCond.wait(lock);
	}
    }

private:

    RecMutex& _mutex;
    bool _trylock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _trylockCond;
    Mutex _trylockMutex;
};

typedef Handle<RecMutexTestThread> RecMutexTestThreadPtr;

RecMutexTest::RecMutexTest() :
    TestBase(recMutexTestName)
{
}

void
RecMutexTest::run()
{
    RecMutex mutex;
    RecMutexTestThreadPtr t;
    ThreadControl control;

    {
	RecMutex::Lock lock(mutex);
	
	// TEST: lock twice
	RecMutex::Lock lock2(mutex);
	
	// TEST: TryLock
	RecMutex::TryLock lock3(mutex);
	
	// TEST: Start thread, try to acquire the mutex.
	t = new RecMutexTestThread(mutex);
	control = t->start();
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
    }

    //
    // TEST: Once the recursive mutex has been released, the thread
    // should acquire the mutex and then terminate.
    //
    control.join();
}
