// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/Blobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

#ifdef __HP_aCC
//
// Compiler bug!
// The conditional in Glacier::Blobject::Blobject below result in a
// std::exception "thread synchronization error" at runtime
// when using string literals (looks like a RogueWave bug)
// The work around is to use static strings:
//

static const string traceServer = "Glacier2.Trace.Server";
static const string traceClient = "Glacier2.Trace.Client";

static const string serverForwardContext = "Glacier2.Server.ForwardContext";
static const string clientForwardContext = "Glacier2.Client.ForwardContext";

static const string serverSleepTime = "Glacier2.Server.SleepTime";
static const string clientSleepTime = "Glacier2.Client.SleepTime";
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
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt(clientSleepTime)))

#else
    _traceLevel(_reverse ?
		_properties->getPropertyAsInt("Glacier2.Trace.Server") :
		_properties->getPropertyAsInt("Glacier2.Trace.Client")),
    _forwardContext(_reverse ?
		    _properties->getPropertyAsInt("Glacier2.Server.ForwardContext") > 0 :
		    _properties->getPropertyAsInt("Glacier2.Client.ForwardContext") > 0),
    _sleepTime(_reverse ?
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt("Glacier2.Server.SleepTime")) :
	       IceUtil::Time::milliSeconds(_properties->getPropertyAsInt("Glacier2.Client.SleepTime")))
#endif   
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

    GlacierCB(const AMD_Object_ice_invokePtr& cb) :
	_cb(cb)
    {
    }

    virtual void
    ice_response(bool ok, const vector<Byte>& outParams)
    {
	_cb->ice_response(ok, outParams);
    }

    virtual void
    ice_exception(const Exception& ex)
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
	modifyProxy(proxy, current);

	if(proxy->ice_isTwoway())
	{
	    AMI_Object_ice_invokePtr amiCB = new GlacierCB(amdCB);
	    _requestQueue->addRequest(new Request(proxy, inParams, current, _forwardContext, amiCB));
	}
	else
	{
	    vector<Byte> dummy;
	    amdCB->ice_response(true, dummy);
	    _requestQueue->addRequest(new Request(proxy, inParams, current, _forwardContext, 0));
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

void
Glacier::Blobject::modifyProxy(ObjectPrx& proxy, const Current& current) const
{
    if(!current.facet.empty())
    {
	proxy = proxy->ice_newFacet(current.facet);
    }

    bool batch = false;

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
		    batch = false;
		    break;
		}
		
		case 'o':
		{
		    proxy = proxy->ice_oneway();
		    batch = false;
		    break;
		}
		
		case 'd':
		{
		    proxy = proxy->ice_datagram();
		    batch = false;
		    break;
		}
		
		case 'O':
		{
		    proxy = proxy->ice_batchOneway();
		    batch = true;
		    break;
		}
		
		case 'D':
		{
		    proxy = proxy->ice_batchDatagram();
		    batch = true;
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
}
