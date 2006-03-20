// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/ServerBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ServerBlobject::ServerBlobject(const CommunicatorPtr& communicator, const ConnectionPtr& connection) :
    Glacier2::Blobject(communicator, true),
    _connection(connection)
{
}

Glacier2::ServerBlobject::~ServerBlobject()
{
    assert(!_connection);
}

void
Glacier2::ServerBlobject::destroy()
{
    assert(_connection); // Destroyed?
    _connection = 0;
    Blobject::destroy();
}

void
Glacier2::ServerBlobject::ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr& amdCB,
					   const std::pair<const Byte*, const Byte*>& inParams,
					   const Current& current)
{
    assert(_connection); // Destroyed?

    ObjectPrx proxy = _connection->createProxy(current.id);
    assert(proxy);

    invoke(proxy, amdCB, inParams, current);
}
