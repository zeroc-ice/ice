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
using namespace Glacier2;

static const string serverAlwaysBatch = "Glacier2.Server.AlwaysBatch";
static const string clientAlwaysBatch = "Glacier2.Client.AlwaysBatch";
static const string serverThreadStackSize = "Glacier2.Server.ThreadStackSize";
static const string clientThreadStackSize = "Glacier2.Client.ThreadStackSize";

Glacier2::Blobject::Blobject(const CommunicatorPtr& communicator, bool reverse) :
    _properties(communicator->getProperties()),
    _logger(communicator->getLogger()),
    _alwaysBatch(reverse ?
		 _properties->getPropertyAsInt(serverAlwaysBatch) > 0 :
		 _properties->getPropertyAsInt(clientAlwaysBatch) > 0)
{
    Int threadStackSize = reverse ?
	_properties->getPropertyAsInt(serverThreadStackSize) :
	_properties->getPropertyAsInt(clientThreadStackSize);

    _requestQueue = new RequestQueue(communicator, reverse);
    _requestQueue->start(static_cast<size_t>(threadStackSize));
}

Glacier2::Blobject::~Blobject()
{
    assert(!_requestQueue);
}

void
Glacier2::Blobject::destroy()
{
    assert(_requestQueue); // Destroyed?
    _requestQueue->destroy();
    _requestQueue = 0;
}

class Glacier2CB : public AMI_Object_ice_invoke
{
public:

    Glacier2CB(const AMD_Object_ice_invokePtr& cb) :
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
Glacier2::Blobject::invoke(ObjectPrx& proxy, const AMD_Object_ice_invokePtr& amdCB, const vector<Byte>& inParams,
			  const Current& current)
{
    assert(_requestQueue); // Destroyed?

    //
    // Set the correct facet on the proxy.
    //
    if(!current.facet.empty())
    {
	proxy = proxy->ice_newFacet(current.facet);
    }

    //
    // Modify the proxy according to the _fwd context field.
    //
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
		    break;
		}
		
		case 'o':
		{
		    if(_alwaysBatch)
		    {
			proxy = proxy->ice_batchOneway();
		    }
		    else
		    {
			proxy = proxy->ice_oneway();
		    }
		    break;
		}
		
		case 'd':
		{
		    if(_alwaysBatch)
		    {
			proxy = proxy->ice_batchDatagram();
		    }
		    else
		    {
			proxy = proxy->ice_datagram();
		    }
		    break;
		}
		
		case 'O':
		{
		    proxy = proxy->ice_batchOneway();
		    break;
		}
		
		case 'D':
		{
		    proxy = proxy->ice_batchDatagram();
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
    
    //
    // Create a new request and add it to the request queue.
    //
    if(proxy->ice_isTwoway())
    {
	AMI_Object_ice_invokePtr amiCB = new Glacier2CB(amdCB);
	_requestQueue->addRequest(new Request(proxy, inParams, current, amiCB));
    }
    else
    {
	vector<Byte> dummy;
	amdCB->ice_response(true, dummy);
	_requestQueue->addRequest(new Request(proxy, inParams, current, 0));
    }
}
