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

#include <MonitorRecMutexTest.h>
#include <TestCommon.h>

using namespace std;
using namespace IceUtil;

class MonitorRecMutexTestThread : public Thread
{
public:
    
    MonitorRecMutexTestThread(Monitor<RecMutex>& m) :
	_monitor(m),
	_trylock(false)
    {
    }

    virtual void run()
    {
	try
	{
	    Monitor<RecMutex>::TryLock lock(_monitor);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

	Monitor<RecMutex>::Lock lock(_monitor);
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

private:

    Monitor<RecMutex>& _monitor;
    bool _trylock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _trylockCond;
    Mutex _trylockMutex;
};

typedef Handle<MonitorRecMutexTestThread> MonitorRecMutexTestThreadPtr;

class MonitorRecMutexTestThread2 : public Thread, public Monitor<RecMutex>
{
public:
    
    MonitorRecMutexTestThread2(Monitor<RecMutex>& monitor) :
	finished(false),
	_monitor(monitor)
    {
    }

    virtual void run()
    {
	Monitor<RecMutex>::Lock lock(_monitor);
	_monitor.wait();
	finished = true;
    }

    bool finished;

private:

    Monitor<RecMutex>& _monitor;
};

typedef Handle<MonitorRecMutexTestThread2> MonitorRecMutexTestThread2Ptr;

static const std::string monitorRecMutexTestName("monitor<recmutex>");

MonitorRecMutexTest::MonitorRecMutexTest() :
    TestBase(monitorRecMutexTestName)
{
}

void
MonitorRecMutexTest::run()
{
    Monitor<RecMutex> monitor;
    MonitorRecMutexTestThreadPtr t;
    MonitorRecMutexTestThread2Ptr t2;
    MonitorRecMutexTestThread2Ptr t3;
    ThreadControl control;
    ThreadControl control2;


    {
	Monitor<RecMutex>::Lock lock(monitor);

	Monitor<RecMutex>::TryLock lock2(monitor);
	
	// TEST: TryLock
	try
	{
	    Monitor<RecMutex>::TryLock lock(monitor);
	}
	catch(const LockedException&)
	{
	    test(false);
	}
	
	// TEST: Start thread, try to acquire the mutex.
	t = new MonitorRecMutexTestThread(monitor);
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
    t2 = new MonitorRecMutexTestThread2(monitor);
    control = t2->start();
    t3 = new MonitorRecMutexTestThread2(monitor);
    control2 = t3->start();

    // Give the thread time to start waiting.
    ThreadControl::sleep(1000);

    {
	Monitor<RecMutex>::Lock lock(monitor);
	monitor.notify();
    }

    // Give one thread time to terminate
    ThreadControl::sleep(1000);

    test((t2->finished && !t3->finished) || (t3->finished && !t2->finished));

    {
	Monitor<RecMutex>::Lock lock(monitor);
	monitor.notify();
    }
    control.join();
    control2.join();

    // TEST: notifyAll() wakes one consumer.
    t2 = new MonitorRecMutexTestThread2(monitor);
    control = t2->start();
    t3 = new MonitorRecMutexTestThread2(monitor);
    control2 = t3->start();

    // Give the threads time to start waiting.
    ThreadControl::sleep(1000);

    {
	Monitor<RecMutex>::Lock lock(monitor);
	monitor.notifyAll();
    }

    control.join();
    control2.join();

    // TEST: timedwait
    {
	Monitor<RecMutex>::Lock lock(monitor);
	test(!monitor.timedwait(500));
    }
}
