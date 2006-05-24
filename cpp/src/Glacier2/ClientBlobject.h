// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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

class ClientBlobjectImpl;

class ClientBlobject : public Glacier2::Blobject
{
public:

    virtual ~ClientBlobject();

    virtual void ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr&,
    				  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

    Ice::ObjectProxySeq add(const Ice::ObjectProxySeq&, const Ice::Current&); // Returns evicted proxies.

    static ClientBlobjectPtr create(const Ice::CommunicatorPtr&, const std::string&);

    StringFilterPtr categoryFilter();
    StringFilterPtr adapterIdFilter();
    IdentityFilterPtr objectIdFilter();

private:
    const RoutingTablePtr _routingTable;

    ClientBlobjectImpl* _impl;

    const int _rejectTraceLevel;

    ClientBlobject(const Ice::CommunicatorPtr&, ClientBlobjectImpl*); 
};
}

#endif
