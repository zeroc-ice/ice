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

Glacier::TwowayThrottle::TwowayThrottle(const CommunicatorPtr& communicator, bool reverse) :
    _communicator(communicator),
    _reverse(reverse),
    _properties(_communicator->getProperties()),
    _logger(_communicator->getLogger()),
    _traceLevel(_properties->getPropertyAsInt("Glacier.Router.Trace.Throttle")),
    _max(_reverse ?
	 _properties->getPropertyAsInt("Glacier.Router.Server.Throttle.Twoways") :
	 _properties->getPropertyAsInt("Glacier.Router.Client.Throttle.Twoways")),
    _count(0)
{
}

Glacier::TwowayThrottle::~TwowayThrottle()
{
    assert(_count == 0);
}

void
Glacier::TwowayThrottle::twowayStarted(const Ice::ObjectPrx& proxy, const Ice::Current& current)
{
    if(_max <= 0)
    {
	return;
    }
    
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	assert(_count <= _max);
	
	while(_count == _max)
	{
	    if(_traceLevel >= 1)
	    {
		Trace out(_logger, "Glacier");
		out << "throttling ";
		if(_reverse)
		{
		    out << "reverse ";
		}
		out << "twoway call:";
		out << "\nnumber of calls = " << _count;
		out << "\nproxy = " << _communicator->proxyToString(proxy);
		out << "\noperation = " << current.operation;
	    }

	    wait();
	}
	
	++_count;
    }
}

void
Glacier::TwowayThrottle::twowayFinished()
{
    if(_max <= 0)
    {
	return;
    }
    
    {
	IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
	
	assert(_count <= _max);
	
	if(_count == _max)
	{
	    notifyAll();
	}
	
	--_count;
    }
}


#ifdef __HP_aCC
//
// Compiler bug!
// The conditional in Glacier::Blobject::Blobject below result in a
// std::exception "thread synchronization error" at runtime
// when using string litterals (looks like a RogueWave bug)
// The work around is to use static strings:
//

static const string traceServer = "Glacier.Router.Trace.Server";
static const string traceClient = "Glacier.Router.Trace.Client";

static const string serverForwardContext = "Glacier.Router.Server.ForwardContext";
static const string clientForwardContext = "Glacier.Router.Client.ForwardContext";

static const string serverSleepTime = "Glacier.Router.Server.SleepTime";
static const string clientSleepTime = "Glacier.Router.Client.SleepTime";
#endif

Glacier::Blobject::Blobject(const CommunicatorPtr& communicator, bool reverse) :
    _communicator(communicator),
    _reverse(reverse),
    _properties(_communicator->getProperties()),
    _logger(_communicator->getLogger()),

#ifdef __HP_aCC
    // 
    // Compiler bug, see above
    //
    _traceLevel(_reverse ?
		_properties->getPropertyAsInt(traceServer) :
		_properties->getPropertyAsInt(traceClient)),
    _forwardContext(_reverse ?
		    _properties->getPropertyAsInt(serverForwardContext) > 0 :
		    _properties->getPropertyAsInt(clientForwardContext) > 0),
    _sleepTime(_reverse ?
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(serverSleepTime)) :
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(clientSleepTime))),

#else
    _traceLevel(_reverse ?
		_properties->getPropertyAsInt("Glacier.Router.Trace.Server") :
		_properties->getPropertyAsInt("Glacier.Router.Trace.Client")),
    _forwardContext(_reverse ?
		    _properties->getPropertyAsInt("Glacier.Router.Server.ForwardContext") > 0 :
		    _properties->getPropertyAsInt("Glacier.Router.Client.ForwardContext") > 0),
    _sleepTime(_reverse ?
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt("Glacier.Router.Server.SleepTime")) :
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt("Glacier.Router.Client.SleepTime"))),
#endif   

    _twowayThrottle(_communicator, _reverse)
{
    _requestQueue = new RequestQueue(_communicator, _traceLevel, _reverse, _sleepTime);
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

    GlacierCB(const AMD_Object_ice_invokePtr& cb, TwowayThrottle& twowayThrottle) :
	_cb(cb),
	_twowayThrottle(twowayThrottle)
    {
    }

    virtual
    ~GlacierCB()
    {
	_twowayThrottle.twowayFinished();
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
    TwowayThrottle& _twowayThrottle;
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
		amiCB = new GlacierCB(amdCB, _twowayThrottle);
		_twowayThrottle.twowayStarted(proxy, current);
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
