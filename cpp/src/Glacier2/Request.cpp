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
    //
    // Both override values have to be non-empty.
    //
    if(_override.empty() || other->_override.empty())
    {
	return false;
    }

    //
    // Override does not work for twoways, because a response is
    // expected for each request.
    //
    if(_proxy->ice_isTwoway() || other->_proxy->ice_isTwoway())
    {
	return false;
    }

    //
    // We cannot override if the proxies differ.
    //
    if(_proxy != other->_proxy)
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
    assert(_requests.empty());
    assert(!_communicator);
}

void 
Glacier::RequestQueue::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    _destroy = true;
    _requests.clear();
    _communicator = 0;
    
    notify();
}

void 
Glacier::RequestQueue::addRequest(const RequestPtr& request)
{
    assert(request);

    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    for(vector<RequestPtr>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(request->override(*p))
        {
            *p = request; // Replace old request if this is an override.
            return;
        }
    }

    _requests.push_back(request); // No override, add new request.

    notify();
}

void 
Glacier::RequestQueue::run()
{
    while(true)
    {
	CommunicatorPtr communicator;
	vector<RequestPtr> requests;

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

	    //
	    // Wait indefinitely if there's no requests to send.
	    //
            while(!_destroy && _requests.empty())
            {		
		wait();
            }

            if(_destroy)
            {
                return;
            }

	    communicator = _communicator;
	    requests.swap(_requests);
	}
        
        //
        // Send requests, flush batch requests, and sleep outside the
        // thread synchronization, so that new messages can be added
        // while this is being done.
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
		    out << "routing to:"
			<< "\nproxy = " << communicator->proxyToString(proxy)
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
	    //
	    // This sends all batched requests.
	    //
	    communicator->flushBatchRequests();
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
