// **********************************************************************
//
// Copyright (c) 2002
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

#include <MonitorMutexTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

class MonitorMutexTestThread : public Thread
{
public:
    
    MonitorMutexTestThread(Monitor<Mutex>& m) :
	_monitor(m),
	_trylock(false)
    {
    }

    virtual void run()
    {
	try
	{
	    Monitor<Mutex>::TryLock lock(_monitor);
	    test(false);
	}
	catch(const ThreadLockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

	Monitor<Mutex>::Lock lock(_monitor);
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

    Monitor<Mutex>& _monitor;
    bool _trylock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _trylockCond;
    Mutex _trylockMutex;
};

typedef Handle<MonitorMutexTestThread> MonitorMutexTestThreadPtr;

class MonitorMutexTestThread2 : public Thread
{
public:
    
    MonitorMutexTestThread2(Monitor<Mutex>& monitor) :
	finished(false),
	_monitor(monitor)
    {
    }

    virtual void run()
    {
	Monitor<Mutex>::Lock lock(_monitor);
	_monitor.wait();
	finished = true;
    }

    bool finished;

private:

    Monitor<Mutex>& _monitor;
};

typedef Handle<MonitorMutexTestThread2> MonitorMutexTestThread2Ptr;

static const string monitorMutexTestName("monitor<mutex>");

MonitorMutexTest::MonitorMutexTest() :
    TestBase(monitorMutexTestName)
{
}

void
MonitorMutexTest::run()
{
    Monitor<Mutex> monitor;
    MonitorMutexTestThreadPtr t;
    MonitorMutexTestThread2Ptr t2;
    MonitorMutexTestThread2Ptr t3;
    ThreadControl control;
    ThreadControl control2;

    {
	Monitor<Mutex>::Lock lock(monitor);
	
	// TEST: TryLock
	try
	{
	    Monitor<Mutex>::TryLock lock(monitor);
	    test(false);
	}
	catch(const ThreadLockedException&)
	{
	    // Expected
	}
	
	// TEST: Start thread, try to acquire the mutex.
	t = new MonitorMutexTestThread(monitor);
	control = t->start();
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();

    // TEST: notify() wakes one consumer.
    t2 = new MonitorMutexTestThread2(monitor);
    control = t2->start();
    t3 = new MonitorMutexTestThread2(monitor);
    control2 = t3->start();

    // Give the thread time to start waiting.
    ThreadControl::sleep(Time::seconds(1));
    
    {
	Monitor<Mutex>::Lock lock(monitor);
	monitor.notify();
    }

    // Give one thread time to terminate
    ThreadControl::sleep(Time::seconds(1));

    test((t2->finished && !t3->finished) || (t3->finished && !t2->finished));

    {
	Monitor<Mutex>::Lock lock(monitor);
	monitor.notify();
    }
    control.join();
    control2.join();

    // TEST: notifyAll() wakes one consumer.
    t2 = new MonitorMutexTestThread2(monitor);
    control = t2->start();
    t3 = new MonitorMutexTestThread2(monitor);
    control2 = t3->start();

    // Give the threads time to start waiting.
    ThreadControl::sleep(Time::seconds(1));

    {
	Monitor<Mutex>::Lock lock(monitor);
	monitor.notifyAll();
    }

    control.join();
    control2.join();

    // TEST: timedWait
    {
	Monitor<Mutex>::Lock lock(monitor);
	test(!monitor.timedWait(Time::milliSeconds(500)));
    }
}
