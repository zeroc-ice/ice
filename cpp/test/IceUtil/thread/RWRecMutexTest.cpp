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
	_trylock(false)
    {
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

protected:

    RWRecMutex& _mutex;
    bool _trylock;

    //
    // Use native Condition variable here, not Monitor.
    //
    Cond _trylockCond;
    Mutex _trylockMutex;
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
	try
	{
	    RWRecMutex::TryRLock lock(_mutex);
	    // Expected
	}
	catch(const LockedException&)
	{
	    test(false);
	}

	_trylock = true;
	_trylockCond.signal();

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
	try
	{
	    RWRecMutex::TryRLock lock(_mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

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
	while (!_unlock)
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
	while (!_lock)
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
	while (!_lock)
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
	try
	{
	    RWRecMutex::TryWLock lock(_mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	_trylock = true;
	_trylockCond.signal();

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
	RWRecMutex::RLock rlock2(mutex);

	try
	{
	    RWRecMutex::TryRLock rlock2(mutex);
	}
	catch(const LockedException&)
	{
	    test(false);
	}

	try
	{
	    RWRecMutex::TryWLock wlock(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	try
	{
	    RWRecMutex::TryWLock wlock(mutex, Time::milliSeconds(10));
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
    }

    // TEST: TryLock (write)
    {
	RWRecMutex::WLock wlock(mutex);
	
	// TEST: TryLock
	try
	{
	    RWRecMutex::TryRLock rlock(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
	try
	{
	    RWRecMutex::TryRLock rlock(mutex, Time::milliSeconds(10));
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
	try
	{
	    RWRecMutex::TryWLock wlock(mutex);
	    // Expected
	}
	catch(const LockedException&)
	{
	    test(false);
	}
	try
	{
	    RWRecMutex::TryWLock wlock(mutex, Time::milliSeconds(10));
	    // Expected
	}
	catch(const LockedException&)
	{
	    test(false);
	}
    }

    // TEST: read lock
    {
	RWRecMutex::RLock rlock(mutex);

	// TEST: Start thread, try to acquire the mutex.
	t = new RWRecMutexReadTestThread(mutex);
	control = t->start();
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
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
	
	// TEST: Wait until the trylock has been tested.
	t->waitTrylock();
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

	// TEST: Wait until the trylock has been tested. The thread is
	// now waiting on a write lock.
	t->waitTrylock();

	// It's necessary for a small sleep here to ensure that the
	// thread is actually waiting on a write lock.
	ThreadControl::sleep(Time::seconds(1));

	try
	{
	    RWRecMutex::TryRLock rlock2(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}
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

	// TEST: Wait until the trylock has been tested. The thread is
	// now waiting on a read lock.
	t->waitTrylock();

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
	RWRecMutexUpgradeReadThreadPtr t = new RWRecMutexUpgradeReadThread(mutex);
	control = t->start();

	// Wait for the thread to acquire the read lock.
	t->waitLock();

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
	try
	{
	    RWRecMutex::TryRLock rlock2(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	//
	// As should a write lock.
	//
	try
	{
	    RWRecMutex::TryWLock rlock2(mutex);
	    test(false);
	}
	catch(const LockedException&)
	{
	    // Expected
	}

	//
	// Once the read lock is released then the upgrade should
	// succeed & the thread should terminate.
	//
	t->signalUnlock();

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
