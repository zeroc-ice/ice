//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/NodeAdminRouter.h>
#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>

using namespace IceGrid;
using namespace Ice;
using namespace std;

NodeServerAdminRouter::NodeServerAdminRouter(const shared_ptr<NodeI>& node) :
    AdminRouter(node->getTraceLevels()),
    _node(node)
{
}

void
NodeServerAdminRouter::ice_invokeAsync(pair<const Ice::Byte*, const Ice::Byte*> inParams,
                                       function<void(bool, const pair<const Ice::Byte*, const Ice::Byte*>&)> response,
                                       function<void(exception_ptr)> exception,
                                       const Ice::Current& current)
{
    //
    // First, get the ServerI servant
    //
    Identity serverId = _node->createServerIdentity(current.id.name);
    auto server = dynamic_pointer_cast<ServerI>(_node->getAdapter()->find(serverId));
    if(server == nullptr)
    {
        if(_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
            out << "could not find Admin proxy for server `" << current.id.name << "'";
        }

        throw ObjectNotExistException(__FILE__, __LINE__);
    }

    //
    // Then get a proxy to the Process facet of the real admin object
    //
    auto target = server->getProcess();

    if(target == nullptr)
    {
        if(_traceLevels->admin > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->adminCat);
            out << "no Process proxy registered with server `" << current.id.name << "'";
        }

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

    invokeOnTarget(target, inParams, move(response), move(exception), current);
}
