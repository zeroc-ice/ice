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

#include <Glacier/Request.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::Request::Request(const ObjectPrx& proxy, const vector<Byte>& inParams, const Current& current,
			  bool forwardContext, const AMI_Object_ice_invokePtr& amiCB) :
    _proxy(proxy),
    _inParams(inParams),
    _current(current),
    _forwardContext(forwardContext),
    _amiCB(amiCB)
{
    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
	_override = p->second;
    }
}

void
Glacier::Request::invoke()
{
    
    if(_proxy->ice_isTwoway())
    {
	assert(_amiCB);
	try
	{
	    if(_forwardContext)
	    {
		_proxy->ice_invoke_async(_amiCB, _current.operation, _current.mode, _inParams, _current.ctx);
	    }
	    else
	    {
		_proxy->ice_invoke_async(_amiCB, _current.operation, _current.mode, _inParams);
	    }
	}
	catch(const Ice::Exception& ex)
	{
	    _amiCB->ice_exception(ex);
	}
    }
    else
    {
	vector<Byte> dummy;
	if(_forwardContext)
	{
	    _proxy->ice_invoke(_current.operation, _current.mode, _inParams, dummy, _current.ctx);
	}
	else
	{
	    _proxy->ice_invoke(_current.operation, _current.mode, _inParams, dummy);
	}
    }
}

bool
Glacier::Request::override(const RequestPtr& other)
{
    if(_override.empty() || other->_override.empty())
    {
	return false;
    }

    return _override == other->_override;
}

const ObjectPrx&
Glacier::Request::getProxy() const
{
    return _proxy;
}

const Current&
Glacier::Request::getCurrent() const
{
    return _current;
}

Glacier::RequestQueue::RequestQueue(const Ice::CommunicatorPtr& communicator, int traceLevel, bool reverse,
				    const IceUtil::Time& sleepTime) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _traceLevel(traceLevel),
    _reverse(reverse),
    _sleepTime(sleepTime),
    _destroy(false)
{
}

Glacier::RequestQueue::~RequestQueue()
{
    assert(_destroy);
    assert(_missives.empty());
    assert(_requests.empty());
    assert(!_communicator);
}

void 
Glacier::RequestQueue::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    _destroy = true;
    _missives.clear();
    _requests.clear();
    _communicator = 0;
    _logger = 0;
    
    notify();
}

void 
Glacier::RequestQueue::addMissive(const RequestPtr& missive)
{
    assert(missive);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    if(_missives.empty() && _requests.empty())
    {
        notify();
    }
    
    for(vector<RequestPtr>::iterator p = _missives.begin(); p != _missives.end(); ++p)
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
Glacier::RequestQueue::addRequest(const RequestPtr& request)
{
    assert(request);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    if(_requests.empty())
    {
        notify();
    }

    _requests.push_back(request);
}

void 
Glacier::RequestQueue::run()
{
    while(true)
    {
	vector<ObjectPrx> proxies;
	vector<RequestPtr> requests;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	    //
	    // Reset the next missive time if it's not set.
	    //
	    if(_nextMissiveTime == IceUtil::Time())
	    {
		_nextMissiveTime = IceUtil::Time::now() + _sleepTime;
	    }

	    //
	    // Wait indefinitely if there's no requests or missive to
	    // send.
	    //
            while(!_destroy && _requests.empty() && _missives.empty())
            {		
		wait();
            }

            //
	    // Wait for the next missive batch to be sent if there's
	    // no requeests to send.
	    //
	    IceUtil::Time now;

	    while(!_destroy && _requests.empty())
	    {
		now = IceUtil::Time::now();
		if(now >= _nextMissiveTime)
		{
		    break;
		}
		
		timedWait(_nextMissiveTime - now);
	    }

            if(_destroy)
            {
                return;
            }

	    if(!_requests.empty())
	    {
		_requests.swap(requests);
	    }

            if(!_missives.empty() && _nextMissiveTime < now)
	    {
		_nextMissiveTime = IceUtil::Time(); // Reset the next missive time.

		proxies.reserve(_missives.size());
		vector<RequestPtr>::const_iterator p;
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
	}
        
        //
        // Flush and sleep outside the thread synchronization, so that
        // new messages can be added to this missive queue while this
        // thread sends a batch and sleeps.
        //
        try
        {
	    //
	    // This sends all requests.
	    //
	    for(vector<RequestPtr>::const_iterator p = requests.begin(); p != requests.end(); ++p)
	    {
		(*p)->invoke();
	    }

            //
            // This sends all batched missives.
            //
	    _communicator->flushBatchRequests();
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
