// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/Request.h>

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
    
    notify();
}

void 
Glacier::RequestQueue::addMissive(const RequestPtr& missive)
{
    assert(missive);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    for(vector<RequestPtr>::iterator p = _missives.begin(); p != _missives.end(); ++p)
    {
        if(missive->override(*p))
        {
            *p = missive; // Replace old missive if this is an override.
            return;
        }
    }

    _missives.push_back(missive); // No override, add new missive.

    notify();
}

void 
Glacier::RequestQueue::addRequest(const RequestPtr& request)
{
    assert(request);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    _requests.push_back(request);

    notify();
}

void 
Glacier::RequestQueue::run()
{
    while(true)
    {
	vector<RequestPtr> requests;
	vector<RequestPtr> missives;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	    //
	    // Wait indefinitely if there's no requests or missive to
	    // send.
	    //
            while(!_destroy && _requests.empty() && _missives.empty())
            {		
		wait();
            }

            if(_destroy)
            {
                return;
            }

	    if(!_requests.empty())
	    {
		_requests.swap(requests);
	    }

	    if(!_missives.empty())
	    {
		_missives.swap(missives);
	    }
	}
        
        //
        // Send requests and missives, flush batch requests, and sleep
        // outside the thread synchronization, so that new messages
        // can be added while this is being done.
        //

        try
        {
	    for(vector<RequestPtr>::const_iterator p = requests.begin(); p != requests.end(); ++p)
	    {
		if(_traceLevel >= 2)
		{
		    const ObjectPrx& proxy = (*p)->getProxy();
		    const Current& current = (*p)->getCurrent();
		    
		    Trace out(_logger, "Glacier");
		    
		    if(_reverse)
		    {
			out << "reverse ";
		    }
		    out << "routing to:\n"
			<< "\nproxy = " << _communicator->proxyToString(proxy)
			<< "\noperation = " << current.operation;
		}

		(*p)->invoke();
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
		out << "routing exception:\n" << ex;
	    }
        }

	try
	{
	    for(vector<RequestPtr>::const_iterator p = missives.begin(); p != missives.end(); ++p)
	    {
		if(_traceLevel >= 2)
		{
		    const ObjectPrx& proxy = (*p)->getProxy();
		    const Current& current = (*p)->getCurrent();
		    
		    Trace out(_logger, "Glacier");
		    
		    if(_reverse)
		    {
			out << "reverse ";
		    }
		    
		    out << "batch routing to:"
			<< "\nproxy = " << _communicator->proxyToString(proxy)
			<< "\noperation = " << current.operation;
		}
		
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
	
	//
	// In order to avoid flooding, we add a delay, if so
	// requested.
	//
	if(_sleepTime > IceUtil::Time())
	{
	    IceUtil::ThreadControl::sleep(_sleepTime);
	}
    }
}
