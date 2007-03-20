// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <SimpleEvictor.h>
#include <ItemI.h>

using namespace std;
using namespace IceUtil;

namespace
{
int cacheMisses = 0;
}

SimpleEvictor::SimpleEvictor(CurrentDatabase& currentDb, int size) :
    EvictorBase(size),
    _currentDb(currentDb)
{
}
    

Ice::ObjectPtr 
SimpleEvictor::add(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{
    cookie = 0;

    {
        StaticMutex::Lock lock(globalMutex);
        cacheMisses++;
        if(cacheMisses % 1000 == 0)
        {
            cout << cacheMisses << " cache misses" << endl;
        }
    }
    
    //
    // You can simulate more expensing cache-misses by adding a sleep here:
    //
    // ThreadControl::sleep(Time::milliSeconds(1));

    //
    // Use a const Database& to avoid starting a transaction 
    // (just an optimization)
    //
    const Database& cdb = _currentDb.get();
    Database::const_iterator p = cdb.find(current.id.name);
    if(p == cdb.end())
    {
        return 0;
    }
    else
    {
        return new ItemI(_currentDb, p->second);
    }
}

void 
SimpleEvictor::evict(const Ice::ObjectPtr& current, const Ice::LocalObjectPtr&)
{
    //
    // Nothing to do
    //
}
