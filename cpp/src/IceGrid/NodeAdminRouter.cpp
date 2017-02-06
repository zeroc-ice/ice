// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/NodeAdminRouter.h>
#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>

using namespace IceGrid;
using namespace Ice;
using namespace std;


IceGrid::NodeServerAdminRouter::NodeServerAdminRouter(const NodeIPtr& node) :
    _node(node)
{
}

void
IceGrid::NodeServerAdminRouter::ice_invoke_async(const AMD_Object_ice_invokePtr& cb,
                                                 const std::pair<const Ice::Byte*, const Ice::Byte*>& inParams,
                                                 const Current& current)
{
    //
    // First, get the ServerI servant
    //
    Identity serverId = _node->createServerIdentity(current.id.name);
    ServerIPtr server = ServerIPtr::dynamicCast(_node->getAdapter()->find(serverId));
    if(server == 0)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    //
    // Then get a proxy to the Process facet of the real admin object
    //
    ObjectPrx target = server->getProcess();

    if(target == 0)
    {
        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    //
    // If this is a legacy Process proxy with no facet, we keep target as is
    //
    if(current.facet != "Process")
    {
        //
        // Set the facet
        //
        target = target->ice_facet(current.facet);
    }

    invokeOnTarget(target, cb, inParams, current);
}

