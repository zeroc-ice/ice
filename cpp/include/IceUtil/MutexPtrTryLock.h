// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_UTIL_MUTEX_PTR_TRY_LOCK_H
#define ICE_UTIL_MUTEX_PTR_TRY_LOCK_H

#include <IceUtil/Config.h>
#include <IceUtil/Mutex.h>

namespace IceUtilInternal
{

template<class T>
class MutexPtrTryLock
{
public:
    
    MutexPtrTryLock<T>(const T* mutex) :
        _mutex(mutex),
        _acquired(false)
    {
        if(_mutex)
        {
            _acquired = _mutex->tryLock();
        }
    }

    ~MutexPtrTryLock<T>()
    {
        if(_mutex && _acquired)
        {
            _mutex->unlock();
        }
    }

    void acquire() const
    {
        if(_mutex)
        {
            _mutex->lock();
            _acquired = true;
        }
    }

    void release() const
    {
        if(_mutex)
        {
            if(!_acquired)
            {
                throw IceUtil::ThreadLockedException(__FILE__, __LINE__);
            }
            _mutex->unlock();
            _acquired = false;
        }
    }

    bool acquired() const
    {
        return _acquired;
    }
   
private:
    
    // Not implemented; prevents accidental use.
    //
    MutexPtrTryLock<T>(const MutexPtrTryLock&);
    MutexPtrTryLock<T>& operator=(const MutexPtrTryLock<T>&);

    const T* _mutex;
    mutable bool _acquired;
};

} // End namespace IceUtilInternal

#endif
