// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
        _tryLock(false)
    {
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

protected:

    RWRecMutex& _mutex;
    bool _tryLock;

    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _tryLockCond;
    Mutex _tryLockMutex;
};

class RWRecMutexReadTestThread : public RWRecMutexTestThread
{
public:
    
    RWRecMutexReadTestThread(RWRecMutex& m) :
        RWRecMutexTestThread(m)
    {
    }

    virtual void
    run()
    {
        RWRecMutex::TryRLock tlock(_mutex);
        test(tlock.acquired());
        
        {
            Mutex::Lock lock(_tryLockMutex);
            _tryLock = true;
        }
        _tryLockCond.signal();

        RWRecMutex::RLock lock(_mutex);
    }
};

class RWRecMutexReadTestThread2 : public RWRecMutexTestThread
{
public:
    
    RWRecMutexReadTestThread2(RWRecMutex& m) :
        RWRecMutexTestThread(m)
    {
    }

    virtual void
    run()
    {
        
        RWRecMutex::TryRLock tlock(_mutex);
        test(!tlock.acquired());

        {
            Mutex::Lock lock(_tryLockMutex);
            _tryLock = true;
        }
        _tryLockCond.signal();

        RWRecMutex::RLock lock(_mutex);
    }
};

class RWRecMutexUpgradeReadThread : public Thread
{
public:
    
    RWRecMutexUpgradeReadThread(RWRecMutex& m) :
        _mutex(m),
        _unlock(false),
        _lock(false)
    {
    }
    virtual void
    run()
    {
        RWRecMutex::RLock lock(_mutex);

        signalLock();
        waitUnlock();
    }

    void
    waitUnlock()
    {
        Mutex::Lock lock(_unlockMutex);
        while(!_unlock)
        {
            _unlockCond.wait(lock);
        }
    }

    void
    signalUnlock()
    {
        Mutex::Lock lock(_unlockMutex);
        _unlock = true;
        _unlockCond.signal();
    }

    void
    signalLock()
    {
        Mutex::Lock lock(_lockMutex);
        _lock = true;
        _lockCond.signal();
    }

    void
    waitLock()
    {
        Mutex::Lock lock(_lockMutex);
        while(!_lock)
        {
            _lockCond.wait(lock);
        }
    }

private:

    RWRecMutex& _mutex;

    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _unlockCond;
    Mutex _unlockMutex;
    bool _unlock;

    Cond _lockCond;
    Mutex _lockMutex;
    bool _lock;
};
typedef Handle<RWRecMutexUpgradeReadThread> RWRecMutexUpgradeReadThreadPtr;

class RWRecMutexUpgradeTestThread : public Thread
{
public:
    
    RWRecMutexUpgradeTestThread(RWRecMutex& m) :
        _mutex(m),
        _lock(false),
        _upgradeAcquired(false)
    {
    }

    virtual void
    run()
    {
        RWRecMutex::RLock lock(_mutex);

        signalLock();
        lock.upgrade();

        _upgradeAcquired = true;
    }

    void
    signalLock()
    {
        Mutex::Lock lock(_lockMutex);
        _lock = true;
        _lockCond.signal();
    }

    void
    waitLock()
    {
        Mutex::Lock lock(_lockMutex);
        while(!_lock)
        {
            _lockCond.wait(lock);
        }
    }

    bool
    upgradeAcquired() const
    {
        return _upgradeAcquired;
    }

private:

    RWRecMutex& _mutex;

    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _lockCond;
    Mutex _lockMutex;
    bool _lock;

    bool _upgradeAcquired;
};
typedef Handle<RWRecMutexUpgradeTestThread> RWRecMutexUpgradeTestThreadPtr;

class RWRecMutexWriteTestThread : public RWRecMutexTestThread
{
public:
    
    RWRecMutexWriteTestThread(RWRecMutex& m) :
        RWRecMutexTestThread(m)
    {
    }

    virtual void
    run()
    {
        
        RWRecMutex::TryWLock tlock(_mutex);
        test(!tlock.acquired());

        {
            Mutex::Lock lock(_tryLockMutex);
            _tryLock = true;
        }
        _tryLockCond.signal();

        RWRecMutex::WLock lock(_mutex);
    }
};
typedef Handle<RWRecMutexTestThread> RWRecMutexTestThreadPtr;

