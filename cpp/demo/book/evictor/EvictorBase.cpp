#include <EvictorBase.h>
#include <Ice/LocalException.h>

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
    // Create a cookie.
    //
    EvictorCookiePtr ec = new EvictorCookie;
    cookie = ec;

    //
    // Check if we have a servant in the map already.
    //
    EvictorMap::iterator i = _map.find(c.id);
    bool newEntry = i == _map.end();
    if(!newEntry)
    {
	//
	// Got an entry already, dequeue the entry from its current position.
	//
	ec->entry = i->second;
	_queue.erase(ec->entry->pos);
    }
    else
    {
	//
	// We do not have an entry. Ask the derived class to
	// instantiate a servant and add a new entry to the map.
	//
	ec->entry = new EvictorEntry;
	ec->entry->servant = add(c, ec->entry->userCookie); // Down-call
	if(!ec->entry->servant)
	{
	    return 0;
	}
	ec->entry->useCount = 0;
	i = _map.insert(std::make_pair(c.id, ec->entry)).first;
    }

    //
    // Increment the use count of the servant and enqueue
    // the entry at the front, so we get LRU order.
    //
    ++(ec->entry->useCount);
    ec->entry->pos = _queue.insert(_queue.begin(), i);

    return ec->entry->servant;
}

void
EvictorBase::finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock lock(_mutex);

    EvictorCookiePtr ec = EvictorCookiePtr::dynamicCast(cookie);

    //
    // Decrement use count and check if there is something to evict.
    //
    --(ec->entry->useCount);
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
    for(int i = static_cast<int>(_map.size() - _size); i > 0; --i)
    {
	EvictorQueue::reverse_iterator p = _queue.rbegin();
	if((*p)->second->useCount == 0)
	{
	    evict((*p)->second->servant, (*p)->second->userCookie); // Down-call
	    EvictorMap::iterator pos = *p;
	    _queue.erase((*p)->second->pos);
	    _map.erase(pos);
	}
    }
}
