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
static list<CRITICAL_SECTION*>* _criticalSectionList; 

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
    _criticalSectionList = new list<CRITICAL_SECTION*>;
}

Init::~Init()
{
    for(list<CRITICAL_SECTION*>::iterator p = _criticalSectionList->begin(); 
	p != _criticalSectionList->end(); ++p)
    {
	DeleteCriticalSection(*p);
	delete *p;
    }
  
    delete _criticalSectionList;
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
	_mutexInitialized = true;
	_criticalSectionList->push_back(_mutex);
    }
    LeaveCriticalSection(&_criticalSection);
}

#endif

IceUtil::StaticMutex IceUtil::globalMutex = ICE_STATIC_MUTEX_INITIALIZER;

