// **********************************************************************
//
// Copyright (c) 2003
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

#include <IceUtil/StaticRecMutex.h>
#include <IceUtil/ThreadException.h>

#ifdef _WIN32
#   include <list>
#   include <algorithm>

using namespace std;

static CRITICAL_SECTION _criticalSection;

#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
typedef list<CRITICAL_SECTION*> MutexList;
#   else
typedef list<HANDLE> MutexList;
#   endif

static MutexList* _recMutexList;

//
// Although apparently not documented by Microsoft, static objects are
// initialized before DllMain/DLL_PROCESS_ATTACH and finalized after
// DllMain/DLL_PROCESS_DETACH ... that's why we use a static object.
//

namespace IceUtil
{

class RecInit
{
public:

    RecInit();
    ~RecInit();
};

static RecInit _recInit;

RecInit::RecInit()
{
    InitializeCriticalSection(&_criticalSection);

    _recMutexList = new MutexList;
}

RecInit::~RecInit()
{
#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
    for(MutexList::iterator p = _recMutexList->begin(); 
	p != _recMutexList->end(); ++p)
    {
	DeleteCriticalSection(*p);
	delete *p;
    }
#   else
    for_each(_recMutexList->begin(), _recMutexList->end(), 
	     CloseHandle);
#   endif
    delete _recMutexList;
    DeleteCriticalSection(&_criticalSection);
}
}

//
// For full thread-safety, we assume that _mutexInitialized cannot be seen as true
// before CRITICAL_SECTION has been updated. This is true on x86. Does IA64 
// provide the same memory ordering guarantees?
//

void IceUtil::StaticRecMutex::initialize() const
{
    EnterCriticalSection(&_criticalSection);
    if(!_mutexInitialized)
    {
#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
	_mutex = new CRITICAL_SECTION;
	InitializeCriticalSection(_mutex);
	_recMutexList->push_back(_mutex);
#   else
	_mutex = CreateMutex(0, false, 0);
	if(_mutex == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
	_recMutexList->push_back(_mutex);
#   endif
	_mutexInitialized = true;
    }
    LeaveCriticalSection(&_criticalSection);
}

#elif defined(__sun)

void IceUtil::StaticRecMutex::initialize() const
{
    int rc;
    pthread_mutexattr_t attr;

    if((rc = pthread_mutexattr_init(&attr)) != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    if((rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE)) != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    if((rc = pthread_mutex_init(&_mutex, &attr)) != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }
    if((rc = pthread_mutexattr_destroy(&attr)) != 0)
    {
	throw ThreadSyscallException(__FILE__, __LINE__, rc);
    }

    _mutexInitialized = true;
}

#endif
