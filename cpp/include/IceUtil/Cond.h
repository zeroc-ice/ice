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

    bool wait(long = -1) const;
    void post(int = 1) const;

private:

    mutable HANDLE _sem;
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
    // broadcast restarts all the threads that are waiting on the
    // condition variable cond. Nothing happens if no threads are
    // waiting on cond.
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
    wait(const Lock& lock) const
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
    timedwait(const Lock& lock, long msec) const
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
    waitImpl(const M& mutex) const
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
    timedwaitImpl(const M& mutex, long msec) const
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

    void waitImpl(const RecMutex&) const;
    void waitImpl(const Mutex&) const;
    bool timedwaitImpl(const RecMutex&, long) const;
    bool timedwaitImpl(const Mutex&, long) const;

#else

    template <typename M> void waitImpl(const M&) const;
    template <typename M> bool timedwaitImpl(const M&, long) const;


#endif

#ifdef WIN32
    void wake(bool);
    void preWait() const;
    void postWait(bool) const;
    bool dowait(long) const;

    Mutex _internal;
    Semaphore _gate;
    Semaphore _queue;
    mutable long _blocked;
    mutable long _unblocked;
    mutable long _toUnblock;
#else
    mutable pthread_cond_t _cond;
#endif

};

#ifndef WIN32
template <typename M> inline void
Cond::waitImpl(const M& mutex) const
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
Cond::timedwaitImpl(const M& mutex, long msec) const
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
