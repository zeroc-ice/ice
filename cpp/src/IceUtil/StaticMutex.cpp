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

#include <IceUtil/StaticMutex.h>
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

static MutexList* _mutexList;

// Although apparently not documented by Microsoft, static objects are
// initialized before DllMain/DLL_PROCESS_ATTACH and finalized after
// DllMain/DLL_PROCESS_DETACH ... that's why we use a static object.

namespace IceUtil
{

class Init
{
public:

    Init();
    ~Init();
};

static Init _init;


Init::Init()
{
    InitializeCriticalSection(&_criticalSection);

    _mutexList = new MutexList;
}

Init::~Init()
{
#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
    for(MutexList::iterator p = _mutexList->begin(); 
	p != _mutexList->end(); ++p)
    {
	DeleteCriticalSection(*p);
	delete *p;
    }
#   else
    for_each(_mutexList->begin(), _mutexList->end(), 
	     CloseHandle);
#   endif
    delete _mutexList;
    DeleteCriticalSection(&_criticalSection);
}
}

// For full thread-safety, we assume that _mutexInitialized cannot be seen as true
// before CRITICAL_SECTION has been updated. This is true on x86. Does IA64 
// provide the same memory ordering guarantees?
//
void IceUtil::StaticMutex::initialize() const
{
    EnterCriticalSection(&_criticalSection);
    if(!_mutexInitialized)
    {
        _mutex = new CRITICAL_SECTION;
	InitializeCriticalSection(_mutex);

#   if defined(_WIN32_WINNT) && _WIN32_WINNT >= 0x0400
	_mutex = new CRITICAL_SECTION;
	InitializeCriticalSection(_mutex);
	_mutexList->push_back(_mutex);
#   else
	_recursionCount = 0;
	_mutex = CreateMutex(0, false, 0);
	if(_mutex == 0)
	{
	    throw ThreadSyscallException(__FILE__, __LINE__, GetLastError());
	}
	_mutexList->push_back(_mutex);
#   endif
	_mutexInitialized = true;
    }
    LeaveCriticalSection(&_criticalSection);
}

#endif

IceUtil::StaticMutex IceUtil::globalMutex = ICE_STATIC_MUTEX_INITIALIZER;

