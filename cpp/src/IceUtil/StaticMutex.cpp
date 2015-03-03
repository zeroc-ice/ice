// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// We disable deprecation warning here, to allow clean compilation of
// of deprecated methods.
//
#ifdef _MSC_VER
#   pragma warning( disable : 4996 )
#endif

#include <IceUtil/StaticMutex.h>
#include <IceUtil/ThreadException.h>

#ifdef _WIN32
#   include <list>
#   include <algorithm>

using namespace std;

static CRITICAL_SECTION _criticalSection;

//
// Although apparently not documented by Microsoft, static objects are
// initialized before DllMain/DLL_PROCESS_ATTACH and finalized after
// DllMain/DLL_PROCESS_DETACH ... However, note that after the DLL is
// detached the allocated StaticMutexes may still be accessed. See
// http://blogs.msdn.com/larryosterman/archive/2004/06/10/152794.aspx
// for some details. This means that there is no convenient place to
// cleanup the globally allocated static mutexes.
//

namespace IceUtil
{

class Init
{
public:

    Init();
};

static Init _init;

Init::Init()
{
    InitializeCriticalSection(&_criticalSection);
}

}

void IceUtil::StaticMutex::initialize() const
{
    //
    // Yes, a double-check locking. It should be safe since we use memory barriers
    // (through InterlockedCompareExchangePointer) in both reader and writer threads
    //
    EnterCriticalSection(&_criticalSection);

    //
    // The second check
    //
    if(_mutex == 0)
    {
        CRITICAL_SECTION* newMutex = new CRITICAL_SECTION;
        InitializeCriticalSection(newMutex);

        //
        // _mutex is written after the new initialized CRITICAL_SECTION/Mutex
        //
        void* oldVal = InterlockedCompareExchangePointer(reinterpret_cast<void**>(&_mutex), newMutex, 0);
        assert(oldVal == 0);

    }
    LeaveCriticalSection(&_criticalSection);
}

bool 
IceUtil::StaticMutex::initialized() const
{
    //
    // Read mutex and then inserts a memory barrier to ensure we can't 
    // see tmp != 0 before we see the initialized object
    //
    void* tmp = _mutex;
    return InterlockedCompareExchangePointer(reinterpret_cast<void**>(&tmp), 0, 0) != 0;
}

void
IceUtil::StaticMutex::lock() const
{
    if(!initialized())
    {
        initialize();
    }
    EnterCriticalSection(_mutex);
    assert(_mutex->RecursionCount == 1);
}

bool
IceUtil::StaticMutex::tryLock() const
{
    if(!initialized())
    {
        initialize();
    }
    if(!TryEnterCriticalSection(_mutex))
    {
        return false;
    }
    if(_mutex->RecursionCount > 1)
    {
        LeaveCriticalSection(_mutex);
        throw ThreadLockedException(__FILE__, __LINE__);
    }
    return true;
}

void
IceUtil::StaticMutex::unlock() const
{
    assert(_mutex != 0);
    assert(_mutex->RecursionCount == 1);
    LeaveCriticalSection(_mutex);
}

void
IceUtil::StaticMutex::unlock(LockState&) const
{
    assert(_mutex != 0);
    assert(_mutex->RecursionCount == 1);
    LeaveCriticalSection(_mutex);
}

void
IceUtil::StaticMutex::lock(LockState&) const
{
    if(!initialized())
    {
        initialize();
    }
    EnterCriticalSection(_mutex);
}

#else

void
IceUtil::StaticMutex::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
        if(rc == EDEADLK)
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        else
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
}

bool
IceUtil::StaticMutex::tryLock() const
{
    int rc = pthread_mutex_trylock(&_mutex);
    if(rc != 0 && rc != EBUSY)
    {
        if(rc == EDEADLK)
        {
            throw ThreadLockedException(__FILE__, __LINE__);
        }
        else
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    return (rc == 0);
}

void
IceUtil::StaticMutex::unlock() const
{
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
IceUtil::StaticMutex::unlock(LockState& state) const
{
    state.mutex = &_mutex;
}

void
IceUtil::StaticMutex::lock(LockState&) const
{
}

#endif