class RWRecMutexUpgradeThread : public Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RWRecMutexUpgradeThread(RWRecMutex& m, bool timed = false)
        : _m(m),
          _timed(timed),
          _destroyed(false),
          _upgrading(false),
          _hasWriteLock(false),
          _failed(false)
    {
    }

    virtual void
    run()
    {
        //
        // Acquire a read lock.
        //
        RWRecMutex::RLock tlock(_m);

        {
            Lock sync(*this);
            _upgrading = true;
            notify();
        }

        try
        {
            if(_timed)
            {
                if(!_m.timedUpgrade(IceUtil::Time::seconds(10)))
                {
                    _failed = true;
                }
            }
            else
            {
                _m.upgrade();
            }
        }
        catch(DeadlockException&)
        {
            _failed = true;
        }

        {
            Lock sync(*this);
            _hasWriteLock = true;
            notify();

            while(!_destroyed)
            {
                wait();
            }
        }
    }

    void
    waitUpgrade()
    {
        Lock sync(*this);

        //
        // Wait for the _upgrading flag to be set.
        //
        while(!_upgrading)
        {
            wait();
        }
    }

    void
    destroy()
    {
        Lock sync(*this);
        _destroyed = true;
        notify();
    }

    //
    // This waits a maximum of N seconds if the lock is not already
    // acquired. It could while forever, but that would cause the test
    // to hang in the event of a failure.
    //
    bool
    waitHasWriteLock()
    {
        Lock sync(*this);
        if(!_hasWriteLock)
        {
            timedWait(Time::seconds(10));
        }
        return _hasWriteLock;
    }

    bool
    failed() const
    {
        return _failed;
    }

private:

    RWRecMutex& _m;
    bool _timed;
    bool _destroyed;
    bool _upgrading;
    bool _hasWriteLock;
    bool _failed;
};
typedef Handle<RWRecMutexUpgradeThread> RWRecMutexUpgradeThreadPtr;

class RWRecMutexWriteThread : public Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RWRecMutexWriteThread(RWRecMutex& m)
        : _m(m),
          _destroyed(false),
          _waitWrite(false),
          _hasWriteLock(false)
    {
    }

    virtual void
    run()
    {
        {
            Lock sync(*this);
            _waitWrite = true;
            notify();
        }
        //
        // Acquire a read lock.
        //
        RWRecMutex::WLock sync(_m);

        {
            Lock sync(*this);
            _hasWriteLock = true;
            notify();

            while(!_destroyed)
            {
                wait();
            }
        }
    }

    void
    waitWrite()
    {
        Lock sync(*this);

        //
        // Wait for the _upgrading flag to be set.
        //
        while(!_waitWrite)
        {
            wait();
        }

        //
        // Its necessary to sleep for 1 second to ensure that the
        // thread is actually IN the upgrade and waiting.
        //
        ThreadControl::sleep(Time::seconds(1));
    }

    void
    destroy()
    {
        Lock sync(*this);
        _destroyed = true;
        notify();
    }

    bool
    hasWriteLock()
    {
        Lock sync(*this);
        return _hasWriteLock;
    }

    //
    // This waits a maximum of N seconds if the lock is not already
    // acquired. It could while forever, but that would cause the test
    // to hang in the event of a failure.
    //
    bool
    waitHasWriteLock()
    {
        Lock sync(*this);
        if(!_hasWriteLock)
        {
            timedWait(Time::seconds(10));
        }
        return _hasWriteLock;
    }

