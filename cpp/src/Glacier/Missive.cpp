// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Glacier/Missive.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::Missive::Missive(const ObjectPrx& proxy, const vector<Byte>& inParams, const Current& current) :
    _proxy(proxy),
    _inParams(inParams),
    _current(current)
{
}

ObjectPrx
Glacier::Missive::invoke()
{
    // TODO: Should we forward the context? Perhaps a config parameter?
    std::vector<Byte> dummy;
    _proxy->ice_invoke(_current.operation, _current.nonmutating, _inParams, dummy, _current.context);
    return _proxy;
}

bool
Glacier::Missive::override(const MissivePtr& missive)
{
    return false;
}

Glacier::MissiveQueue::MissiveQueue() :
    _destroy(false)
{
}

Glacier::MissiveQueue::~MissiveQueue()
{
    assert(_destroy);
    assert(_missives.empty());
}

void 
Glacier::MissiveQueue::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    _destroy = true;
    _missives.clear();
    
    notify();
}

void 
Glacier::MissiveQueue::add(const MissivePtr& missive)
{
    assert(missive);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    if (_missives.empty())
    {
        notify();
    }
    
    for (std::vector<MissivePtr>::iterator p = _missives.begin(); p != _missives.end(); ++p)
    {
        if (missive->override(*p))
        {
            *p = missive; // Replace old missive if this is an override.
            return;
        }
    }

    _missives.push_back(missive); // No override, add new missive.
}

void 
Glacier::MissiveQueue::run()
{
    while (true)
    {
	vector<ObjectPrx> proxies;
	
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
            
            while (!_destroy && _missives.empty())
            {
                wait();
            }
            
            if (_destroy)
            {
                return;
            }
            
	    proxies.reserve(_missives.size());
	    vector<MissivePtr>::const_iterator p;
	    for (p = _missives.begin(); p != _missives.end(); ++p)
	    {
		try
		{
		    proxies.push_back((*p)->invoke());
		}
		catch (const Ice::Exception& ex)
		{
		    //
		    // Remember exception and destroy the missive queue.
		    //
		    _destroy = true;
		    _missives.clear();
		    _exception = std::auto_ptr<Ice::Exception>(ex.ice_clone());
		    return;
		}
		_missives.clear();
	    }
	}
        
        //
        // Flush and sleep outside the thread synchronization, so that
        // new messages can be added to this missive queue while this
        // thread sends a batch and sleeps.
        //
        try
        {
            //
            // This sends all batched missives.
            //
	    sort(proxies.begin(), proxies.end());
	    proxies.erase(unique(proxies.begin(), proxies.end()), proxies.end());
            vector<ObjectPrx>::const_iterator p;
	    for (p = proxies.begin(); p != proxies.end(); ++p)
	    {
		(*p)->ice_flush();
	    }
            
            //
            // In order to avoid flooding the missive receivers, we add
            // a delay between sending missives.
	    //
	    // TODO: Configurable.
            //
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(250));
        }
        catch (const Ice::Exception& ex)
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
            
            //
            // Remember exception and destroy the missive queue.
            //
            _destroy = true;
            _missives.clear();
            _exception = std::auto_ptr<Ice::Exception>(ex.ice_clone());
            return;
        }
    }
}
