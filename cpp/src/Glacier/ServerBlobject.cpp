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
#include <Glacier/ServerBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier;

Glacier::ServerBlobject::ServerBlobject(const ObjectAdapterPtr& clientAdapter) :
    Glacier::Blobject(clientAdapter->getCommunicator()),
    _clientAdapter(clientAdapter)
{
    PropertiesPtr properties = _communicator->getProperties();
    _traceLevel = properties->getPropertyAsInt("Glacier.Router.Trace.Server");
    _forwardContext = properties->getPropertyAsInt("Glacier.Router.Server.ForwardContext") > 0;
    _batchSleepTime = IceUtil::Time::milliSeconds(
	properties->getPropertyAsIntWithDefault("Glacier.Router.Server.BatchSleepTime", 250));
}

bool
Glacier::ServerBlobject::reverse()
{
    return true;
}

void
Glacier::ServerBlobject::destroy()
{
    //
    // No mutex protection necessary, destroy is only called after all
    // object adapters have shut down.
    //
    _clientAdapter = 0;
    Blobject::destroy();
}

void
Glacier::ServerBlobject::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB, const vector<Byte>& inParams,
					  const Current& current)
{
    assert(_clientAdapter); // Destroyed?

    ObjectPrx proxy = _clientAdapter->createReverseProxy(current.id);
    assert(proxy);

    invoke(proxy, amdCB, inParams, current);
}
