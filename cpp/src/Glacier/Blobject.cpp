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


bool
Glacier::Blobject::invoke(ObjectPrx& proxy, const vector<Byte>& inParams, vector<Byte>& outParams,
			  const Current& current)
{
    try
    {
	MissiveQueuePtr missiveQueue = modifyProxy(proxy, current);

	if(missiveQueue) // Batch routing?
	{
	    missiveQueue->add(new Missive(proxy, inParams, current, _forwardContext));
	    return true;
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
	    
	    if(_forwardContext)
	    {
		return proxy->ice_invoke(current.operation, current.mode, inParams, outParams, current.ctx);
	    }
	    else
	    {
		return proxy->ice_invoke(current.operation, current.mode, inParams, outParams);
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

    assert(false);
    return true; // To keep the compiler happy.
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
