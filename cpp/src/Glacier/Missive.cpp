// **********************************************************************
//
// Copyright (c) 2002
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

#include <Glacier/Missive.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::Missive::Missive(const ObjectPrx& proxy, const vector<Byte>& inParams, const Current& current,
			  bool forwardContext) :
    _proxy(proxy),
    _inParams(inParams),
    _current(current),
    _forwardContext(forwardContext)
{
    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
	_override = p->second;
    }
}

void
Glacier::Missive::invoke()
{
    std::vector<Byte> dummy;
    if(_forwardContext)
    {
	_proxy->ice_invoke(_current.operation, _current.mode, _inParams, dummy, _current.ctx);
    }
    else
    {
	_proxy->ice_invoke(_current.operation, _current.mode, _inParams, dummy);
    }
}

bool
Glacier::Missive::override(const MissivePtr& other)
{
    if(_override.empty() || other->_override.empty())
    {
	return false;
    }

    return _override == other->_override;
}

const ObjectPrx&
Glacier::Missive::getProxy() const
{
    return _proxy;
}

const Current&
Glacier::Missive::getCurrent() const
{
    return _current;
}

Glacier::MissiveQueue::MissiveQueue(const Ice::CommunicatorPtr& communicator, int traceLevel, bool reverse,
				    const IceUtil::Time& sleepTime) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _traceLevel(traceLevel),
    _reverse(reverse),
    _sleepTime(sleepTime),
    _destroy(false)
{
}

Glacier::MissiveQueue::~MissiveQueue()
{
    assert(_destroy);
    assert(_missives.empty());
    assert(!_communicator);
}

void 
Glacier::MissiveQueue::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    _destroy = true;
    _missives.clear();
    _communicator = 0;
    _logger = 0;
    
    notify();
}

void 
Glacier::MissiveQueue::add(const MissivePtr& missive)
{
    assert(missive);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    if(_missives.empty())
    {
        notify();
    }
    
    for(std::vector<MissivePtr>::iterator p = _missives.begin(); p != _missives.end(); ++p)
    {
        if(missive->override(*p))
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
    while(true)
    {
	vector<ObjectPrx> proxies;
	
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
            
            while(!_destroy && _missives.empty())
            {
                wait();
            }
            
            if(_destroy)
            {
                return;
            }
            
	    proxies.reserve(_missives.size());
	    vector<MissivePtr>::const_iterator p;
	    for(p = _missives.begin(); p != _missives.end(); ++p)
	    {
		try
		{
		    const ObjectPrx& proxy = (*p)->getProxy();
		    const Current& current = (*p)->getCurrent();

		    if(_traceLevel >= 2)
		    {
			Trace out(_logger, "Glacier");
			if(_reverse)
			{
			    out << "reverse ";
			}
			out << "batch routing to:\n"
			    << "proxy = " << _communicator->proxyToString(proxy) << '\n'
			    << "operation = " << current.operation << '\n'
			    << "mode = " << current.mode;
		    }
		    
		    (*p)->invoke();
		    proxies.push_back(proxy);
		}
		catch(const Ice::Exception& ex)
		{
		    if(_traceLevel >= 1)
		    {
			Trace out(_logger, "Glacier");
			if(_reverse)
			{
			    out << "reverse ";
			}
			out << "batch routing exception:\n" << ex;
		    }
		}
	    }

	    _missives.clear();
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
//	    sort(proxies.begin(), proxies.end());
//	    proxies.erase(unique(proxies.begin(), proxies.end()), proxies.end());
            vector<ObjectPrx>::const_iterator p;
	    for(p = proxies.begin(); p != proxies.end(); ++p)
	    {
		(*p)->ice_flush();
	    }
            
            //
            // In order to avoid flooding the missive receivers, we add
            // a delay between sending missives.
	    //
	    if(_sleepTime > IceUtil::Time())
	    {
		IceUtil::ThreadControl::sleep(_sleepTime);
	    }
        }
        catch(const Ice::Exception& ex)
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
            
	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
		if(_reverse)
		{
		    out << "reverse ";
		}
		out << "batch routing exception:\n" << ex;
	    }
        }
    }
}
