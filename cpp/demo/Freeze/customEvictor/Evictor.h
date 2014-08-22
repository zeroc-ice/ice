// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef EVICTOR_H
#define EVICTOR_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <CurrentDatabase.h>
#include <ItemI.h>
#include <list>

struct EvictorEntry;
typedef IceUtil::Handle<EvictorEntry> EvictorEntryPtr;

//
// A map that allows lookups while loading from storage.
//
class EvictorCache : public IceUtil::Cache<Ice::Identity, EvictorEntry>
{
public:

    EvictorCache(CurrentDatabase&);

protected:

    virtual EvictorEntryPtr load(const Ice::Identity&);
    virtual void pinned(const EvictorEntryPtr&, Position);

private:

    CurrentDatabase& _currentDb;
};

//
// We'll keep our entries in a queue.
//
typedef std::list<EvictorEntryPtr> EvictorQueue;

struct EvictorEntry : public Ice::LocalObject
{
    EvictorEntry(const ItemIPtr&);

    //
    // Immutable after initialization / insertion into the Cache map.
    //
    const Ice::ObjectPtr servant;
    EvictorCache::Position cachePosition;
   
    //
    // Protected by the Evictor mutex.
    //
    EvictorQueue::iterator queuePosition;
    int useCount;
    bool stale; // stale is true when the entry is no longer (or not yet) in the Cache map
};

class Evictor : public Ice::ServantLocator
{
public:
    
    Evictor(CurrentDatabase&, int);
    
    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);
    
private:

    void evict();

    EvictorCache _cache;
    IceUtil::Mutex _mutex;

    EvictorQueue _queue;
    int _queueSize;
    int _size;
};

#endif
