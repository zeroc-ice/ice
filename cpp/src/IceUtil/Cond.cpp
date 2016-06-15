// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Cond.h>

#ifndef _WIN32
#    include <sys/time.h>
#endif

#ifdef _WIN32

#   ifdef ICE_HAS_WIN32_CONDVAR

IceUtil::Cond::Cond()
{
    InitializeConditionVariable(&_cond);
}

IceUtil::Cond::~Cond()
{
}

void
IceUtil::Cond::signal()
{
    WakeConditionVariable(&_cond);
}

void
IceUtil::Cond::broadcast()
{
    WakeAllConditionVariable(&_cond);
}

#   else

IceUtilInternal::Semaphore::Semaphore(long initial)
{
    _sem = CreateSemaphore(0, initial, 0x7fffffff, 0);
    if(_sem == INVALID_HANDLE_VALUE)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

IceUtilInternal::Semaphore::~Semaphore()
{
    CloseHandle(_sem);
}

void
IceUtilInternal::Semaphore::wait() const
{
    DWORD rc = WaitForSingleObject(_sem, INFINITE);
    if(rc != WAIT_OBJECT_0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

bool
IceUtilInternal::Semaphore::timedWait(const IceUtil::Time& timeout) const
{
    IceUtil::Int64 msTimeout = timeout.toMilliSeconds();
    if(msTimeout < 0 || msTimeout > 0x7FFFFFFF)
    {
        throw IceUtil::InvalidTimeoutException(__FILE__, __LINE__, timeout);
    } 

    DWORD rc = WaitForSingleObject(_sem, static_cast<DWORD>(msTimeout));
    if(rc != WAIT_TIMEOUT && rc != WAIT_OBJECT_0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
    return rc != WAIT_TIMEOUT;
}

void
IceUtilInternal::Semaphore::post(int count) const
{
    int rc = ReleaseSemaphore(_sem, count, 0);
    if(rc == 0)
    {
        throw IceUtil::ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
}

//
// The _queue semaphore is used to wait for the condition variable to
// be signaled. When signal is called any further thread signaling or
// threads waiting to wait (in preWait) are blocked from proceeding
// using an addition _gate semaphore) until the correct number of
// threads waiting on the _queue semaphore drain through postWait
//
// As each thread drains through postWait if there are further threads
// to unblock (toUnblock > 0) the _queue is posted again to wake a
// further waiting thread, otherwise the _gate is posted which permits
// any signaling or preWait threads to continue. Therefore, the _gate
// semaphore is used protect further entry into signal or wait until
// all signaled threads have woken.
//
// _blocked is the number of waiting threads.  _unblocked is the
// number of threads which have unblocked. We use two variables
// because _blocked is protected by the _gate, whereas _unblocked is
// protected by the _internal mutex. There is an assumption here about
// memory visibility since postWait does not itself acquire the _gate
// semaphore (note that the _gate must be held if _state != StateIdle).
//
// Threads timing out present a particular issue because they may have
// woken without a corresponding notification and its easy to leave
// the _queue in a state where a spurious wakeup will occur --
// consider a notify and a timed wake occuring at the same time. In
// this case, if we are not careful the _queue will have been posted,
// but the waking thread may not consume the semaphore.
//
IceUtil::Cond::Cond() :
    _gate(1),
    _blocked(0),
    _unblocked(0),
    _state(IceUtil::Cond::StateIdle)
{
}

IceUtil::Cond::~Cond()
{
}

void
IceUtil::Cond::signal()
{
    wake(false);
}

void
IceUtil::Cond::broadcast()
{
    wake(true);
}

void
IceUtil::Cond::wake(bool broadcast)
{
    //
    // Lock gate. The gate will be locked if there are threads waiting
    // to drain from postWait.
    //
    _gate.wait();

    //
    // Lock the internal mutex.
    //
    IceUtil::Mutex::Lock sync(_internal);

    //
    // Adjust the count of the number of waiting/blocked threads.
    //
    if(_unblocked != 0)
    {
        _blocked -= _unblocked;
        _unblocked = 0;
    }

    //
    // If there are waiting threads then we enter a signal or
    // broadcast state.
    //
    if(_blocked > 0)
    {
        //
        // Unblock some number of waiters. We use -1 for the signal
        // case.
        //
        assert(_state == StateIdle);
        _state = (broadcast) ? StateBroadcast : StateSignal;
        //
        // Posting the queue wakes a single waiting thread. After this
        // occurs the waiting thread will wake and then either post on
        // the _queue to wake the next waiting thread, or post on the
        // gate to permit more signaling to proceed.
        //
        // Release before posting to avoid potential immediate
        // context switch due to the mutex being locked.
        //
        sync.release();
        _queue.post();
    }
    else
    {
        //
        // Otherwise no blocked waiters, release the gate.
        //
        // Release before posting to avoid potential immediate
        // context switch due to the mutex being locked.
        //
        sync.release();
        _gate.post();
    }
}

void
IceUtil::Cond::preWait() const
{
    //
    // _gate is used to protect _blocked. Furthermore, this prevents
    // further threads from entering the wait state while a
    // signal/broadcast is being processed.
    //
    _gate.wait();
    _blocked++;
    _gate.post();
}

void
IceUtil::Cond::postWait(bool timedOutOrFailed) const
{
    IceUtil::Mutex::Lock sync(_internal);

    //
    // One more thread has unblocked.
    //
    _unblocked++;

    //
    // If _state is StateIdle then this must be a timeout, otherwise its a
    // spurious wakeup which is incorrect.
    //
    if(_state == StateIdle)
    {
        assert(timedOutOrFailed);
        return;
    }

    if(timedOutOrFailed)
    {
        //
        // If the thread was the last blocked thread and there's a
        // pending signal/broadcast, reset the signal/broadcast to
        // prevent spurious wakeup.
        //
        if(_blocked == _unblocked)
        {
            _state = StateIdle;
            //
            // Consume the queue post to prevent spurious wakeup. Note
            // that although the internal mutex could be released
            // prior to this wait() call, doing so gains nothing since
            // this wait() MUST return immediately (if it does not
            // there is a major bug and the entire application will
            // deadlock).
            //
            _queue.wait();
            //
            // Release before posting to avoid potential immediate
            // context switch due to the mutex being locked.
            //
            sync.release();
            _gate.post();
        }
    }
    else
    {
        //
        // At this point, the thread must have been woken up because
        // of a signal/broadcast.
        //
        if(_state == StateSignal || _blocked == _unblocked) // Signal or no more blocked threads
        {
            _state = StateIdle;
            // Release before posting to avoid potential immediate
            // context switch due to the mutex being locked.
            sync.release();
            _gate.post();
        }
        else // Broadcast and more blocked threads to wake up.
        {
            // Release before posting to avoid potential immediate
            // context switch due to the mutex being locked.
            sync.release();
            _queue.post();
        }
    }
}

void
IceUtil::Cond::dowait() const
{
    try
    {
        _queue.wait();
        postWait(false);
    }
    catch(...)
    {
        postWait(true);
        throw;
    }
}

bool
IceUtil::Cond::timedDowait(const Time& timeout) const
{
    try
    {
        bool rc = _queue.timedWait(timeout);
        postWait(!rc);
        return rc;
    }
    catch(...)
    {
        postWait(true);
        throw;
    }
}

#    endif // ICE_HAS_WIN32_CONDVAR

#else

IceUtil::Cond::Cond()
{
    pthread_condattr_t attr;

    int rc = pthread_condattr_init(&attr);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

#if !defined(__hppa) && !defined(__APPLE__) && !defined(__FreeBSD__)
    rc = pthread_condattr_setclock(&attr, CLOCK_MONOTONIC); 
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
#endif

    rc = pthread_cond_init(&_cond, &attr);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    rc = pthread_condattr_destroy(&attr);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

IceUtil::Cond::~Cond()
{
#ifndef NDEBUG
    int rc = pthread_cond_destroy(&_cond);
    assert(rc == 0);
#else
    pthread_cond_destroy(&_cond);
#endif
}

void
IceUtil::Cond::signal()
{
    int rc = pthread_cond_signal(&_cond);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
IceUtil::Cond::broadcast()
{
    int rc = pthread_cond_broadcast(&_cond);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

#endif