private:

    RWRecMutex& _m;
    bool _destroyed;
    bool _waitWrite;
    bool _hasWriteLock;
};
typedef Handle<RWRecMutexWriteThread> RWRecMutexWriteThreadPtr;

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

        // RLock testing
        test(rlock.acquired());
        
        try
        {
            rlock.acquire();
            test(false);
        }
        catch(const ThreadLockedException&)
        {
            // Expected
        }

        try
        {
            rlock.tryAcquire();
            test(false);
        }
        catch(const ThreadLockedException&)
        {
            // Expected
        }

        test(rlock.acquired());
        rlock.release();
        test(!rlock.acquired());

        try
        {
            rlock.release();
            test(false);
        }
        catch(const ThreadLockedException&)
        {
            // Expected
        }
                
        test(rlock.tryAcquire() == true);
        test(rlock.acquired()); 

        RWRecMutex::RLock rlock2(mutex);
        
        RWRecMutex::TryRLock trlock(mutex);
        test(trlock.acquired());

        RWRecMutex::TryWLock twlock(mutex);
        test(!twlock.acquired());

        RWRecMutex::TryWLock twlock2(mutex, Time::milliSeconds(10));
        test(!twlock2.acquired());
    }

    // TEST: TryLock (write)
    {
        RWRecMutex::WLock wlock(mutex);

        // WLock testing
        test(wlock.acquired());
        
        try
        {
            wlock.acquire();
            test(false);
        }
        catch(const ThreadLockedException&)
        {
            // Expected
        }

        try
        {
            wlock.tryAcquire();
            test(false);
        }
        catch(const ThreadLockedException&)
        {
            // Expected
        }

        test(wlock.acquired());
        wlock.release();
        test(!wlock.acquired());

        try
        {
            wlock.release();
            test(false);
        }
        catch(const ThreadLockedException&)
        {
            // Expected
        }
                
        test(wlock.tryAcquire() == true);
        test(wlock.acquired()); 

        RWRecMutex::TryRLock trlock(mutex);
        test(!trlock.acquired());
        
        RWRecMutex::TryRLock trlock2(mutex, Time::milliSeconds(10));
        test(!trlock2.acquired());
        
        RWRecMutex::TryWLock twlock(mutex);
        test(twlock.acquired());
        
        RWRecMutex::TryWLock twlock2(mutex, Time::milliSeconds(10));
        test(twlock2.acquired());
    }

    // TEST: read lock
    {
        RWRecMutex::RLock rlock(mutex);

        // TEST: Start thread, try to acquire the mutex.
        t = new RWRecMutexReadTestThread(mutex);
        control = t->start();
        
        // TEST: Wait until the tryLock has been tested.
        t->waitTryLock();
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
        
        // TEST: Wait until the tryLock has been tested.
        t->waitTryLock();
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

        // TEST: Wait until the tryLock has been tested. The thread is
        // now waiting on a write lock.
        t->waitTryLock();

        // It's necessary for a small sleep here to ensure that the
        // thread is actually waiting on a write lock.
        ThreadControl::sleep(Time::seconds(1));

        RWRecMutex::TryRLock trlock(mutex);
        test(!trlock.acquired());
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();

    // TEST: Lock upgrading.
    {
        RWRecMutex::RLock rlock(mutex);

        //
        // Mutex now holds a write lock.
        //
        mutex.upgrade();

        // Start thread that tries to acquire write lock
        t = new RWRecMutexReadTestThread2(mutex);
        control = t->start();

        // TEST: Wait until the tryLock has been tested. The thread is
        // now waiting on a read lock.
        t->waitTryLock();

        // It's necessary for a small sleep here to ensure that the
        // thread is actually waiting on a read lock.
        ThreadControl::sleep(Time::seconds(1));
    }

    //
    // TEST: Once the mutex has been released, the thread should
    // acquire the mutex and then terminate.
    //
    control.join();

    //
    // COMPILERBUG: Under Linux with gcc this causes the Mutex test to
    // fail for some unknown reason. After spending more than enough
    // time looking into this problem it was decided that this is a
    // compiler bug of some sort.
    //
#if !(defined(__linux) && defined(__GNUC__))
    // TEST: Lock upgrading. This time a reader thread is started
    // first.
    {
        RWRecMutexUpgradeReadThreadPtr t1 = new RWRecMutexUpgradeReadThread(mutex);
        control = t1->start();

        // Wait for the thread to acquire the read lock.
        t1->waitLock();

        // Spawn a thread to try acquiring the lock
        RWRecMutexUpgradeTestThreadPtr t2 = new RWRecMutexUpgradeTestThread(mutex);
        ThreadControl control2 = t2->start();
        t2->waitLock();

        //
        // Small sleep to find out whether the thread actually
        // terminates (which means that the write lock upgrade was
        // mistakenly acquired).
        //
        ThreadControl::sleep(Time::seconds(1));

        test(!t2->upgradeAcquired());

        //
        // A read lock at this point should fail.
        //
        RWRecMutex::TryRLock trlock(mutex);
        test(!trlock.acquired());
        
        //
        // As should a write lock.
        //
        RWRecMutex::TryWLock twlock(mutex);
        test(!twlock.acquired());
        
        //
        // Once the read lock is released then the upgrade should
        // succeed & the thread should terminate.
        //
        t1->signalUnlock();

        control2.join();
        control.join();

        //
        // Now both a read & write lock should be available.
        //
        {
            RWRecMutex::WLock rlock2(mutex);
        }
        {
            RWRecMutex::RLock rlock2(mutex);
        }
    }
#endif

    // TEST: Ensure that only one reader can upgrade to a writer.
    // Other readers get a DeadlockException.
    {
        mutex.readLock();

        RWRecMutexUpgradeThreadPtr t1 = new RWRecMutexUpgradeThread(mutex);
        ThreadControl control1 = t1->start();

        //
        // Wait for the thread to get into the upgrade call. The
        // upgrade will hang since the readLock is held by this thread
        // and therefore cannot succeed until we release our read
        // lock.
        //
        t1->waitUpgrade();

        //
        // Its necessary to sleep for 1 second to ensure that the
        // thread is actually IN the upgrade and waiting.
        //
        ThreadControl::sleep(Time::seconds(1));

        try
        {
            mutex.upgrade();
            test(false);
        }
        catch(const DeadlockException&)
        {
        }

        //
        // Release the waiting thread, join.
        //
        mutex.unlock();
        t1->destroy();
        control1.join();

        test(!t1->failed());
    }

    // TEST: Same as previous test, but for a timedUpgrade.
    {
        mutex.readLock();

        RWRecMutexUpgradeThreadPtr t1 = new RWRecMutexUpgradeThread(mutex, true);
        ThreadControl control1 = t1->start();

        t1->waitUpgrade();
        //
        // Its necessary to sleep for 1 second to ensure that the
        // thread is actually IN the upgrade and waiting.
        //
        ThreadControl::sleep(Time::seconds(1));

        try
        {
            mutex.upgrade();
            test(false);
        }
        catch(const DeadlockException&)
        {
        }

        //
        // Release the waiting thread, join.
        //
        mutex.unlock();
        t1->destroy();
        control1.join();

        test(!t1->failed());
    }

    // TEST: Check that an upgrader is given preference over a writer.
    {
        mutex.readLock();

        RWRecMutexUpgradeThreadPtr t1 = new RWRecMutexUpgradeThread(mutex);
        
        ThreadControl control1 = t1->start();

        //
        // Its not necessary to sleep here, since the upgrade thread
        // acquires the read lock before signalling. Therefore the
        // write thread cannot get the write lock.
        //
        t1->waitUpgrade();

        RWRecMutexWriteThreadPtr t2 = new RWRecMutexWriteThread(mutex);
        ThreadControl control2 = t2->start();
        t2->waitWrite();
        //
        // Its necessary to sleep for 1 second to ensure that the
        // thread is actually IN the write lock and waiting.
        //
        ThreadControl::sleep(Time::seconds(1));

        //
        // Unlocking the read mutex lets the upgrade continue. At this
        // point t1 should have the write-lock, and t2 should not.
        //
        test(!t2->hasWriteLock());
        mutex.unlock();

        //
        // Wait for t1 to get the write lock. It will not release it
        // until the thread is destroyed. t2 should not have the write
        // lock.
        //
        test(!t1->failed());
        test(t1->waitHasWriteLock());
        test(!t2->hasWriteLock());
        t1->destroy();
        t2->destroy();

        //
        // After the thread has terminated the thread must have
        // acquired the write lock.
        //
        test(t2->waitHasWriteLock());

        control1.join();
        control2.join();
    }
}
