// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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

class ClientBlobjectImpl;

class ClientBlobject : public Glacier2::Blobject
{
public:

    ClientBlobject(const Ice::CommunicatorPtr&, const FilterManagerPtr&, const Ice::Context&); 
    virtual ~ClientBlobject();

    virtual void ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

    Ice::ObjectProxySeq add(const Ice::ObjectProxySeq&, const Ice::Current&); // Returns evicted proxies.

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
