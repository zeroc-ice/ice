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
	
	RecMutex::TryLock tlock(_mutex);
	test(!tlock.acquired());
	
	{
	    Mutex::Lock lock(_trylockMutex);
	    _trylock = true;
	}
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
	test(lock3.acquired());
	
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
