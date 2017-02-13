// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
        _tryLock(false)
    {
    }

    virtual void run()
    {
        
        Monitor<RecMutex>::TryLock tlock(_monitor);
        test(!tlock.acquired());
        
        {
            Mutex::Lock lock(_tryLockMutex);
            _tryLock = true;
        }
        _tryLockCond.signal();

        Monitor<RecMutex>::Lock lock(_monitor);
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

    Monitor<RecMutex>& _monitor;
    bool _tryLock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _tryLockCond;
    Mutex _tryLockMutex;
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

static const string monitorRecMutexTestName("monitor<recmutex>");

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
        test(lock2.acquired());
        
        // TEST: TryLock
        
        Monitor<RecMutex>::TryLock tlock(monitor);
        test(tlock.acquired());
        
        // TEST: Start thread, try to acquire the mutex.
        t = new MonitorRecMutexTestThread(monitor);
        control = t->start();
        
        // TEST: Wait until the tryLock has been tested.
        t->waitTryLock();
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
    ThreadControl::sleep(Time::seconds(1));

    {
        Monitor<RecMutex>::Lock lock(monitor);
        monitor.notify();
    }

    // Give one thread time to terminate
    ThreadControl::sleep(Time::seconds(1));

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
    ThreadControl::sleep(Time::seconds(1));

    {
        Monitor<RecMutex>::Lock lock(monitor);
        monitor.notifyAll();
    }

    control.join();
    control2.join();

    // TEST: timedWait
    {
        Monitor<RecMutex>::Lock lock(monitor);
        test(!monitor.timedWait(Time::milliSeconds(500)));
    }
}
