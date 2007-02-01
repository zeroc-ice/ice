// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/CountDownLatch.h>
#include <IceUtil/ThreadException.h>

IceUtil::CountDownLatch::CountDownLatch(int count) :
    _count(count)
{
    if(count < 0)
    {
        throw Exception(__FILE__, __LINE__);
    }

#ifdef _WIN32
    _event = CreateEvent(0, TRUE, FALSE, 0);
    if(_event == 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
    }
#else
    int rc = pthread_mutex_init(&_mutex, 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    
    rc = pthread_cond_init(&_cond, 0);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }   
#endif
}

IceUtil::CountDownLatch::~CountDownLatch()
{
#ifdef _WIN32
    CloseHandle(_event);
#else
    int rc = 0;
    rc = pthread_mutex_destroy(&_mutex);
    assert(rc == 0);
    rc = pthread_cond_destroy(&_cond);
    assert(rc == 0);
#endif
}

void 
IceUtil::CountDownLatch::await() const
{
#ifdef _WIN32
    while(InterlockedExchangeAdd(&_count, 0) > 0)
    {
        DWORD rc = WaitForSingleObject(_event, INFINITE);
        assert(rc == WAIT_OBJECT_0 || rc == WAIT_FAILED);
        
        if(rc == WAIT_FAILED)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
    }
#else
    lock();
    while(_count > 0)
    {
        int rc = pthread_cond_wait(&_cond, &_mutex);
        if(rc != 0)
        {
            pthread_mutex_unlock(&_mutex);
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    unlock();
    
#endif
}

void 
IceUtil::CountDownLatch::countDown()
{
#ifdef _WIN32
    if(InterlockedDecrement(&_count) == 0)
    {
        if(SetEvent(_event) == 0)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
        }
    }
#else
    bool broadcast = false;

    lock();
    if(_count > 0 && --_count == 0)
    {
        broadcast = true;
    }
#if defined(__APPLE__)
    //
    // On MacOS X we do the broadcast with the mutex held. This seems to be necessary to prevent the 
    // broadcast call to hang (spinning in an infinite loop).
    //
    if(broadcast)
    {
        int rc = pthread_cond_broadcast(&_cond);
        if(rc != 0)
        {
            pthread_mutex_unlock(&_mutex);
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
    unlock();
    
#else
    unlock();
    
    if(broadcast)
    {
        int rc = pthread_cond_broadcast(&_cond);
        if(rc != 0)
        {
            throw ThreadSyscallException(__FILE__, __LINE__, rc);
        }
    }
#endif

#endif
}

int 
IceUtil::CountDownLatch::getCount() const
{
#ifdef _WIN32
    int count = InterlockedExchangeAdd(&_count, 0);
    return count > 0 ? count : 0;
#else
    lock();
    int result = _count;
    unlock();
    return result;
#endif
}

#ifndef _WIN32
void
IceUtil::CountDownLatch::lock() const
{
    int rc = pthread_mutex_lock(&_mutex);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

void
IceUtil::CountDownLatch::unlock() const
{
    int rc = pthread_mutex_unlock(&_mutex);
    if(rc != 0)
    {
        throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
}

#endif
