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

#include <Ice/RoutingTable.h>
#include <Glacier/Blobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::Blobject::Blobject(const CommunicatorPtr& communicator, bool reverse) :
    _communicator(communicator),
    _logger(communicator->getLogger()),
    _reverse(reverse)
{
    PropertiesPtr properties = _communicator->getProperties();
    if(_reverse)
    {
	const_cast<Int&>(_traceLevel) = properties->getPropertyAsInt("Glacier.Router.Trace.Server");
	const_cast<bool&>(_forwardContext) = properties->getPropertyAsInt("Glacier.Router.Server.ForwardContext") > 0;
	const_cast<IceUtil::Time&>(_batchSleepTime) = IceUtil::Time::milliSeconds(
	    properties->getPropertyAsIntWithDefault("Glacier.Router.Server.BatchSleepTime", 250));
    }
    else
    {
	const_cast<Int&>(_traceLevel) = properties->getPropertyAsInt("Glacier.Router.Trace.Client");
	const_cast<bool&>(_forwardContext) = properties->getPropertyAsInt("Glacier.Router.Client.ForwardContext") > 0;
	const_cast<IceUtil::Time&>(_batchSleepTime) = IceUtil::Time::milliSeconds(
	    properties->getPropertyAsIntWithDefault("Glacier.Router.Client.BatchSleepTime", 250));
    }

    _requestQueue = new RequestQueue(_communicator, _traceLevel, _reverse, _batchSleepTime);
    _requestQueueControl = _requestQueue->start();
}

Glacier::Blobject::~Blobject()
{
    assert(!_communicator);
    assert(!_requestQueue);
}

void
Glacier::Blobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _communicator = 0;

    _requestQueue->destroy();
    _requestQueueControl.join();
    _requestQueue = 0;
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
	bool missive = modifyProxy(proxy, current);

	if(missive) // Batch routing?
	{
	    vector<Byte> dummy;
	    amdCB->ice_response(true, dummy);

	    _requestQueue->addMissive(new Request(proxy, inParams, current, _forwardContext));
	}
	else // Regular routing.
	{
	    AMI_Object_ice_invokePtr amiCB;

	    if(proxy->ice_isTwoway())
	    {
		amiCB = new GlacierCB(amdCB);
	    }
	    else
	    {
		vector<Byte> dummy;
		amdCB->ice_response(true, dummy);
	    }

	    _requestQueue->addRequest(new Request(proxy, inParams, current, _forwardContext, amiCB));
	}
    }
    catch(const Exception& ex)
    {
	if(_traceLevel >= 1)
	{
	    Trace out(_logger, "Glacier");
	    if(_reverse)
	    {
		out << "reverse ";
	    }
	    out << "routing exception:\n" << ex;
	}

	ex.ice_throw();
    }

    return;
}

bool
Glacier::Blobject::modifyProxy(ObjectPrx& proxy, const Current& current)
{
    if(!current.facet.empty())
    {
	proxy = proxy->ice_newFacet(current.facet);
    }

    bool missive = false;

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
		    missive = false;
		    break;
		}
		
		case 'o':
		{
		    proxy = proxy->ice_oneway();
		    missive = false;
		    break;
		}
		
		case 'd':
		{
		    proxy = proxy->ice_datagram();
		    missive = false;
		    break;
		}
		
		case 'O':
		{
		    proxy = proxy->ice_batchOneway();
		    missive = true;
		    break;
		}
		
		case 'D':
		{
		    proxy = proxy->ice_batchDatagram();
		    missive = true;
		    break;
		}
		
		case 's':
		{
		    proxy = proxy->ice_secure(true);
		    break;
		}
		
		case 'z':
		{
		    proxy = proxy->ice_compress(true);
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

    return missive;
}
