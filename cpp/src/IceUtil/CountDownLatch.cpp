// **********************************************************************
//
// Copyright (c) 2004
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
    _event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if(_event == NULL)
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
    unlock();
    
    if(broadcast)
    {
	int rc = pthread_cond_broadcast(&_cond);
	if(rc != 0)
	{
	    pthread_mutex_unlock(&_mutex);
	    throw ThreadSyscallException(__FILE__, __LINE__, rc);
	}
    }
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
