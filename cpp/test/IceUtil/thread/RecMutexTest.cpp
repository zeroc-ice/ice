// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>

#include <RecMutexTest.h>
#include <TestHelper.h>

using namespace std;
using namespace IceUtil;

static const string recMutexTestName("recursive mutex");

class RecMutexTestThread : public Thread
{
public:

    RecMutexTestThread(RecMutex& m) :
        _mutex(m),
        _tryLock(false)
    {
    }

    virtual void run()
    {

        RecMutex::TryLock tlock(_mutex);
        test(!tlock.acquired());

        {
            Mutex::Lock lock(_tryLockMutex);
            _tryLock = true;
        }
        _tryLockCond.signal();

        RecMutex::Lock lock(_mutex);
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

    RecMutex& _mutex;
    bool _tryLock;
    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _tryLockCond;
    Mutex _tryLockMutex;
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

        // TEST: Wait until the tryLock has been tested.
        t->waitTryLock();

    }

    //
    // TEST: Once the recursive mutex has been released, the thread
    // should acquire the mutex and then terminate.
    //

    control.join();

}
