// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/NodeServerAdminRouter.h>
#include <Ice/Ice.h>
#include <IceGrid/ServerI.h>
#include <IceGrid/ServerAdapterI.h>

using namespace IceGrid;
using namespace Ice;
using namespace std;

namespace
{

class InvokeAMICallback : public AMI_Array_Object_ice_invoke
{
public:

    InvokeAMICallback(const AMD_Object_ice_invokePtr& cb) :
        _cb(cb)
    {
    }

    virtual void ice_response(bool ok, const pair<const Byte*, const Byte*>& outParams)
    {
        _cb->ice_response(ok, outParams);
    }
    
    virtual void ice_exception(const Ice::Exception&)
    {
        _cb->ice_exception(ObjectNotExistException(__FILE__, __LINE__)); // Server admin object is unreachable
    }
    
private:
    AMD_Object_ice_invokePtr _cb;
};

}

IceGrid::NodeServerAdminRouter::NodeServerAdminRouter(const NodeIPtr& node) :
    _node(node)
{
}

void
IceGrid::NodeServerAdminRouter::ice_invoke_async(const AMD_Object_ice_invokePtr& cb, 
                                                 const pair<const Byte*, const Byte*>& inParams,
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

    //
    // Call with AMI
    //
    target->ice_invoke_async(new InvokeAMICallback(cb), current.operation, current.mode, inParams, current.ctx);
}
