// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_UTIL_COND_H
#define ICE_UTIL_COND_H

#include <IceUtil/Config.h>
#include <IceUtil/Exception.h>

#ifdef WIN32
//
// Needed for implementation under WIN32.
//
#    include <IceUtil/Mutex.h>
//
// See member-template note for waitImpl & timedwaitImpl.
//
#    include <IceUtil/RecMutex.h>
#endif

namespace IceUtil
{

//
// Forward declaration (for friend declarations).
//
template <class T> class Monitor;
class RecMutex;
class Mutex;

#ifdef WIN32
//
// Needed for implementation.
//
class Semaphore
{
public:

    Semaphore(long = 0);
    ~Semaphore();

    bool wait(long = -1);
    void post(int = 1);

private:

    HANDLE _sem;
};
#else

struct timespec msecToTimespec(long timeout);

#endif

//
// Condition variable implementation. Conforms to the same semantics
// as a POSIX threads condition variable.
//
class ICE_UTIL_API Cond : public noncopyable
{
public:

    Cond();
    ~Cond();

    //
    // signal restarts one of the threads that are waiting on the
    // condition variable cond.  If no threads are waiting on cond,
    // nothing happens. If several threads are waiting on cond,
    // exactly one is restarted, but it is not specified which.
    //
    void signal();

    //
    // pthread_cond_broadcast restarts all the threads that are
    // waiting on the condition variable cond. Nothing happens if no
    // threads are waiting on cond.
    //
    void broadcast();

    //
    // MSVC doesn't support out-of-class definitions of member
    // templates. See KB Article Q241949 for details.
    //

    //
    // wait atomically unlocks the mutex and waits for the condition
    // variable to be signaled. Before returning to the calling thread
    // the mutex is reaquired.
    //
    template <typename Lock> inline void
    wait(Lock& lock)
    {
	waitImpl(lock._mutex);
    }

    //
    // wait atomically unlocks the mutex and waits for the condition
    // variable to be signaled for up to msec milliseconds. Before
    // returning to the calling thread the mutex is reaquired. Returns
    // true if the condition variable was signaled, false on a
    // timeout.
    //
    template <typename Lock> inline bool
    timedwait(Lock& lock, long msec)
    {
	timedwaitImpl(lock._mutex, msec);
    }

private:

    friend class Monitor<Mutex>;
    friend class Monitor<RecMutex>;

    //
    // The Monitor implementation uses waitImpl & timedwaitImpl.
    //
#ifdef WIN32

    //
    // For some reason under WIN32 with VC6 using a member-template
    // for waitImpl & timedwaitImpl results in a link error for
    // RecMutex.
    //
/*
    template <typename M> void
    waitImpl(M& mutex)
    {
        preWait();

        typedef typename M::LockState LockState;

        LockState state;
        mutex.unlock(state);

        try
        {
            dowait(-1);
            mutex.lock(state);
        }
        catch(...)
        {
            mutex.lock(state);
            throw;
        }
    }
    template <typename M> bool
    timedwaitImpl(M& mutex, long msec)
    {
        preWait();

        typedef typename M::LockState LockState;

        LockState state;
        mutex.unlock(state);

        try
        {
            bool rc = dowait(msec);
            mutex.lock(state);
            return rc;
        }
        catch(...)
        {
            mutex.lock(state);
            throw;
        }
    }
 */

    void
    waitImpl(RecMutex& mutex)
    {
	preWait();

	RecMutex::LockState state;
	mutex.unlock(state);

	try
	{
	    dowait(-1);
	    mutex.lock(state);
	}
	catch(...)
	{
	    mutex.lock(state);
	    throw;
	}
    }

    void
    waitImpl(Mutex& mutex)
    {
	preWait();

	Mutex::LockState state;
	mutex.unlock(state);

	try
	{
	    dowait(-1);
	    mutex.lock(state);
	}
	catch(...)
	{
	    mutex.lock(state);
	    throw;
	}
    }
    
    bool
    timedwaitImpl(RecMutex& mutex, long msec)
    {
	preWait();

	RecMutex::LockState state;
	mutex.unlock(state);

	try
	{
	    bool rc = dowait(msec);
	    mutex.lock(state);
	    return rc;
	}
	catch(...)
	{
	    mutex.lock(state);
	    throw;
	}
    }

    bool
    timedwaitImpl(Mutex& mutex, long msec)
    {
	preWait();

	Mutex::LockState state;
	mutex.unlock(state);

	try
	{
	    bool rc = dowait(msec);
	    mutex.lock(state);
	    return rc;
	}
	catch(...)
	{
	    mutex.lock(state);
	    throw;
	}
    }

#else

    template <typename M> void waitImpl(M&);
    template <typename M> bool timedwaitImpl(M&, long);

#endif

#ifdef WIN32
    void wake(bool);
    void preWait();
    void postWait(bool);
    bool dowait(long);

    Mutex _internal;
    Semaphore _gate;
    Semaphore _queue;
    long _blocked;
    long _unblocked;
    long _toUnblock;
#else
    pthread_cond_t _cond;
#endif

};

#ifndef WIN32
template <typename M> inline void
Cond::waitImpl(M& mutex)
{
    typedef typename M::LockState LockState;
    
    LockState state;
    mutex.unlock(state);
    int rc = pthread_cond_wait(&_cond, state.mutex);
    mutex.lock(state);
    
    if (rc != 0)
    {
	throw SyscallException(strerror(rc), __FILE__, __LINE__);
    }
}

template <typename M> inline bool
Cond::timedwaitImpl(M& mutex, long msec)
{
    typedef typename M::LockState LockState;
    
    LockState state;
    mutex.unlock(state);
    
    struct timespec ts = msecToTimespec(msec);
    int rc = pthread_cond_timedwait(&_cond, state.mutex, &ts);
    mutex.lock(state);
    
    if (rc != 0)
    {
	//
	// pthread_cond_timedwait returns ETIMEOUT in the event of a
	// timeout.
	//
	if (rc != ETIMEDOUT)
	{
	    throw SyscallException(strerror(rc), __FILE__, __LINE__);
	}
	return false;
    }
    return true;
}
#endif

} // End namespace IceUtil

#endif
