// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/StaticMutex.h>
#include <IceUtil/ThreadException.h>

#ifdef _WIN32
#   include <list>
#   include <algorithm>

using namespace std;

static CRITICAL_SECTION _criticalSection;

typedef list<CRITICAL_SECTION*> MutexList;

static MutexList* _mutexList;

//
// Although apparently not documented by Microsoft, static objects are
// initialized before DllMain/DLL_PROCESS_ATTACH and finalized after
// DllMain/DLL_PROCESS_DETACH ... that's why we use a static object.
//

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
    for(MutexList::iterator p = _mutexList->begin(); 
        p != _mutexList->end(); ++p)
    {
        DeleteCriticalSection(*p);
        delete *p;
    }
    delete _mutexList;
    DeleteCriticalSection(&_criticalSection);
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
        _mutexList->push_back(_mutex);

    }
    LeaveCriticalSection(&_criticalSection);
}
#endif

IceUtil::StaticMutex IceUtil::globalMutex = ICE_STATIC_MUTEX_INITIALIZER;
