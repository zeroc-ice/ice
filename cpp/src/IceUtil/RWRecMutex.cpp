// **********************************************************************
//
// Copyright (c) 2001
// IONA Technologies, Inc.
// Waltham, MA, USA
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
IceUtil::RWRecMutex::readLock()
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
IceUtil::RWRecMutex::tryReadLock()
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
IceUtil::RWRecMutex::writeLock()
{
    Mutex::Lock lock(_mutex);

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
IceUtil::RWRecMutex::tryWriteLock()
{
    Mutex::Lock lock(_mutex);

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
IceUtil::RWRecMutex::unlock()
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
	    _count = 0;
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
