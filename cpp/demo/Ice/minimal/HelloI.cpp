// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <HelloI.h>
#include <Ice/Ice.h>

using namespace std;

void
HelloI::sayHello(const Ice::Current&) const
{
    cout << "Hello World!" << endl;
}

#include <EvictorBase.h>

EvictorBase::EvictorBase(Ice::Int size)
    : _size(size)
{
    if (_size < 0)
    {
        _size = 1000;
    }
}

Ice::ObjectPtr
EvictorBase::locate(const Ice::Current& c, Ice::LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock lock(_mutex);

    //
    // Check if we have a servant in the map already.
    //
    EvictorEntryPtr entry;
    EvictorMap::iterator i = _map.find(c.id);
    bool newEntry = i == _map.end();
    if(!newEntry)
    {
        //
        // Got an entry already, dequeue the entry from its current position.
        //
        entry = i->second;
        _queue.erase(entry->pos);
    }
    else
    {
        //
        // We do not have an entry. Ask the derived class to
        // instantiate a servant and add a new entry to the map.
        //
        EvictorEntryPtr entry = new EvictorEntry;
        entry->servant = add(c, entry->userCookie); // Down-call
        if(!entry->servant)
        {
            return 0;
        }
        entry->useCount = 0;
        i = _map.insert(std::make_pair(c.id, entry)).first;
    }

    //
    // Increment the use count of the servant and enqueue
    // the entry at the front, so we get LRU order.
    //
    ++(entry->useCount);
    entry->pos = _queue.insert(_queue.begin(), i);

    return entry->servant;
}

void
EvictorBase::finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock lock(_mutex);

    EvictorEntryPtr entry = EvictorEntryPtr::dynamicCast(cookie);

    //
    // Decrement use count and check if there is something to evict.
    //
    --(entry->useCount);
    evictServants();
}

void
EvictorBase::deactivate(const std::string& category)
{
    IceUtil::Mutex::Lock lock(_mutex);

    _size = 0;
    evictServants();
}

void
EvictorBase::evictServants()
{
    //
    // If the evictor queue has grown larger than the limit,
    // look at the excess elements to see whether any of them
    // can be evicted.
    //
    EvictorQueue::reverse_iterator p = _queue.rbegin();
    int excessEntries = static_cast<int>(_map.size() - _size);
    for(int i = 0; i < excessEntries; ++i)
    {
        EvictorMap::iterator pos = *p++;
        if(pos->second->useCount == 0)
        {
            evict(pos->second->servant, pos->second->userCookie); // Down-call
            _queue.erase(pos->second->pos);
            _map.erase(pos);
        }
    }
}
