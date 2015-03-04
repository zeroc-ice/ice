// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Evictor.h>
#include <IceUtil/Mutex.h>

using namespace std;
using namespace IceUtil;

namespace
{

int cacheMisses = 0;
IceUtil::Mutex* globalMutex = 0;

class Init
{
public:

    Init()
    {
        globalMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete globalMutex;
        globalMutex = 0;
    }
};

Init init;

}

//
// EvictorCache
//
EvictorCache::EvictorCache(CurrentDatabase& currentDb) :
    _currentDb(currentDb)
{
}

//
// Returns 0 if object is not found.
//
EvictorEntryPtr
EvictorCache::load(const Ice::Identity& itemId)
{
    {
        IceUtil::Mutex::Lock lock(*globalMutex);
        cacheMisses++;
        if(cacheMisses % 1000 == 0)
        {
            cout << cacheMisses << " cache misses" << endl;
        }
    }

    //
    // You can simulate more expensive cache-misses by adding a sleep here:
    //
    // ThreadControl::sleep(Time::milliSeconds(1));

    //
    // Use a const Database& to avoid starting a transaction (just an optimization).
    //
    const Database& cdb = _currentDb.get();
    Database::const_iterator p = cdb.find(itemId.name);
    if(p == cdb.end())
    {
        return 0;
    }
    else
    {
        return new EvictorEntry(new ItemI(_currentDb, p->second));
    }
}

//
// Finish initializing the entry after it has been inserted in the
// Cache map, but before any other thread can find it.
//
// pinned() is called while IceUtil::Cache's internal mutex is locked,
// so we must be careful with lock acquisition order. For example we
// cannot acquire the Evictor mutex here, since we also call
// IceUtil::Cache with the Evictor mutex locked.
//
void 
EvictorCache::pinned(const EvictorEntryPtr& entry, EvictorCache::Position cp)
{
    entry->stale = false;
    entry->cachePosition = cp;
}

//
// EvictorEntry
//
EvictorEntry::EvictorEntry(const ItemIPtr& item) :
    servant(item),
    useCount(-1),
    stale(true)
{
}

//
// Evictor
//
Evictor::Evictor(CurrentDatabase& currentDb, int size) :
    _cache(currentDb),
    _queueSize(0),
    _size(size)
{
}

Ice::ObjectPtr 
Evictor::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{
    cookie = 0;

    //
    // Lookup the cookie (EvictorEntry) in the cache; this will call load() 
    // if the entry is not yet in there.
    //
    // If we get an entry that was just evicted (stale == true), we try again.
    //
    for(;;)
    {
        EvictorEntryPtr entry = _cache.pin(current.id);

        if(entry == 0)
        {
            //
            // Will raise ObjectNotExistException.
            //
            return 0;
        }
        else
        {
            {
                //
                // Lock _mutex when reading/writing useCount, stale and queuePosition.
                //
                Mutex::Lock lock(_mutex);

                if(entry->stale)
                {
                    //
                    // Another thread just evicted this entry; try again.
                    //
                    continue;
                }

                cookie = entry;

                if(entry->useCount < 0)
                {
                    entry->useCount = 0;
                    _queueSize++;
                }
                else
                {
                    _queue.erase(entry->queuePosition);
                }

                _queue.push_front(entry);
                entry->queuePosition = _queue.begin();
                entry->useCount++;
            }
        }

        //
        // We have at least one useCount, and servant is immutable.
        //
        return entry->servant;
    }
}

void 
Evictor::finished(const Ice::Current& /*current*/, const Ice::ObjectPtr& /*servant*/, 
                  const Ice::LocalObjectPtr& cookie)
{
    if(cookie != 0)
    {
        Mutex::Lock lock(_mutex);

        EvictorEntryPtr entry = EvictorEntryPtr::dynamicCast(cookie);
        assert(entry != 0);

        entry->useCount--;
        assert(entry->useCount >= 0);

        evict();
    }       
}

void 
Evictor::deactivate(const string& /*category*/)
{
    Mutex::Lock lock(_mutex);
    _size = 0;
    evict();
}

void
Evictor::evict()
{
    //
    // Called with _mutex locked; try to erase the excess entries.
    //
    EvictorQueue::reverse_iterator p = _queue.rbegin();

    int toErase = (_queueSize - _size);

    while(toErase > 0 && p != _queue.rend())
    {
        //
        // Get the last unused element from the evictor queue.
        //
        while(p != _queue.rend() && (*p)->useCount != 0)
        {
            ++p;
        }

        if(p != _queue.rend())
        {
            EvictorEntryPtr& entry = *p;
            assert(!entry->stale);
            entry->stale = true;
            _cache.unpin(entry->cachePosition);

            //
            // Erase returns a normal iterator to the item after the erased item;
            // and then reverse_iterator() makes q point to the item before it.
            //
            p = EvictorQueue::reverse_iterator(_queue.erase(entry->queuePosition));
            toErase--;
            _queueSize--;
        }
    }
}
