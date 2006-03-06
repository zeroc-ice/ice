// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IceE.h>
#include <IceE/RWRecMutex.h>

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
}
