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

#include <MutexTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

static const string mutexTestName("mutex");

class MutexTestThread : public Thread
{
public:
    
    MutexTestThread(Mutex& m) :
	_mutex(m),
	_trylock(false)
    {
    }

    virtual void run()
    {
	try
	{
	    Mutex::TryLock lock(_mutex);
	    test(false);
	}
	catch(const ThreadLockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

	Mutex::Lock lock(_mutex);
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

    Mutex& _mutex;
    bool _trylock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _trylockCond;
    Mutex _trylockMutex;
};

typedef Handle<MutexTestThread> MutexTestThreadPtr;

MutexTest::MutexTest() :
    TestBase(mutexTestName)
{
}

void
MutexTest::run()
{
    Mutex mutex;
    MutexTestThreadPtr t;
    ThreadControl control;

    {
	Mutex::Lock lock(mutex);
	
	// TEST: TryLock
	try
	{
	    Mutex::TryLock lock2(mutex);
	    test(false);
	}
	catch(const ThreadLockedException&)
	{
	    // Expected
	}
	
	// TEST: Start thread, try to acquire the mutex.
	t = new MutexTestThread(mutex);
	control = t->start();
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();
}
