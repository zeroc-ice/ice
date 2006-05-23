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

#include <Glacier2/RoutingTable.h>
#include <Glacier2/Blobject.h>
#include <Glacier2/Session.h>
#include <Glacier2/FilterI.h>

namespace Glacier2
{

class ClientBlobject;
typedef IceUtil::Handle<ClientBlobject> ClientBlobjectPtr;

class ClientBlobject : public Glacier2::Blobject
{
public:

    ClientBlobject(const Ice::CommunicatorPtr&, const RoutingTablePtr&, const StringFilterIPtr&,
		   const StringFilterIPtr&, const IdentityFilterIPtr&);
    virtual ~ClientBlobject();

    virtual void ice_invoke_async(const Ice::AMD_Array_Object_ice_invokePtr&,
    				  const std::pair<const Ice::Byte*, const Ice::Byte*>&, const Ice::Current&);

private:

    const RoutingTablePtr _routingTable;
    const StringFilterIPtr _categoryFilter;
    const StringFilterIPtr _adapterIdFilter;
    const IdentityFilterIPtr _objectIdFilter;
    const bool _filtersEnabled;
    const int _rejectTraceLevel;
};
}

#endif
