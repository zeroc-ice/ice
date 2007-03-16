// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/StaticMutex.h>

#ifdef _WIN32
void IceUtil::StaticMutex::initialize() const
{
    //
    // First we create the new critical section
    //
    CRITICAL_SECTION* newCriticalSection = new CRITICAL_SECTION;
    InitializeCriticalSection(newCriticalSection);
	    	
    //
    // Then assign it to _mutex
    // Note that Windows performs a full memory barrier before the assignment;
    // this ensures we write the initialized critical section before we write _mutex.
    //
    if(InterlockedCompareExchangePointer(reinterpret_cast<void**>(&_mutex), newCriticalSection, 0) != 0)
    {
	//
	// Another thread was doing the same thing
	//
	DeleteCriticalSection(newCriticalSection);
	delete newCriticalSection;
    }

    //
    // This implementation does not attempt to clean up the initialized and assigned critical sections:
    // they leak
    //
}
#endif

IceUtil::StaticMutex IceUtil::globalMutex = ICE_STATIC_MUTEX_INITIALIZER;
