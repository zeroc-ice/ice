// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/RWRecMutex.h>
#include <IceE/Exception.h>
#include <IceE/Time.h>


IceE::RWRecMutex::RWRecMutex() :
    _count(0),
    _writerId(0),
    _waitingWriters(0)
{
}

IceE::RWRecMutex::~RWRecMutex()
{
}

void
IceE::RWRecMutex::readLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // Wait while a writer holds the lock or while writers are waiting
    // to get the lock.
    //
    while(_count < 0 || _waitingWriters != 0)
    {
	_readers.wait(lock);
    }
    _count++;
}

bool
IceE::RWRecMutex::tryReadLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // Would block if a writer holds the lock or if writers are
    // waiting to get the lock.
    //
    if(_count < 0 || _waitingWriters != 0)
    {
	return false;
    }
    _count++;
    return true;
}

bool
IceE::RWRecMutex::timedReadLock(const Time& timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // Wait while a writer holds the lock or while writers are waiting
    // to get the lock.
    //
    Time end = Time::now() + timeout;
    while(_count < 0 || _waitingWriters != 0)
    {
	Time remainder = end - Time::now();
	if(remainder > Time())
	{
	    if(_readers.timedWait(lock, remainder) == false)
	    {
		return false;
	    }
	}
	else
	{
	    return false;
	}
    }

    _count++;
    return true;
}

void
IceE::RWRecMutex::writeLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if(_count < 0 && _writerId == ThreadControl().id())
    {
	--_count;
	return;
    }

    //
    // Wait for the lock to become available and increment the number
    // of waiting writers.
    //
    while(_count != 0)
    {
	_waitingWriters++;
	try
	{
	    _writers.wait(lock);
	}
	catch(...)
	{
	    --_waitingWriters;
	    throw;
	}
	_waitingWriters--;
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl().id();
}

bool
IceE::RWRecMutex::tryWriteLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if(_count < 0 && _writerId == ThreadControl().id())
    {
	--_count;
	return true;
    }

    //
    // If there are readers or other writers then the call would block.
    //
    if(_count != 0)
    {
	return false;
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl().id();
    return true;
}

bool
IceE::RWRecMutex::timedWriteLock(const Time& timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    if(_count < 0 && _writerId == ThreadControl().id())
    {
	--_count;
	return true;
    }

    //
    // Wait for the lock to become available and increment the number
    // of waiting writers.
    //
    Time end = Time::now() + timeout;
    while(_count != 0)
    {
	Time remainder = end - Time::now();
	if(remainder > Time())
	{
	    _waitingWriters++;
	    try
	    {
		bool result = _writers.timedWait(lock, remainder);
		_waitingWriters--;
		if(result == false)
		{
		    return false;
		}
	    }
	    catch(...)
	    {
		--_waitingWriters;
		throw;
	    }
	}
	else
	{
	    return false;
	}
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl().id();
    return true;
}

void
IceE::RWRecMutex::unlock() const
{
    bool ww;
    bool wr;
    {
	Mutex::Lock lock(_mutex);

	assert(_count != 0);

	//
	// If _count < 0, the calling thread is a writer that holds the
	// lock, so release the lock.  Otherwise, _count is guaranteed to
	// be > 0, so the calling thread is a reader releasing the lock.
	//
	if(_count < 0)
	{
	    //
	    // Writer called unlock
	    //
	    ++_count;

	    //
	    // If the write lock wasn't totally released we're done.
	    //
	    if(_count != 0)
	    {
		return;
	    }
	}
	else
	{
	    //
	    // Reader called unlock
	    //
	    --_count;
	}

	//
	// Writers are waiting (ww) if _waitingWriters > 0.  In that
	// case, it's OK to let another writer into the region once there
	// are no more readers (_count == 0).  Otherwise, no writers are
	// waiting but readers may be waiting (wr).
	//
	ww = (_waitingWriters != 0 && _count == 0);
	wr = (_waitingWriters == 0);
    }

    //
    // Wake up a waiting writer if there is one. If not, wake up all
    // readers (just in case -- there may be none).
    //
    if(ww)
    {
	//
	// Wake writer
	//
	_writers.signal();
    }
    else if(wr)
    {
	_writerId = 0;
	//
	// Wake readers
	//
	_readers.broadcast();
    }
}

void
IceE::RWRecMutex::upgrade() const
{
    Mutex::Lock lock(_mutex);

    // Reader owns at least one count
    //
    assert(_count > 0);
    --_count;
   
    //
    // Wait to acquire the write lock.
    //
    while(_count != 0)
    {
	_waitingWriters++;
	try
	{
	    _writers.wait(lock);
	}
	catch(...)
	{
	    --_waitingWriters;
	    _count++;
	    throw;
	}
	_waitingWriters--;	
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl().id();
}

bool
IceE::RWRecMutex::timedUpgrade(const Time& timeout) const
{
    Mutex::Lock lock(_mutex);

    // Reader owns at least one count
    //
    assert(_count > 0);
    --_count;

    //
    // Wait to acquire the write lock.
    //
    Time end = Time::now() + timeout;
    while(_count != 0)
    {
	Time remainder = end - Time::now();
	if(remainder > Time())
	{
	    _waitingWriters++;
	    try
	    {
		bool result = _writers.timedWait(lock, remainder);
		_waitingWriters--;
		if(result == false)
		{
		    _count++;
		    return false;
		}
	    }
	    catch(...)
	    {
		--_waitingWriters;
		_count++;
		throw;
	    }
	}
	else
	{
	    //
	    // If a timeout occurred then the lock wasn't acquired
	    _count++;
	    return false;
	}
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl().id();
    return true;
}
