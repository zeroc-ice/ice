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

#include <Ice/RoutingTable.h>
#include <Glacier/Blobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::Blobject::Blobject(const CommunicatorPtr& communicator) :
    _communicator(communicator),
    _logger(communicator->getLogger())
{
}

Glacier::Blobject::~Blobject()
{
    assert(!_communicator);
    assert(!_missiveQueue);
}

void
Glacier::Blobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _communicator = 0;
    _logger = 0;

    {
	IceUtil::Mutex::Lock lock(_missiveQueueMutex);
	if(_missiveQueue)
	{
	    _missiveQueue->destroy();
	    _missiveQueueControl.join();
	    _missiveQueue = 0;
	}
    }
}

class GlacierCB : public AMI_Object_ice_invoke
{
public:

    GlacierCB(const AMD_Object_ice_invokePtr& cb) :
	_cb(cb)
    {
    }

    virtual void
    ice_response(bool ok, const ::std::vector< ::Ice::Byte>& outParams)
    {
	_cb->ice_response(ok, outParams);
    }

    virtual void
    ice_exception(const ::IceUtil::Exception& ex)
    {
	_cb->ice_exception(ex);
    }

private:

    AMD_Object_ice_invokePtr _cb;
};

void
Glacier::Blobject::invoke(ObjectPrx& proxy, const AMD_Object_ice_invokePtr& amdCB, const vector<Byte>& inParams,
			  const Current& current)
{
    try
    {
	MissiveQueuePtr missiveQueue = modifyProxy(proxy, current);

	if(missiveQueue) // Batch routing?
	{
	    vector<Byte> dummy;
	    amdCB->ice_response(true, dummy);

	    missiveQueue->add(new Missive(proxy, inParams, current, _forwardContext));
	}
	else // Regular routing.
	{
	    if(_traceLevel >= 2)
	    {
		Trace out(_logger, "Glacier");
		if(reverse())
		{
		    out << "reverse ";
		}
		out << "routing to:\n"
		    << "proxy = " << _communicator->proxyToString(proxy) << '\n'
		    << "operation = " << current.operation << '\n'
		    << "mode = " << current.mode;
	    }

	    if(proxy->ice_isOneway() || proxy->ice_isDatagram())
	    {
		vector<Byte> dummy;
		amdCB->ice_response(true, dummy);

		if(_forwardContext)
		{
		    proxy->ice_invoke(current.operation, current.mode, inParams, dummy, current.ctx);
		}
		else
		{
		    proxy->ice_invoke(current.operation, current.mode, inParams, dummy);
		}
	    }
	    else
	    {
		AMI_Object_ice_invokePtr amiCB = new GlacierCB(amdCB);

		if(_forwardContext)
		{
		    proxy->ice_invoke_async(amiCB, current.operation, current.mode, inParams, current.ctx);
		}
		else
		{
		    proxy->ice_invoke_async(amiCB, current.operation, current.mode, inParams);
		}
	    }
	}
    }
    catch(const Exception& ex)
    {
	if(_traceLevel >= 1)
	{
	    Trace out(_logger, "Glacier");
	    if(reverse())
	    {
		out << "reverse ";
	    }
	    out << "routing exception:\n" << ex;
	}

	ex.ice_throw();
    }

    return;
}

MissiveQueuePtr
Glacier::Blobject::modifyProxy(ObjectPrx& proxy, const Current& current)
{
    if(!current.facet.empty())
    {
	proxy = proxy->ice_newFacet(current.facet);
    }

    MissiveQueuePtr missiveQueue;
    Context::const_iterator p = current.ctx.find("_fwd");
    if(p != current.ctx.end())
    {
	for(unsigned int i = 0; i < p->second.length(); ++i)
	{
	    char option = p->second[i];
	    switch(option)
	    {
		case 't':
		{
		    proxy = proxy->ice_twoway();
		    missiveQueue = 0;
		    break;
		}
		
		case 'o':
		{
		    proxy = proxy->ice_oneway();
		    missiveQueue = 0;
		    break;
		}
		
		case 'd':
		{
		    proxy = proxy->ice_datagram();
		    missiveQueue = 0;
		    break;
		}
		
		case 'O':
		{
		    proxy = proxy->ice_batchOneway();
		    missiveQueue = getMissiveQueue();
		    break;
		}
		
		case 'D':
		{
		    proxy = proxy->ice_batchDatagram();
		    missiveQueue = getMissiveQueue();
		    break;
		}
		
		case 's':
		{
		    proxy = proxy->ice_secure(true);
		    break;
		}
		
		default:
		{
		    Warning out(_logger);
		    out << "unknown forward option `" << option << "'";
		    break;
		}
	    }
	}
    }

    return missiveQueue;
}

MissiveQueuePtr
Glacier::Blobject::getMissiveQueue()
{
    //
    // Lazy missive queue initialization.
    //
    IceUtil::Mutex::Lock lock(_missiveQueueMutex);
    if(!_missiveQueue)
    {
	_missiveQueue = new MissiveQueue(_communicator, _traceLevel, reverse(), _batchSleepTime);
	_missiveQueueControl = _missiveQueue->start();
    }
    return _missiveQueue;
}
