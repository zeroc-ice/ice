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
using namespace Glacier2;

Glacier2::ServerBlobject::ServerBlobject(const CommunicatorPtr& communicator, const TransportInfoPtr& transport) :
    Glacier2::Blobject(communicator, true),
    _transport(transport)
{
}

Glacier2::ServerBlobject::~ServerBlobject()
{
    assert(!_transport);
}

void
Glacier2::ServerBlobject::destroy()
{
    assert(_transport); // Destroyed?
    _transport = 0;
    Blobject::destroy();
}

void
Glacier2::ServerBlobject::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB, const vector<Byte>& inParams,
					  const Current& current)
{
    assert(_transport); // Destroyed?

    ObjectPrx proxy = _transport->createProxy(current.id);
    assert(proxy);

    invoke(proxy, amdCB, inParams, current);
}
