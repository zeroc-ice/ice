// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <IceUtil/RWRecMutex.h>
#include <IceUtil/Exception.h>

#include <assert.h>

IceUtil::RWRecMutex::RWRecMutex() :
    _count(0),
    _waitingWriters(0)
{
}

IceUtil::RWRecMutex::~RWRecMutex()
{
}

void
IceUtil::RWRecMutex::readlock() const
{
    Mutex::Lock lock(_mutex);

    //
    // Wait while a writer holds the lock or while writers are waiting
    // to get the lock.
    //
    while (_count < 0 || _waitingWriters != 0)
    {
	_readers.wait(lock);
    }
    _count++;
}

void
IceUtil::RWRecMutex::tryReadlock() const
{
    Mutex::Lock lock(_mutex);

    //
    // Would block if a writer holds the lock or if writers are
    // waiting to get the lock.
    //
    if (_count < 0 || _waitingWriters != 0)
    {
	throw LockedException(__FILE__, __LINE__);
    }
    _count++;
}

void
IceUtil::RWRecMutex::timedTryReadlock(int timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // Wait while a writer holds the lock or while writers are waiting
    // to get the lock.
    //
    // TODO: This needs to check the time after each notify...
    //
    while (_count < 0 || _waitingWriters != 0)
    {
	if (!_readers.timedWait(lock, timeout))
	{
	    throw LockedException(__FILE__, __LINE__);
	}
    }

    _count++;
}

void
IceUtil::RWRecMutex::writelock() const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if (_count < 0 && _writerControl == ThreadControl())
    {
	--_count;
	return;
    }

    //
    // Wait for the lock to become available and increment the number
    // of waiting writers.
    //
    while (_count != 0)
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
}

void
IceUtil::RWRecMutex::tryWritelock() const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if (_count < 0 && _writerControl == ThreadControl())
    {
	--_count;
	return;
    }

    //
    // If there are readers or other writers then the call would block.
    //
    if (_count != 0)
    {
	throw LockedException(__FILE__, __LINE__);
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
}

void
IceUtil::RWRecMutex::timedTryWritelock(int timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if (_count < 0 && _writerControl == ThreadControl())
    {
	--_count;
	return;
    }

    //
    // Wait for the lock to become available and increment the number
    // of waiting writers.
    //
    // TODO: This needs to check the time after each notify...
    //
    if (_count != 0)
    {
	_waitingWriters++;
	bool timedOut;
	try
	{
	    timedOut = !_writers.timedWait(lock, timeout);
	}
	catch(...)
	{
	    --_waitingWriters;
	    throw;
	}
	_waitingWriters--;

	//
	// If a timeout occurred then the lock wasn't acquired.
	//
	if (timedOut)
	{
	    throw LockedException(__FILE__, __LINE__);
	}
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
}

void
IceUtil::RWRecMutex::unlock() const
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
	if (_count < 0)
	{
	    //
	    // Writer called unlock
	    //
	    ++_count;

	    //
	    // If the write lock wasn't totally released we're done.
	    //
	    if (_count != 0)
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
    if (ww)
    {
	//
	// Wake writer
	//
	_writers.signal();
    }
    else if (wr)
    {
	//
	// Wake readers
	//
	_readers.broadcast();
    }
}

void
IceUtil::RWRecMutex::upgrade() const
{
    Mutex::Lock lock(_mutex);

    //
    // Reader called unlock
    //
    assert(_count > 0);
    --_count;

    //
    // Wait to acquire the write lock.
    //
    while (_count != 0)
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
}

void
IceUtil::RWRecMutex::timedUpgrade(int timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // Reader called unlock
    //
    assert(_count > 0);
    --_count;

    //
    // Wait to acquire the write lock.
    //
    // TODO: This needs to check the time after each notify...
    //
    while (_count != 0)
    {
	_waitingWriters++;
	bool timedOut;
	try
	{
	    timedOut = !_writers.timedWait(lock, timeout);
	}
	catch(...)
	{
	    --_waitingWriters;
	    throw;
	}
	_waitingWriters--;

	//
	// If a timeout occurred then the lock wasn't acquired. Ensure
	// that the _count is increased again before returning.
	//
	if (timedOut)
	{
	    ++_count;
	    throw LockedException(__FILE__, __LINE__);
	}
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
}
