// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CLIENT_BLOBJECT_H
#define CLIENT_BLOBJECT_H

#include <Glacier2/RoutingTable.h>
#include <Glacier2/Blobject.h>

namespace Glacier2
{

class ClientBlobject;
typedef IceUtil::Handle<ClientBlobject> ClientBlobjectPtr;

class ClientBlobject : public Glacier2::Blobject
{
public:

    ClientBlobject(const Ice::CommunicatorPtr&, const RoutingTablePtr&, const Ice::StringSeq&);
    virtual ~ClientBlobject();

    virtual void destroy();

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&, const Ice::ByteSeq&, const Ice::Current&);

private:

    RoutingTablePtr _routingTable;
    const std::vector<std::string> _allowCategories;
    const int _rejectTraceLevel;
};

}

#endif
