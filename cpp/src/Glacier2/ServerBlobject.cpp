// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#include <Glacier2/ServerBlobject.h>

using namespace std;
using namespace Ice;
using namespace Glacier2;

Glacier2::ServerBlobject::ServerBlobject(const InstancePtr& instance, const ConnectionPtr& connection) :
    Glacier2::Blobject(instance, connection, Ice::Context())
{
}

Glacier2::ServerBlobject::~ServerBlobject()
{
}

void
Glacier2::ServerBlobject::ice_invoke_async(const Ice::AMD_Object_ice_invokePtr& amdCB,
                                           const std::pair<const Byte*, const Byte*>& inParams,
                                           const Current& current)
{
    ObjectPrxPtr proxy = _reverseConnection->createProxy(current.id);
    assert(proxy);

    invoke(proxy, amdCB, inParams, current);
}
