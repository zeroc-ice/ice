// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/RoutingTable.h>
#include <Glacier2/ServerBlobject.h>

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
