// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/RWRecMutex.h>
#include <IceUtil/Exception.h>
#include <IceUtil/Time.h>

IceUtil::DeadlockException::DeadlockException(const char* file, int line) :
    Exception(file, line)
{
}

const char* IceUtil::DeadlockException::_name = "IceUtil::DeadlockException";

::std::string
IceUtil::DeadlockException::ice_name() const
{
    return _name;
}

IceUtil::Exception*
IceUtil::DeadlockException::ice_clone() const
{
    return new DeadlockException(*this);
}

void
IceUtil::DeadlockException::ice_throw() const
{
    throw *this;
}

IceUtil::RWRecMutex::RWRecMutex() :
    _count(0),
    _waitingWriters(0),
    _upgrading(false)
{
}

IceUtil::RWRecMutex::~RWRecMutex()
{
}

void
IceUtil::RWRecMutex::readLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // Wait while a writer holds the lock or while writers or an upgrader
    // are waiting to get the lock.
    //
    while(_count < 0 || _waitingWriters != 0)
    {
        _readers.wait(lock);
    }
    ++_count;
}

bool
IceUtil::RWRecMutex::tryReadLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // Would block if a writer holds the lock or if writers or an upgrader
    // are waiting to get the lock.
    //
    if(_count < 0 || _waitingWriters != 0)
    {
        return false;
    }
    ++_count;
    return true;
}

bool
IceUtil::RWRecMutex::timedReadLock(const Time& timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // Wait while a writer holds the lock or while writers or an upgrader
    // are waiting to get the lock.
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

    ++_count;
    return true;
}

void
IceUtil::RWRecMutex::writeLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if(_count < 0 && _writerId == ThreadControl())
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
        ++_waitingWriters;
        try
        {
            _writers.wait(lock);
        }
        catch(...)
        {
            --_waitingWriters;
            throw;
        }
        --_waitingWriters;
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl();
}

bool
IceUtil::RWRecMutex::tryWriteLock() const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    //
    if(_count < 0 && _writerId == ThreadControl())
    {
        --_count;
        return true;
    }

    //
    // If there are readers or another writer then the call would block.
    //
    if(_count != 0)
    {
        return false;
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl();
    return true;
}

bool
IceUtil::RWRecMutex::timedWriteLock(const Time& timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // If the mutex is already write locked by this writer then
    // decrement _count, and return.
    if(_count < 0 && _writerId == ThreadControl())
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
            ++_waitingWriters;
            try
            {
                bool result = _writers.timedWait(lock, remainder);
                --_waitingWriters;
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
    _writerId = ThreadControl();
    return true;
}

void
IceUtil::RWRecMutex::unlock() const
{
    bool ww = false;
    bool wr = false;
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
    } // Unlock mutex.

    //
    // Wake up a waiting writer if there is one. If not, wake up all
    // readers (just in case--there may be none).
    //
    if(ww)
    {
        if(_upgrading)
        {
            //
            // If there is an untimed upgrader, it runs.
            //
            _upgrader.signal();
        }
        else
        {
            //
            // Wake a normal writer.
            //
            _writers.signal();
        }
    }
    else if(wr)
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

    if(_upgrading)
    {
        throw DeadlockException(__FILE__, __LINE__);
    }

    //
    // Reader owns at least one count.
    //
    assert(_count > 0);
    --_count;
   
    //
    // Wait to acquire the write lock.
    //
    _upgrading = true;
    while(_count != 0)
    {
        ++_waitingWriters;
        try
        {
            _upgrader.wait(lock);
        }
        catch(...)
        {
            _upgrading = false;
            --_waitingWriters;
            ++_count;
            throw;
        }
        --_waitingWriters;      
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl();
    _upgrading = false;
}

bool
IceUtil::RWRecMutex::timedUpgrade(const Time& timeout) const
{
    Mutex::Lock lock(_mutex);

    //
    // If another reader is already waiting for an upgrade,
    // this upgrade cannot possibly succeed.
    //
    if(_upgrading)
    {
        return false;
    }

    // Reader owns at least one count
    //
    assert(_count > 0);
    --_count;

    //
    // Wait to acquire the write lock.
    //
    _upgrading = true;
    Time end = Time::now() + timeout;
    while(_count != 0)
    {
        Time remainder = end - Time::now();
        if(remainder > Time())
        {
            ++_waitingWriters;
            try
            {
                bool result = _upgrader.timedWait(lock, remainder);
                --_waitingWriters;
                if(!result)
                {
                    _upgrading = false;
                    ++_count;
                    return false;
                }
            }
            catch(...)
            {
                _upgrading = false;
                --_waitingWriters;
                ++_count;
                throw;
            }
        }
        else
        {
            //
            // The lock isn't acquired if a timeout occurred.
            //
            ++_count;
            _upgrading = false;
            return false;
        }
    }

    //
    // Got the lock, indicate it's held by a writer.
    //
    _count = -1;
    _writerId = ThreadControl();
    _upgrading = false;
    return true;
}

void
IceUtil::RWRecMutex::downgrade() const
{
    Mutex::Lock lock(_mutex);

    if(++_count == 0)
    {
        _count = 1;
    }
}
