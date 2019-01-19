//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef CLIENT_BLOBJECT_H
#define CLIENT_BLOBJECT_H

#include <Glacier2/Blobject.h>
#include <Glacier2/Session.h>

namespace Glacier2
{

class RoutingTable;
typedef IceUtil::Handle<RoutingTable> RoutingTablePtr;

class ClientBlobject;
typedef IceUtil::Handle<ClientBlobject> ClientBlobjectPtr;

class FilterManager;
typedef IceUtil::Handle<FilterManager> FilterManagerPtr;

class ClientBlobject : public Glacier2::Blobject
{
public:

    ClientBlobject(const InstancePtr&, const FilterManagerPtr&, const Ice::Context&, const RoutingTablePtr&);
    virtual ~ClientBlobject();

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

    StringSetPtr categories();
    StringSetPtr adapterIds();
    IdentitySetPtr identities();

private:

    const RoutingTablePtr _routingTable;
    const FilterManagerPtr _filters;
    const int _rejectTraceLevel;
};
}

#endif
