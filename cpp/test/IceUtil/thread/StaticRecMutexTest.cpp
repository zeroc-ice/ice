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

#include <IceUtil/IceUtil.h>

#include <StaticRecMutexTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string staticRecMutexTestName("static recursive mutex");

static StaticRecMutex staticMutex = ICE_STATIC_RECMUTEX_INITIALIZER;

class StaticRecMutexTestThread : public Thread
{
public:
    
    StaticRecMutexTestThread() :
	_tryLock(false)
    {
    }

    virtual void run()
    {
	
	StaticRecMutex::TryLock tlock(staticMutex);
	test(!tlock.acquired());
	
	{
	    Mutex::Lock lock(_tryLockMutex);
	    _tryLock = true;
	}
	_tryLockCond.signal();

	StaticRecMutex::Lock lock(staticMutex);
    }

    void
    waitTryLock()
    {
	Mutex::Lock lock(_tryLockMutex);
	while(!_tryLock)
	{
	    _tryLockCond.wait(lock);
	}
    }

private:

    bool _tryLock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _tryLockCond;
    Mutex _tryLockMutex;
};

typedef Handle<StaticRecMutexTestThread> StaticRecMutexTestThreadPtr;

StaticRecMutexTest::StaticRecMutexTest() :
    TestBase(staticRecMutexTestName)
{
}

void
StaticRecMutexTest::run()
{
    StaticRecMutexTestThreadPtr t;
    ThreadControl control;

    {
	StaticRecMutex::Lock lock(staticMutex);
	
	// TEST: lock twice
	StaticRecMutex::Lock lock2(staticMutex);

	// TEST: TryLock
	StaticRecMutex::TryLock lock3(staticMutex);
	test(lock3.acquired());
	
	// TEST: Start thread, try to acquire the mutex.
	t = new StaticRecMutexTestThread();
	control = t->start();
	
	// TEST: Wait until the tryLock has been tested.
	t->waitTryLock();
	
    }

    //
    // TEST: Once the recursive mutex has been released, the thread
    // should acquire the mutex and then terminate.
    //

    control.join();
   
}
