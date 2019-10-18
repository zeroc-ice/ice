//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_UTIL_COND_H
#define ICE_UTIL_COND_H

#include <IceUtil/Config.h>
#include <IceUtil/Time.h>
#include <IceUtil/ThreadException.h>

#if defined(_WIN32) && !defined(ICE_HAS_WIN32_CONDVAR)
#   include <IceUtil/Mutex.h>

namespace IceUtilInternal
{
//
// Needed for implementation.
//
class Semaphore
{
public:

    Semaphore(long = 0);
    ICE_API ~Semaphore();

    void wait() const;
    bool timedWait(const IceUtil::Time&) const;
    void post(int = 1) const;

private:

    mutable HANDLE _sem;
};
}
#endif

namespace IceUtil
{

//
// Forward declaration (for friend declarations).
//
template <class T> class Monitor;
class RecMutex;
class Mutex;

//
// Condition variable implementation. Conforms to the same semantics
// as a POSIX threads condition variable.
//
class ICE_API Cond : private noncopyable
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
        if(!lock.acquired())
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        waitImpl(lock._mutex);
    }

    //
    // wait atomically unlocks the mutex and waits for the condition
    // variable to be signaled for up to the given timeout. Before
    // returning to the calling thread the mutex is reaquired. Returns
    // true if the condition variable was signaled, false on a
    // timeout.
    //
    template <typename Lock> inline bool
    timedWait(const Lock& lock, const Time& timeout) const
    {
        if(!lock.acquired())
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        return timedWaitImpl(lock._mutex, timeout);
    }

private:

    friend class Monitor<Mutex>;
    friend class Monitor<RecMutex>;

    //
    // The Monitor implementation uses waitImpl & timedWaitImpl.
    //
#if defined(_WIN32) && !defined(ICE_HAS_WIN32_CONDVAR)

    template <typename M> void
    waitImpl(const M& mutex) const
    {
        preWait();

        typedef typename M::LockState LockState;

        LockState state;
        mutex.unlock(state);

        try
        {
            dowait();
            mutex.lock(state);
        }
        catch(...)
        {
            mutex.lock(state);
            throw;
        }
    }
    template <typename M> bool
    timedWaitImpl(const M& mutex, const Time& timeout) const
    {
        preWait();

        typedef typename M::LockState LockState;

        LockState state;
        mutex.unlock(state);

        try
        {
            bool rc = timedDowait(timeout);
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

    template <typename M> void waitImpl(const M&) const;
    template <typename M> bool timedWaitImpl(const M&, const Time&) const;

#endif

#ifdef _WIN32
#  ifdef ICE_HAS_WIN32_CONDVAR
    mutable CONDITION_VARIABLE _cond;
#  else
    void wake(bool);
    void preWait() const;
    void postWait(bool) const;
    bool timedDowait(const Time&) const;
    void dowait() const;

    Mutex _internal;
    IceUtilInternal::Semaphore _gate;
    IceUtilInternal::Semaphore _queue;
    mutable long _blocked;
    mutable long _unblocked;
    enum State
    {
        StateIdle,
        StateSignal,
        StateBroadcast
    };
    mutable State _state;
#  endif
#else
    mutable pthread_cond_t _cond;
#endif

};

#ifdef _WIN32

#   ifdef ICE_HAS_WIN32_CONDVAR

template <typename M> inline void
Cond::waitImpl(const M& mutex) const
{
    typedef typename M::LockState LockState;

    LockState state;
    mutex.unlock(state);
    BOOL ok = SleepConditionVariableCS(&_cond, state.mutex, INFINITE);
    mutex.lock(state);

    if(!ok)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

template <typename M> inline bool
Cond::timedWaitImpl(const M& mutex, const Time& timeout) const
{
    IceUtil::Int64 msTimeout = timeout.toMilliSeconds();
    if(msTimeout < 0 || msTimeout > 0x7FFFFFFF)
    {
        throw IceUtil::InvalidTimeoutException(__FILE__, __LINE__, timeout);
    }

    typedef typename M::LockState LockState;

    LockState state;
    mutex.unlock(state);
    BOOL ok = SleepConditionVariableCS(&_cond, state.mutex, static_cast<DWORD>(msTimeout));
    mutex.lock(state);

    if(!ok)
    {
        DWORD err = GetLastError();

        if(err != ERROR_TIMEOUT)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, err);
        }
        return false;
    }
    return true;
}

#   endif

#else
template <typename M> inline void
Cond::waitImpl(const M& mutex) const
{
    typedef typename M::LockState LockState;

    LockState state;
    mutex.unlock(state);
    int rc = pthread_cond_wait(&_cond, state.mutex);
    mutex.lock(state);

    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

template <typename M> inline bool
Cond::timedWaitImpl(const M& mutex, const Time& timeout) const
{
    if(timeout < Time::microSeconds(0))
    {
        throw InvalidTimeoutException(__FILE__, __LINE__, timeout);
    }

    typedef typename M::LockState LockState;

    LockState state;
    mutex.unlock(state);

#   ifdef __APPLE__
    //
    // The monotonic time is based on mach_absolute_time and pthread
    // condition variables require time from gettimeofday  so we get
    // the realtime time.
    //
    timeval tv = Time::now(Time::Realtime) + timeout;
#   else
    timeval tv = Time::now(Time::Monotonic) + timeout;
#   endif
    timespec ts;
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000;
    int rc = pthread_cond_timedwait(&_cond, state.mutex, &ts);
    mutex.lock(state);

    if(rc != 0)
    {
        //
        // pthread_cond_timedwait returns ETIMEOUT in the event of a
        // timeout.
        //
        if(rc != ETIMEDOUT)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
        return false;
    }
    return true;
}

#endif

} // End namespace IceUtil

#endif
