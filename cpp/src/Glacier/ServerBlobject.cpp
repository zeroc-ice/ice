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
    Glacier::Blobject(clientAdapter->getCommunicator(), true),
    _clientAdapter(clientAdapter)
{
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
