// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/RequestQueue.h>
#include <set>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::Request::Request(const ObjectPrx& proxy, const vector<Byte>& inParams, const Current& current,
			   const AMI_Object_ice_invokePtr& amiCB) :
    _proxy(proxy),
    _inParams(inParams),
    _current(current),
    _amiCB(amiCB)
{
    Context::const_iterator p = current.ctx.find("_ovrd");
    if(p != current.ctx.end())
    {
	_override = p->second;
    }
}

void
Glacier2::Request::invoke(bool forwardContext)
{
    if(_proxy->ice_isTwoway())
    {
	assert(_amiCB);
	try
	{
	    if(forwardContext)
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
	if(forwardContext)
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
Glacier2::Request::override(const RequestPtr& other) const
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
Glacier2::Request::getProxy() const
{
    return _proxy;
}

const Current&
Glacier2::Request::getCurrent() const
{
    return _current;
}

static const string serverTraceRequest = "Glacier2.Server.Trace.Request";
static const string clientTraceRequest = "Glacier2.Client.Trace.Request";
static const string serverTraceOverride = "Glacier2.Server.Trace.Override";
static const string clientTraceOverride = "Glacier2.Client.Trace.Override";
static const string serverForwardContext = "Glacier2.Server.ForwardContext";
static const string clientForwardContext = "Glacier2.Client.ForwardContext";
static const string serverSleepTime = "Glacier2.Server.SleepTime";
static const string clientSleepTime = "Glacier2.Client.SleepTime";

Glacier2::RequestQueue::RequestQueue(const Ice::CommunicatorPtr& communicator, bool reverse) :
    _logger(communicator->getLogger()),
    _reverse(reverse),
    _traceLevelRequest(reverse ?
		       communicator->getProperties()->getPropertyAsInt(serverTraceRequest) :
		       communicator->getProperties()->getPropertyAsInt(clientTraceRequest)),
    _traceLevelOverride(reverse ?
			communicator->getProperties()->getPropertyAsInt(serverTraceOverride) :
			communicator->getProperties()->getPropertyAsInt(clientTraceOverride)),
    _forwardContext(reverse ?
		    communicator->getProperties()->getPropertyAsInt(serverForwardContext) > 0 :
		    communicator->getProperties()->getPropertyAsInt(clientForwardContext) > 0),
    _sleepTime(reverse ?
	       IceUtil::Time::milliSeconds(communicator->getProperties()->getPropertyAsInt(serverSleepTime)) :
	       IceUtil::Time::milliSeconds(communicator->getProperties()->getPropertyAsInt(clientSleepTime))),
    _destroy(false)
{
}

Glacier2::RequestQueue::~RequestQueue()
{
    assert(_destroy);
    assert(_requests.empty());
}

void 
Glacier2::RequestQueue::destroy()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);

    assert(!_destroy);

    _destroy = true;
    _requests.clear();
    
    notify();
}

void 
Glacier2::RequestQueue::addRequest(const RequestPtr& request)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
    
    assert(!_destroy);

    for(vector<RequestPtr>::iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(request->override(*p))
        {
            *p = request; // Replace old request if this is an override.

	    if(_traceLevelOverride >= 1)
	    {
		traceRequest(request, "override");
	    }

            return;
        }
    }

    _requests.push_back(request); // No override, add new request.

    notify();
}

void 
Glacier2::RequestQueue::run()
{
    while(true)
    {
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

	    requests.swap(_requests);
	}
        
        //
        // Send requests, flush batch requests, and sleep outside the
        // thread synchronization, so that new messages can be added
        // while this is being done.
        //

        try
        {
	    set<ConnectionPtr> flushSet;

	    for(vector<RequestPtr>::const_iterator p = requests.begin(); p != requests.end(); ++p)
	    {
		const ObjectPrx& proxy = (*p)->getProxy();

		if(proxy->ice_batchOneway() || proxy->ice_batchDatagram())
		{
		    flushSet.insert(proxy->ice_getConnection());
		}

		if(_traceLevelRequest >= 1)
		{
		    traceRequest(*p, "");
		}

		(*p)->invoke(_forwardContext);
	    }

	    for_each(flushSet.begin(), flushSet.end(), Ice::voidMemFun(&Connection::flushBatchRequests));
	}
        catch(const Ice::Exception& ex)
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(*this);
            
	    if(_traceLevelRequest >= 1)
	    {
		Trace out(_logger, "Glacier2");
		if(_reverse)
		{
		    out << "reverse ";
		}
		out << "routing exception:\n" << ex;
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

void
Glacier2::RequestQueue::traceRequest(const RequestPtr& request, const string& extra) const
{
    Trace out(_logger, "Glacier2");
    
    const ObjectPrx& proxy = request->getProxy();
    const Current& current = request->getCurrent();

    if(_reverse)
    {
	out << "reverse ";
    }

    out << "routing";

    if(!extra.empty())
    {
	out << ' ' << extra;
    }

    out << "\nproxy = " << current.adapter->getCommunicator()->proxyToString(proxy);

    out << "\noperation = " << current.operation;

    out << "\ncontext = ";
    Context::const_iterator q = current.ctx.begin();
    while(q != current.ctx.end())
    {
	out << q->first << '/' << q->second;
	if(++q != current.ctx.end())
	{
	    out << ", ";
	}
    }
}
